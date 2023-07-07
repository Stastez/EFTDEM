#version 430 core
precision highp int;

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
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
uniform uvec2 resolution;
uniform uint numberOfPoints;

uint calculate1DCoordinate(uvec2 pos, uvec2 referenceResolution) {
    return pos.y * referenceResolution.x + pos.x;
}

uint calculatePointIndex(uvec2 pos) {
    return calculate1DCoordinate(pos, uvec2(ceil(sqrt(double(numberOfPoints)))));
}

void main() {
    if (gl_NumWorkGroups.x * 8 * gl_GlobalInvocationID.y + gl_GlobalInvocationID.x >= numberOfPoints) return;

    uint pointIndexIndices = calculatePointIndex(gl_GlobalInvocationID.xy);
    uint pointIndexPoints = pointIndexIndices * 3u;

    float z = points[pointIndexPoints + 2];
    uint zRepresentation = uint(double(z) * (double(~0u) / double(counts[indices[pointIndexIndices]])));

    atomicAdd(heights[indices[pointIndexIndices]], zRepresentation);
}