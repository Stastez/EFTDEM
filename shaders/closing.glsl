#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
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

    for (uint x = max(0u, correctedGlobalInvocation.x - kernelRadius); x < min(resolution.x - 1u, correctedGlobalInvocation.x + kernelRadius); x++) {
        for (uint y = max(0u, correctedGlobalInvocation.y - kernelRadius); y < min(resolution.y - 1, correctedGlobalInvocation.y + kernelRadius); y++) {
            double currentHeight = heights[calculate1DCoordinate(uvec2(x,y))];
            sum += currentHeight;
            divisionSum += step(double(.00001), currentHeight);
        }
    }

    results[calculate1DCoordinate(correctedGlobalInvocation)] = (divisionSum == 0.) ? 0. : sum / divisionSum;//heights[calculate1DCoordinate(gl_GlobalInvocationID.xy)];
}