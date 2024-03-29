#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
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

float errorMetric(float deviation){
    return pow(deviation, 2.);
}

/**
 * Calculates the squared difference between top and bottom as (bottom - top)^2.
 */
void main() {
    if (any(greaterThan(gl_GlobalInvocationID.xy, resolution))) return;

    uint position = calculate1DCoordinate(gl_GlobalInvocationID.xy, resolution);

    // depth measured in positive amount of seconds taken by wave
    results[position] = errorMetric(bottom[position] - top[position]);
    if (isnan(results[position])) results[position] = 1000000.;
}