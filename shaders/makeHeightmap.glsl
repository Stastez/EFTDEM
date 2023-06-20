#version 430 core
precision highp int;

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout (binding = EFTDEM_SORTED_POINT_SUM_BUFFER) restrict buffer sumBuffer{
    uint sums[];
};
layout (binding = EFTDEM_SORTED_POINT_COUNT_BUFFER) restrict buffer countBuffer{
    uint counts[];
};
layout (binding = EFTDEM_HEIGHTMAP_BUFFER) restrict buffer heightMapBuffer{
    double heights[];
};
uniform uvec2 resolution;

uint calculate1DCoordinate(uvec2 pos, uvec2 referenceResolution) {
    return pos.y * referenceResolution.x + pos.x;
}

void main() {
    if (any(greaterThanEqual(gl_GlobalInvocationID.xy, resolution))) return;

    uint coordinate = calculate1DCoordinate(gl_GlobalInvocationID.xy, resolution);

    double heightRepresentation = max(double(sums[coordinate]) / double(counts[coordinate]), double(0));
    double height = heightRepresentation / (double(~0u) / double(counts[coordinate]));

    height = (isnan(height)) ? 0. : height;

    heights[coordinate] = height;
}