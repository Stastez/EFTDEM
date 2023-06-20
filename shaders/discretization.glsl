#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = 5) restrict buffer mapBuffer{
    double heights[];
};
layout (binding = 10) restrict buffer discreteValuesBuffer{
    double discreteValues[];
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

    discreteValues[coord1D] = (heights[coord1D] <= 0.0) ? 0.0 : 1.0;
}
