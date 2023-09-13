#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = EFTDEM_HORIZONTAL_BUFFER) restrict buffer horizontalTotalWeightsBuffer{
    vec2 horizontalTotalWeights[];
};
layout (binding = EFTDEM_KERNEL_BUFFER) restrict buffer kernelBuffer{
    float kernel[];
};
layout (binding = EFTDEM_TOTAL_WEIGHT_BUFFER) restrict buffer totalWeightsBuffer{
    vec2 totalWeights[];
};

uniform uvec2 resolution;
uniform uint kernelRadius;
uniform uvec2 currentInvocation;

uint calculate1DCoordinate(uvec2 pos) {
    return (pos.y * resolution.x + pos.x);
}

/**
 * Calculates the sum of all weights of the kernel, that fall on non-void Pixels, in an area of size kernelSize in every direction around the the pixel,
 * by adding together the interrim results in horizontalTotalWeigts, wich where calculated by the shader gradientHorizontalTotalWeights.
 */
void main() {
    uvec2 correctedGlobalInvocation = gl_GlobalInvocationID.xy + currentInvocation;
    if (any(greaterThanEqual(correctedGlobalInvocation, resolution))) return;
    uint coord1D;

    vec2 totalWeight = vec2(0.);

    for (int ky = -int(kernelRadius); ky <= int(kernelRadius); ky++) {
        uint x = correctedGlobalInvocation.x;
        uint y = clamp(ky + int(correctedGlobalInvocation.y), 0, resolution.y-1);

        uint kernelIndex = abs(ky);

        coord1D = calculate1DCoordinate(uvec2(x,y));
        vec2 horizontalWeight = horizontalTotalWeights[coord1D];
        totalWeight += kernel[kernelIndex] * horizontalWeight;
    }

    coord1D = calculate1DCoordinate(correctedGlobalInvocation);
    totalWeights[coord1D] = totalWeight;
}