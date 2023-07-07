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
uniform uvec2 currentInvocation;
float scattering = 1.;

float weightingFunction(float x) {
    return exp(float(-abs(x/scattering)));
}

uint calculate1DCoordinate(uvec2 pos, uvec2 referenceResolution) {
    return pos.y * referenceResolution.x + pos.x;
}

void main() {
    uvec2 correctedGlobalInvocation = gl_GlobalInvocationID.xy + currentInvocation;
    if (any(greaterThanEqual(correctedGlobalInvocation, resolution))) return;

    uint ownCoordinate = calculate1DCoordinate(correctedGlobalInvocation, resolution);

    float sum = 0.;
    float count = 0.;

    for (int xOffset = -1; xOffset <= 1; ++xOffset) {
        for (int yOffset = -1; yOffset <= 1; ++yOffset) {
            uvec2 actualPosition = correctedGlobalInvocation + uvec2(xOffset, yOffset);
            bool isOnCanvas = all(greaterThanEqual(actualPosition, uvec2(0, 0))) && all(lessThan(actualPosition, resolution));

            uint actual1DCoordinate = calculate1DCoordinate(actualPosition, resolution);
            float height = (!flipped) ? firstHeightMap[actual1DCoordinate] : secondHeightMap[actual1DCoordinate];

            sum += float(isOnCanvas) * height; //weightingFunction(height);
            count += float(isOnCanvas) * float(height != 0.);
        }
    }

    float previousValue = (!flipped) ? firstHeightMap[ownCoordinate] : secondHeightMap[ownCoordinate];

    bool isVoidPixel = (previousValue == 0.);
    count = mix(1., count, step(0.0000001, count));
    float average = mix(previousValue, sum / count, float(isVoidPixel));

    if (!flipped) secondHeightMap[ownCoordinate] = average;
    else firstHeightMap[ownCoordinate] = average;
}