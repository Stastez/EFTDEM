#version 430 core
precision highp int;

#define LOCAL_SIZE 8

layout (local_size_x = LOCAL_SIZE, local_size_y = LOCAL_SIZE, local_size_z = 1) in;
layout (binding = EFTDEM_SORTED_POINT_SUM_BUFFER) restrict buffer sumBuffer{
    uint sums[];
};
layout (binding = EFTDEM_SORTED_POINT_COUNT_BUFFER) restrict buffer countBuffer{
    uint counts[];
};
layout (binding = EFTDEM_HEIGHTMAP_BUFFER) restrict buffer heightMapBuffer{
    float heights[];
};
uniform uvec2 resolution;

uint calculate1DCoordinate(uvec2 pos, uvec2 referenceResolution) {
    return pos.y * referenceResolution.x + pos.x;
}

/**
 * This calculates the average height per grid cell. See sumChunks.glsl for an explanation of the uint usage.
 */
void main() {
    if (any(greaterThanEqual(gl_GlobalInvocationID.xy, resolution))) return;

    uint coordinate = calculate1DCoordinate(gl_GlobalInvocationID.xy, resolution);

    double heightRepresentation = max(double(sums[coordinate]) / double(counts[coordinate]), double(0));
    float height = float(heightRepresentation / (double(~0u) / double(counts[coordinate])));

    height = (isnan(height)) ? 0. : height;

    heights[coordinate] = height;
}