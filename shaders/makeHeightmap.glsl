#version 430 core
precision highp int;

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout (binding = 3) restrict buffer sumBuffer{
    uint sums[];
};
layout (binding = 4) restrict buffer countBuffer{
    uint counts[];
};
layout (binding = 5) restrict buffer heightMapBuffer{
    double heights[];
};
//layout (binding = 6) uniform atomic_uint maxPointsPerGridCell;
uniform uvec2 resolution;

uint calculate1DCoordinate(uvec2 pos, uvec2 referenceResolution) {
    return pos.y * referenceResolution.x + pos.x;
}

void main() {
    if (any(greaterThanEqual(gl_GlobalInvocationID.xy, resolution))) return;

    uint coordinate = calculate1DCoordinate(gl_GlobalInvocationID.xy, resolution);

    double heightRepresentation = double(sums[coordinate]) / double(counts[coordinate]);
    double height = heightRepresentation / double(~0u / counts[coordinate]);

    heights[coordinate] = height;
}