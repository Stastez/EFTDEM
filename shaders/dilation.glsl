#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = EFTDEM_TOTAL_WEIGHT_BUFFER) restrict buffer amountBuffer{
    float amounts[];
};
layout (binding = EFTDEM_CLOSING_MASK_BUFFER) restrict buffer resultBuffer{
    float dilationMask[];
};

uniform uvec2 resolution;
uniform uint kernelRadius;
uniform uvec2 currentInvocation;

uint calculate1DCoordinate(uvec2 pos) {
    return pos.y * resolution.x + pos.x;
}

/**
 * Makes the pixel non-void if any pixels in the area of the kernel around the pixel are non-void.
 */
void main() {
    uvec2 correctedGlobalInvocation = gl_GlobalInvocationID.xy + currentInvocation;
    if (any(greaterThanEqual(correctedGlobalInvocation, resolution))) return;
    uint coord1D;

    coord1D = calculate1DCoordinate(correctedGlobalInvocation);
    dilationMask[coord1D] = (amounts[coord1D] <= 0) ? 0.0 : 1.0;
}