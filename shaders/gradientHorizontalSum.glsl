#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;

layout (binding = EFTDEM_GRADIENT_BUFFER) restrict buffer mapBuffer{
    vec2 values[];
};
layout (binding = EFTDEM_KERNEL_BUFFER) restrict buffer kernelBuffer{
    float kernel[];
};
layout (binding = EFTDEM_HORIZONTAL_BUFFER) restrict buffer horizontalTotalWeightsBuffer{
    vec2 horizontalTotalWeights[];
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

    vec2 totalWeight = vec2(0,0);

    for (int kx = -int(kernelRadius); kx <= int(kernelRadius); kx++) {
        uint x = clamp(kx + correctedGlobalInvocation.x, 0u, resolution.x - 1u);
        uint y = correctedGlobalInvocation.y;

        uint kernelIndex = abs(kx);
        totalWeight += kernel[kernelIndex] * values[calculate1DCoordinate(uvec2(x,y))];
    }

    horizontalTotalWeights[coord1D] = totalWeight;
}