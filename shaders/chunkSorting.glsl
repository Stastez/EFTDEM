#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout (binding = 1) restrict buffer pointBuffer{
    double points[];
};
layout (binding = 3) restrict buffer gridCoordinateBuffer{
    uint coordinate[];
};

uniform uvec2 resolution;
uniform uint numberOfPoints;

uint calculate1DCoordinate(uvec2 pos, uvec2 referenceResolution) {
    return pos.y * referenceResolution.x + pos.x;
}

uint calculatePointIndex(uvec2 pos) {
    return calculate1DCoordinate(pos, uvec2(ceil(sqrt(double(numberOfPoints))))) * 3u;
}

void main() {
    if (gl_GlobalInvocationID.x + gl_GlobalInvocationID.y >= numberOfPoints) return;

    uint pointIndex = calculatePointIndex(gl_GlobalInvocationID.xy);

    vec3 point = vec3(points[pointIndex], points[pointIndex + 1], points[pointIndex + 2]);

    uvec2 returnXY = uvec2(floor(vec2(resolution) * point.xy));
    returnXY.x = clamp(returnXY.x, 0u, resolution.x - 1);
    returnXY.y = clamp(returnXY.y, 0u, resolution.y - 1);

    coordinate[calculate1DCoordinate(gl_GlobalInvocationID.xy, uvec2(ceil(sqrt(double(numberOfPoints)))))] = calculate1DCoordinate(returnXY, resolution);
}