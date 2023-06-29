#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;

layout (binding = EFTDEM_CLOSING_MASK_BUFFER) restrict buffer mapBuffer{
    double discreteValues[];
};
layout (binding = EFTDEM_KERNEL_BUFFER) restrict buffer kernelBuffer{
    double kernel[];
};
layout (binding = EFTDEM_HORIZONTAL_TOTAL_WEIGHT_BUFFER) restrict buffer horizontalTotalWeightsBuffer{
    double horizontalTotalWiegts[];
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

    double totalWeight = 0;

    for (uint kx = 0; kx <= 2*kernelRadius; kx++) {
        uint x = kx - kernelRadius + correctedGlobalInvocation.x;
        uint y = correctedGlobalInvocation.y;

        if (0u <= x && x < resolution.x){
            double currentHeight = double(discreteValues[calculate1DCoordinate(uvec2(x,y))]);
            uint kernelIndex = (kx>kernelRadius) ? kx-kernelRadius : kernelRadius-kx;
            totalWeight += kernel[kernelIndex] * currentHeight;
        }
    }

    horizontalTotalWiegts[coord1D] = totalWeight;
}