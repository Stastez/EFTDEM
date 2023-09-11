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
float scattering = 1.;

float weightingFunction(float x) {
    //return exp(float(-abs(x/scattering)));
    return (x == 0) ? 1. : pow(x, -2.);
}

uint calculate1DCoordinate(uvec2 pos, uvec2 referenceResolution) {
    return pos.y * referenceResolution.x + pos.x;
}

void main() {
    if (any(greaterThanEqual(gl_GlobalInvocationID.xy, resolution))) return;

    uint ownCoordinate = calculate1DCoordinate(gl_GlobalInvocationID.xy, resolution);

    float sum = 0.;
    float count = 0.;

    for (int xOffset = -1; xOffset <= 1; ++xOffset) {
        for (int yOffset = -1; yOffset <= 1; ++yOffset) {
            ivec2 newPosition = ivec2(gl_GlobalInvocationID.xy) + ivec2(xOffset, yOffset);
            uvec2 clampedNewPosition = uvec2(clamp(newPosition, uvec2(0u), resolution - 1u));
            bool isOnCanvas = all(greaterThanEqual(clampedNewPosition, uvec2(0, 0))) && all(lessThan(clampedNewPosition, resolution));

            uint actual1DCoordinate = calculate1DCoordinate(clampedNewPosition, resolution);
            float height = (!flipped) ? firstHeightMap[actual1DCoordinate] : secondHeightMap[actual1DCoordinate];

            float distanceFromKernelOrigin = length(vec2(xOffset, yOffset));
            sum += float(isOnCanvas) * weightingFunction(distanceFromKernelOrigin) * height;
            count += float(isOnCanvas) * weightingFunction(distanceFromKernelOrigin) * float(height != 0.);
        }
    }

    float previousValue = (!flipped) ? firstHeightMap[ownCoordinate] : secondHeightMap[ownCoordinate];

    bool isVoidPixel = (previousValue == 0.);
    count = mix(1., count, float(count > 0.));
    float average = mix(previousValue, sum / count, float(isVoidPixel));

    if (!flipped) secondHeightMap[ownCoordinate] = average;
    else firstHeightMap[ownCoordinate] = average;
}