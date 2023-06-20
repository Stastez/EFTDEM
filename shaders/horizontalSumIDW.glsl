#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = EFTDEM_HEIGHTMAP_BUFFER) restrict buffer mapBuffer{
    double heights[];
};
layout (binding = EFTDEM_SUM_BUFFER) restrict buffer sumBuffer{
    double sums[];
};
layout (binding = EFTDEM_TOTAL_WEIGHT_BUFFER) restrict buffer weightsBuffer{
    double totalWeights[];
};

uniform uvec2 resolution;
uniform uint kernelRadius;
uniform uvec2 currentInvocation;

uint calculate1DCoordinate(uvec2 pos) {
    return pos.y * resolution.x + pos.x;
}

double weightingFunktion(double x){
    double scalingfactor = kernelRadius*2/3;
    double ex = -(x*x/scalingfactor/scalingfactor);
    return exp(float(ex));
}

void main() {
    uvec2 correctedGlobalInvocation = gl_GlobalInvocationID.xy + currentInvocation;
    if (any(greaterThanEqual(correctedGlobalInvocation, resolution))) return;
    uint coord1D = calculate1DCoordinate(correctedGlobalInvocation);

    double sum = 0.;
    double totalWeight = 0;

    for (uint kx = 0; kx <= 2*kernelRadius; kx++) {
        uint x = min(resolution.x-1u, max(0u, kx - kernelRadius + correctedGlobalInvocation.x));
        uint y = correctedGlobalInvocation.y;

        double weight = weightingFunktion(kx-kernelRadius);
        double currentHeight = heights[calculate1DCoordinate(uvec2(x,y))];
        sum += (isnan(currentHeight)) ? 0.0 : weight * currentHeight;
        totalWeight += (currentHeight <= 0.0) ? 0 : weight;
    }

    sums[coord1D] = sum;
    totalWeights[coord1D] = totalWeight;
}