#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = EFTDEM_HORIZONTAL_BUFFER) restrict buffer horizontalSumsBuffer{
    float horizontalSums[];
};
layout (binding = EFTDEM_KERNEL_BUFFER) restrict buffer kernelBuffer{
    float kernel[];
};
layout (binding = EFTDEM_SUM_BUFFER) restrict buffer sumsBuffer{
    float sums[];
};

uniform uvec2 resolution;
uniform uint kernelRadius;
uniform uvec2 currentInvocation;

uint calculate1DCoordinate(uvec2 pos) {
    return pos.y * resolution.x + pos.x;
}

/**
 * Calculates the weighted sum of all heights in an area of size kernelSize in every direction around the the pixel,
 * by adding together the interrim results in horizontalSums, wich where calculated by the shader horizontalSumIDW.
 */
void main() {
    uvec2 correctedGlobalInvocation = gl_GlobalInvocationID.xy + currentInvocation;
    if (any(greaterThanEqual(correctedGlobalInvocation, resolution))) return;
    uint coord1D = calculate1DCoordinate(correctedGlobalInvocation);

    float sum = 0.0;

    for (uint ky = 0; ky <= 2*kernelRadius; ky++) {
        uint x = correctedGlobalInvocation.x;
        uint y = ky - kernelRadius + correctedGlobalInvocation.y;

        if (0u <= y && y < resolution.y){
            uint kernelIndex = (ky>kernelRadius) ? ky-kernelRadius : kernelRadius-ky;
            sum += kernel[kernelIndex] * horizontalSums[calculate1DCoordinate(uvec2(x,y))];
        }
    }

    sums[coord1D] = sum;
}