#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (binding = 0) buffer pointBuffer{
    double heights[];
};
layout (binding = 1) buffer offsetBuffer{
    uint offsets[];
};
layout (binding = 2) buffer resultsBuffer{
    double results[];
};

uniform uvec2 resolution;

uint calculate1DCoordinate(uvec2 pos) {
    return pos.y * resolution.x + pos.x;
}

void main() {
    double sum = 0.;
    uint position = calculate1DCoordinate(gl_GlobalInvocationID.xy);

    for (uint i = offsets[position]; i < offsets[position + 1]; i++) {
        sum += heights[i];
    }

    results[position] = sum / double(offsets[position + 1] - offsets[position]);
}