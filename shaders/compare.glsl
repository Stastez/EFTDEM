#version 430 core

#define LOCAL_SIZE 8

layout (local_size_x = LOCAL_SIZE, local_size_y = LOCAL_SIZE, local_size_z = 1) in;
layout (binding = EFTDEM_HEIGHTMAP_BUFFER) restrict buffer bottomBuffer{
    float bottom[];
};
layout (binding = EFTDEM_SECOND_HEIGHTMAP_BUFFER) restrict buffer topBuffer{
    float top[];
};
layout (binding = EFTDEM_COMPARISON_BUFFER) restrict buffer comparisonBuffer{
    float results[];
};

uniform uvec2 resolution;

uint calculate1DCoordinate(uvec2 pos, uvec2 referenceResolution) {
    return pos.y * referenceResolution.x + pos.x;
}

/**
 * Calculates the difference between the heights of top and bottom as bottom - top.
 */
void main() {
    if (any(greaterThan(gl_GlobalInvocationID.xy, resolution))) return;

    uint position = calculate1DCoordinate(gl_GlobalInvocationID.xy, resolution);

    // depth measured in positive amount of seconds taken by wave
    results[position] = clamp(bottom[position] - top[position], 0., 1.);
}