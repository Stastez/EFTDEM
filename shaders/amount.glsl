#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = EFTDEM_HORIZONTAL_AMOUNT_BUFFER ) restrict buffer amountBuffer{
    uint horizontalAmounts[];
};
layout (binding = EFTDEM_AMOUNT_BUFFER ) restrict buffer resultBuffer{
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
    uint coord1D;

    uint amount = 0;

    for (uint ky = 0; ky <= 2*kernelRadius; ky++) {
        uint x = correctedGlobalInvocation.x;
        uint y = min(resolution.y-1u, max(0u, ky - kernelRadius + correctedGlobalInvocation.y));

        coord1D = calculate1DCoordinate(uvec2(x,y));
        amount += horizontalAmounts[coord1D];
    }

    coord1D = calculate1DCoordinate(correctedGlobalInvocation);
    amounts[coord1D] = amount;
}