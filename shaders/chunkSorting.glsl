#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
layout (binding = 0) restrict buffer pointBuffer{
    double points[];
};
layout (binding = 1) restrict buffer sumBuffer{
    uint sum[];
};
layout (binding = 2) restrict buffer countBuffer{
    double count[];
};

uniform uint pixelPerUnit;
uniform uvec2 resolution;
uniform uvec2 numberOfPoints;

uint getPointindexX(){
    return (gl_GlobalInvocationID.y * numberOfPoints.x + gl_GlobalInvocationID.x) * 3;
}
uint gridCoordToBufferPosition(vec2 pos){
    return pos.y * resolution.x + pos.x;
}

void main() {
    uint pointIndexX = getPointindexX();
    vec3 point = vec3(points[pointIndexX], points[pointIndexX+1], points[pointIndexX+2]);

    sum[gridCoordToBufferPosition(point.xy / pixelPerUnit)];

    results[position] = sum / max(double(1.), double(offsets[position + 1] - offsets[position]));
}