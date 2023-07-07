#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = EFTDEM_HEIGHTMAP_BUFFER) restrict buffer mapBuffer{
    float heights[];
};
layout (binding = EFTDEM_SUM_BUFFER) restrict buffer sumBuffer{
    float horizontalSums[];
};
layout (binding = EFTDEM_TOTAL_WEIGHT_BUFFER) restrict buffer weightsBuffer{
    float horizontalTotalWeights[];
};
layout (binding = EFTDEM_INTERIM_RESULT_BUFFER) restrict buffer resultBuffer{
    float results[];
};

uniform uvec2 resolution;
uniform uint kernelRadius;
uniform uvec2 currentInvocation;

uint calculate1DCoordinate(uvec2 pos) {
    return pos.y * resolution.x + pos.x;
}

float weightingFunction(float x){
    float scalingfactor = kernelRadius*2;
    float ex = -(x*x/scalingfactor/scalingfactor);
    return exp(float(ex));
}

void main() {
    uvec2 correctedGlobalInvocation = gl_GlobalInvocationID.xy + currentInvocation;
    if (any(greaterThanEqual(correctedGlobalInvocation, resolution))) return;
    uint coord1D;

    float sum = 0.;
    float totalWeights = 0;

    for (uint ky = 0; ky <= 2*kernelRadius; ky++) {
        uint x = correctedGlobalInvocation.x;
        uint y = min(resolution.y-1u, max(0u, ky - kernelRadius + correctedGlobalInvocation.y));

        float weight = weightingFunction(ky-kernelRadius);
        coord1D = calculate1DCoordinate(uvec2(x,y));
        sum += (isnan(horizontalSums[coord1D])) ? 0.0 : weight * horizontalSums[coord1D];
        totalWeights += (isnan(horizontalTotalWeights[coord1D])) ? 0 : weight * horizontalTotalWeights[coord1D];
    }

    coord1D = calculate1DCoordinate(correctedGlobalInvocation);
    float average = (totalWeights <= 0) ? 0. : sum / totalWeights;
    results[coord1D] = (heights[coord1D] <= 0.) ? average : heights[coord1D];
}