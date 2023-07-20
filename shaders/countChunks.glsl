#version 430 core
precision highp int;

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout (binding = EFTDEM_RAW_POINT_INDEX_BUFFER) restrict buffer indexBuffer{
    uint indices[];
};
layout (binding = EFTDEM_SORTED_POINT_COUNT_BUFFER) restrict buffer countBuffer{
    uint counts[];
};
uniform uint numberOfPoints;

void main() {
    if (gl_NumWorkGroups.x * 8 * gl_GlobalInvocationID.y + gl_GlobalInvocationID.x >= numberOfPoints) return;

    uint pointIndex = gl_NumWorkGroups.x * 8 * gl_GlobalInvocationID.y + gl_GlobalInvocationID.x;

    atomicAdd(counts[indices[pointIndex]], 1u);
}