#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = EFTDEM_HEIGHTMAP_BUFFER) restrict buffer heightsBuffer{
    float heights[];
};
layout (binding = EFTDEM_SUM_BUFFER) restrict buffer sumsBuffer{
    float sums[];
};
layout (binding = EFTDEM_TOTAL_WEIGHT_BUFFER) restrict buffer amountsBuffer{
    float amounts[];
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

void main() {
    uvec2 correctedGlobalInvocation = gl_GlobalInvocationID.xy + currentInvocation;
    if (any(greaterThanEqual(correctedGlobalInvocation, resolution))) return;
    uint coord1D;

    coord1D = calculate1DCoordinate(correctedGlobalInvocation);
    average[coord1D] = (heights[coord1D] > 0.)
        ? heights[coord1D]
        : ((amounts[coord1D] == 0.0)
            ? 0.0
            : sums[coord1D] / amounts[coord1D]);
}