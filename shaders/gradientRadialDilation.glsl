#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = EFTDEM_GRADIENT_BUFFER) restrict buffer gradientBuffer {
    vec2 gradients[];
};
layout (binding = EFTDEM_HEIGHTMAP_BUFFER) restrict buffer heightMapBuffer1 {
    float firstHeightMap[];
};
layout (binding = EFTDEM_SECOND_HEIGHTMAP_BUFFER) restrict buffer heightMapBuffer2 {
    float secondHeightMap[];
};
layout (binding = EFTDEM_TOTAL_WEIGHT_BUFFER) restrict buffer totalWeightBuffer{
    float totalWeights[];
};
layout (binding = EFTDEM_SUM_BUFFER) restrict buffer sumBuffer{
    float sums[];
};
uniform uvec2 resolution;
uniform bool flipped;

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
            float originalHeight = (!flipped) ? firstHeightMap[actual1DCoordinate] : secondHeightMap[actual1DCoordinate];
            vec2 gradient = gradients[actual1DCoordinate];

            sum += float(isOnCanvas) * float(originalHeight > 0.)
            * ((float(xOffset < 0) * gradient.x
            + float(xOffset > 0) * -gradient.x
            + float(yOffset < 0) * gradient.y
            + float(yOffset > 0) * -gradient.y)
            + originalHeight);

            count += float(isOnCanvas) * float(originalHeight > 0.);
        }
    }

    float previousValue = (!flipped) ? firstHeightMap[ownCoordinate] : secondHeightMap[ownCoordinate];

    bool isVoidPixel = (previousValue <= 0.);
    count = mix(1., count, float(count > 0.));
    float average = mix(previousValue, sum / count, float(isVoidPixel));

    // mix with IDW result
    float idwHeight = sums[ownCoordinate] / (totalWeights[ownCoordinate] + float(totalWeights[ownCoordinate] == 0.));
    //average = mix(idwHeight, average, 0.5);

    if (!flipped) secondHeightMap[ownCoordinate] = average;
    else firstHeightMap[ownCoordinate] = average;
}
