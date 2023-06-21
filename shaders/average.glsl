#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = EFTDEM_HEIGHTMAP_BUFFER ) restrict buffer heightsBuffer{
    double heights[];
};
layout (binding = EFTDEM_SUM_BUFFER ) restrict buffer sumsBuffer{
    double sums[];
};
layout (binding = EFTDEM_AMOUNT_BUFFER ) restrict buffer amountsBuffer{
    uint amounts[];
};
layout (binding = EFTDEM_AVERAGE_BUFFER ) restrict buffer averagesBuffer{
    double average[];
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
    average[coord1D] = (heights[coord1D] > 0) ? heights[coord1D] : (amounts[coord1D] == 0) ? 0.0 : sums[coord1D]/amounts[coord1D];
}