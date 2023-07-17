#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout (binding = EFTDEM_HEIGHTMAP_BUFFER) restrict buffer bottomBuffer{
    double bottom[];
};
layout (binding = EFTDEM_SECOND_HEIGHTMAP_BUFFER) restrict buffer topBuffer{
    double top[];
};
layout (binding = EFTDEM_COMPARISON_BUFFER) restrict buffer comparisonBuffer{
    double results[];
};

uniform uvec2 resolution;

uint calculate1DCoordinate(uvec2 pos, uvec2 referenceResolution) {
    return pos.y * referenceResolution.x + pos.x;
}

double errorValue(double deviation){
    return deviation*deviation;
}

void main() {
    if (any(greaterThan(gl_GlobalInvocationID.xy, resolution))) return;

    uint position = calculate1DCoordinate(gl_GlobalInvocationID.xy, resolution);

    // depth measured in positive amount of seconds taken by wave
    //results[position] = errorValue(clamp(bottom[position] - top[position], double(0), double(1)));
    double temp = abs(bottom[position] - top[position]);
    results[position] = clamp(temp * temp, double(0), double(1));
    //if (isnan(temp)) results[position] = 1.0;
}