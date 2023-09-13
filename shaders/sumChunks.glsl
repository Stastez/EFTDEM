#version 430 core
precision highp int;

#define LOCAL_SIZE 8

layout (local_size_x = LOCAL_SIZE, local_size_y = LOCAL_SIZE, local_size_z = 1) in;
layout (binding = EFTDEM_RAW_POINT_BUFFER) restrict buffer pointBuffer{
    float points[];
};
layout (binding = EFTDEM_RAW_POINT_INDEX_BUFFER) restrict buffer indexBuffer{
    uint indices[];
};
layout (binding = EFTDEM_SORTED_POINT_SUM_BUFFER) restrict buffer sumBuffer{
    uint heights[];
};
layout (binding = EFTDEM_SORTED_POINT_COUNT_BUFFER) restrict buffer countBuffer{
    uint counts[];
};
uniform uint numberOfPoints;

uint calculatePointIdentifier() {
    return gl_NumWorkGroups.x * LOCAL_SIZE * gl_GlobalInvocationID.y + gl_GlobalInvocationID.x;
}

/**
 * Calculates the sum of the heights of all points per grid cell as a uint. This is a workaround for not having
 * EXT_SHADER_ATOMIC_FLOAT or equivalent extensions available on our hardware.
 */
void main() {
    uint pointIndexIndices = calculatePointIdentifier();
    if (pointIndexIndices >= numberOfPoints) return;
    uint pointIndexPoints = pointIndexIndices * 3u;

    float z = points[pointIndexPoints + 2];
    /*
    * This approximates a summation of doubles using uints. Note: the accuracy of this approximation will diminish
    * with an increasing amount of points per grid cell.
    */
    uint zRepresentation = uint(double(z) * (double(~0u) / double(counts[indices[pointIndexIndices]])));

    atomicAdd(heights[indices[pointIndexIndices]], zRepresentation);
}