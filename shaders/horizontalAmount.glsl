#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = 8) restrict buffer mapBuffer{
    double heights[];
};
layout (binding = 9) restrict buffer amountBuffer{
    uint amounts[];
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
    uint coord1D = calculate1DCoordinate(correctedGlobalInvocation);

    uint amount = 0;

    for (uint kx = 0; kx <= 2*kernelRadius; kx++) {
        uint x = min(resolution.x-1u, max(0u, kx - kernelRadius + correctedGlobalInvocation.x));
        uint y = correctedGlobalInvocation.y;

        double currentHeight = heights[calculate1DCoordinate(uvec2(x,y))];
        amount += (currentHeight <= 0.0) ? 1 : 0;
    }

    amounts[coord1D] = amount;
}