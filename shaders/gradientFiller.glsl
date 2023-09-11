#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;

layout (binding = EFTDEM_GRADIENT_BUFFER) restrict buffer gradientBuffer{
    vec2 gradient[];
};
layout (binding = EFTDEM_TOTAL_WEIGHT_BUFFER) restrict buffer totalWeightBuffer{
    vec2 totalWeights[];
};
layout (binding = EFTDEM_SUM_BUFFER) restrict buffer sumBuffer{
    vec2 sums[];
};
layout (binding = EFTDEM_HEIGHTMAP_BUFFER) restrict buffer heightsBuffer{
    float heights[];
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

    coord1D = calculate1DCoordinate(correctedGlobalInvocation);
    //gradient[coord1D] = abs(sums[coord1D] / totalWeights[coord1D]);
    gradient[coord1D] = vec2(0.2);
}