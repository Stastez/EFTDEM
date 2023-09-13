#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = EFTDEM_HEIGHTMAP_BUFFER) restrict buffer heightsBuffer{
    float heights[];
};
layout (binding = EFTDEM_SUM_BUFFER) restrict buffer sumsBuffer{
    float sums[];
};
layout (binding = EFTDEM_TOTAL_WEIGHT_BUFFER) restrict buffer amountsBuffer{
    float totalWeights[];
};
layout (binding = EFTDEM_AVERAGE_BUFFER) restrict buffer averagesBuffer{
    float average[];
};

uniform uvec2 resolution;
uniform uint kernelRadius;
uniform uvec2 currentInvocation;

uint calculate1DCoordinate(uvec2 pos) {
    return pos.y * resolution.x + pos.x;
}

/**
 * Calculates the weighted average of all heights in an area of size kernelSize in every direction around the the pixel,
 * using the precalculated sum of and total weight on all non-void pixels in this area.
 */
void main() {
    uvec2 correctedGlobalInvocation = gl_GlobalInvocationID.xy + currentInvocation;
    if (any(greaterThanEqual(correctedGlobalInvocation, resolution))) return;
    uint coord1D = calculate1DCoordinate(correctedGlobalInvocation);

    average[coord1D] = (heights[coord1D] > 0.0)
        ? heights[coord1D]
        : ((totalWeights[coord1D] == 0.0)
            ? 0.0
            : sums[coord1D] / totalWeights[coord1D]);
}