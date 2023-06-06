#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (binding = 0) restrict buffer mapBuffer{
    double heights[];
};
layout (binding = 1) restrict buffer resultBuffer{
    double results[];
};

uniform uvec2 resolution;
uniform uint kernelPercentageDivisor;

uint calculate1DCoordinate(uvec2 pos) {
    return pos.y * resolution.x + pos.x;
}

void main() {
    double sum = 0.;
    double divisionSum = 0.;

    uvec2 kernelRadius = max(resolution / kernelPercentageDivisor, uvec2(1u));

    for (uint x = max(0u, gl_GlobalInvocationID.x - kernelRadius.x); x < min(resolution.x - 1u, gl_GlobalInvocationID.x + kernelRadius.x); x++) {
        for (uint y = max(0u, gl_GlobalInvocationID.y - kernelRadius.y); y < min(resolution.y - 1, gl_GlobalInvocationID.y + kernelRadius.y); y++) {
            double currentHeight = heights[calculate1DCoordinate(uvec2(x,y))];
            sum += currentHeight;
            divisionSum += step(double(.00001), currentHeight);
        }
    }

    results[calculate1DCoordinate(gl_GlobalInvocationID.xy)] = (divisionSum == 0.) ? 0. : sum / divisionSum;//heights[calculate1DCoordinate(gl_GlobalInvocationID.xy)];
}