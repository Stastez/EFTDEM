#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = EFTDEM_HEIGHTMAP_BUFFER) restrict buffer heightMapBuffer1 {
    float firstHeightMap[];
};
layout (binding = EFTDEM_SECOND_HEIGHTMAP_BUFFER) restrict buffer heightMapBuffer2 {
    float secondHeightMap[];
};
uniform uvec2 resolution;
uniform bool flipped;

uint calculate1DCoordinate(uvec2 pos, uvec2 referenceResolution) {
    return pos.y * referenceResolution.x + pos.x;
}

void main() {
    if (any(greaterThanEqual(gl_GlobalInvocationID.xy, resolution))) return;

    uint ownCoordinate = calculate1DCoordinate(gl_GlobalInvocationID.xy, resolution);

    float minValue = 1.;

    for (int xOffset = -1; xOffset <= 1; ++xOffset) {
        for (int yOffset = -1; yOffset <= 1; ++yOffset) {
            uvec2 actualPosition = gl_GlobalInvocationID.xy + uvec2(xOffset, yOffset);
            bool isOnCanvas = all(greaterThanEqual(actualPosition, uvec2(0, 0))) && all(lessThan(actualPosition, resolution));

            uint actual1DCoordinate = calculate1DCoordinate(actualPosition, resolution);
            float height = (!flipped) ? firstHeightMap[actual1DCoordinate] : secondHeightMap[actual1DCoordinate];
            height = mix(1., height, float(isOnCanvas));
            minValue = min(minValue, height);
        }
    }

    if (!flipped) secondHeightMap[ownCoordinate] = mix(0., secondHeightMap[ownCoordinate], step(0.0000001, minValue));
    else firstHeightMap[ownCoordinate] = mix(0., secondHeightMap[ownCoordinate], step(0.0000001, minValue));
}