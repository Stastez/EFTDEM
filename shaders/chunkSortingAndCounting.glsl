#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
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

void main() {
    if (gl_NumWorkGroups.x * 8 * gl_GlobalInvocationID.y + gl_GlobalInvocationID.x >= numberOfPoints) return;

    uint pointIndexIndices = gl_NumWorkGroups.x * 8 * gl_GlobalInvocationID.y + gl_GlobalInvocationID.x;
    uint pointIndexPoints = pointIndexIndices * 3u;

    vec3 point = vec3(points[pointIndexPoints], points[pointIndexPoints + 1], points[pointIndexPoints + 2]);

    uvec2 returnXY = uvec2(floor(vec2(resolution) * point.xy));
    returnXY.x = clamp(returnXY.x, 0u, resolution.x - 1);
    returnXY.y = clamp(returnXY.y, 0u, resolution.y - 1);

    uint coordinate = calculate1DCoordinate(returnXY, resolution);

    coordinates[pointIndexIndices] = coordinate;
    atomicAdd(counts[coordinate], 1u);
}