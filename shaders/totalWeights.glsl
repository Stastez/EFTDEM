#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = EFTDEM_HORIZONTAL_BUFFER) restrict buffer horizontalTotalWeightsBuffer{
    float horizontalTotalWeigts[];
};
layout (binding = EFTDEM_KERNEL_BUFFER) restrict buffer kernelBuffer{
    float kernel[];
};
layout (binding = EFTDEM_TOTAL_WEIGHT_BUFFER) restrict buffer totalWeightsBuffer{
    float totalWeights[];
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

    float totalWeight = 0.0;

    for (uint ky = 0; ky <= 2*kernelRadius; ky++) {
        uint x = correctedGlobalInvocation.x;
        uint y = min(resolution.y-1u, max(0u, (ky - kernelRadius + correctedGlobalInvocation.y)));

        coord1D = calculate1DCoordinate(uvec2(x,y));
        uint kernelIndex = (ky>kernelRadius) ? ky-kernelRadius : kernelRadius-ky;
        totalWeight += kernel[kernelIndex] * horizontalTotalWeigts[coord1D];
    }

    coord1D = calculate1DCoordinate(correctedGlobalInvocation);
    totalWeights[coord1D] = totalWeight;
}