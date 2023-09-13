#version 430 core
precision highp int;

#define LOCAL_SIZE 8

layout (local_size_x = LOCAL_SIZE, local_size_y = LOCAL_SIZE, local_size_z = 1) in;
layout (binding = EFTDEM_RAW_POINT_INDEX_BUFFER) restrict buffer indexBuffer{
    uint indices[];
};
layout (binding = EFTDEM_SORTED_POINT_COUNT_BUFFER) restrict buffer countBuffer{
    uint counts[];
};
uniform uint numberOfPoints;

/**
 * Counts how many points were sorted into each grid cell.
 */
void main() {
    if (gl_NumWorkGroups.x * LOCAL_SIZE * gl_GlobalInvocationID.y + gl_GlobalInvocationID.x >= numberOfPoints) return;

    uint pointIndex = gl_NumWorkGroups.x * LOCAL_SIZE * gl_GlobalInvocationID.y + gl_GlobalInvocationID.x;

    atomicAdd(counts[indices[pointIndex]], 1u);
}