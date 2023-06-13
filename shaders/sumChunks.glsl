#version 430 core
precision highp int;

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout (binding = 1) restrict buffer pointBuffer{
    double points[];
};
layout (binding = 2) restrict buffer indexBuffer{
    uint indices[];
};
layout (binding = 3) restrict buffer sumBuffer{
    uint heights[];
};
layout (binding = 4) restrict buffer countBuffer{
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

    double z = points[pointIndexPoints + 2];
    uint zRepresentation = uint(z * (double(~0u) / double(counts[indices[pointIndexIndices]])));

    atomicAdd(heights[indices[pointIndexIndices]], zRepresentation);
}