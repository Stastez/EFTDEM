#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;

layout (binding = EFTDEM_CLOSING_MASK_BUFFER) restrict buffer mapBuffer{
    float discreteValues[];
};
layout (binding = EFTDEM_HORIZONTAL_AMOUNT_BUFFER) restrict buffer horizontalAmountsBuffer{
    uint horizontalAmounts[];
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
        uint x = kx - kernelRadius + correctedGlobalInvocation.x;
        uint y = correctedGlobalInvocation.y;

        if (0u <= x && x < resolution.x){
            float currentHeight = discreteValues[calculate1DCoordinate(uvec2(x,y))];
            amount += uint(currentHeight);
        }
    }

    horizontalAmounts[coord1D] = amount;
}