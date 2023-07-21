#version 430 core

#define LOCAL_SIZE 8

layout (local_size_x = LOCAL_SIZE, local_size_y = LOCAL_SIZE, local_size_z = 1) in;
layout (binding = EFTDEM_RAW_POINT_BUFFER) restrict buffer pointBuffer{
    float points[];
};
layout (binding = EFTDEM_RAW_POINT_INDEX_BUFFER) restrict buffer gridCoordinateBuffer{
    uint coordinates[];
};
layout (binding = EFTDEM_SORTED_POINT_COUNT_BUFFER) restrict buffer countBuffer{
    uint counts[];
};

uniform uvec2 resolution;
uniform uint numberOfPoints;

uint calculate1DCoordinate(uvec2 pos, uvec2 referenceResolution) {
    return pos.y * referenceResolution.x + pos.x;
}

uint calculatePointIdentifier() {
    return gl_NumWorkGroups.x * LOCAL_SIZE * gl_GlobalInvocationID.y + gl_GlobalInvocationID.x;
}

void main() {
    uint pointIndexIndices = calculatePointIdentifier();
    if (pointIndexIndices >= numberOfPoints) return;
    uint pointIndexPoints = pointIndexIndices * 3u;
    vec3 point = vec3(points[pointIndexPoints], points[pointIndexPoints + 1], points[pointIndexPoints + 2]);

    uvec2 gridPosition = uvec2(floor(vec2(resolution) * point.xy));
    gridPosition = clamp(gridPosition, uvec2(0u), resolution);

    uint coordinate = calculate1DCoordinate(gridPosition, resolution);

    coordinates[pointIndexIndices] = coordinate;
    atomicAdd(counts[coordinate], 1u);
}