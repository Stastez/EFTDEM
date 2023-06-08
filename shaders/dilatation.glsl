#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = 0) restrict buffer mapBuffer{
    double heights[];
};
layout (binding = 1) restrict buffer resultBuffer{
    double results[];
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

    double sum = 0.;
    double divisionSum = 0.;

    for (uint x = -kernelRadius; x < kernelRadius; x++) {
        for (uint y = -kernelRadius; y < kernelRadius; y++) {
            if (0 <= correctedGlobalInvocation.x + x && correctedGlobalInvocation.x + x < resolution.x &&
            0 <= correctedGlobalInvocation.y + y && correctedGlobalInvocation.y + y < resolution.y) {
                double currentHeight = heights[calculate1DCoordinate(uvec2(x, y))];
                sum += currentHeight;
                divisionSum += step(double(.00001), currentHeight);
            }
        }
    }

    double avrage = (divisionSum == 0.) ? 0. : sum / divisionSum;
    results[calculate1DCoordinate(correctedGlobalInvocation)] = (heights[calculate1DCoordinate(correctedGlobalInvocation)] == 0) ? avrage : heights[calculate1DCoordinate(correctedGlobalInvocation)];
}