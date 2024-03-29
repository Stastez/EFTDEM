#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = EFTDEM_HEIGHTMAP_BUFFER) restrict buffer mapBuffer{
    float heights[];
};
layout (binding = EFTDEM_GRADIENT_BUFFER) restrict buffer gradientBuffer{
    vec2 gradients[];
};

uniform uvec2 resolution;
uniform uint kernelRadius;
uniform uvec2 currentInvocation;

uint calculate1DCoordinate(uvec2 pos) {
    return pos.y * resolution.x + pos.x;
}

/**
 * Calculates the gradient at the position of the pixel
 */
void main() {
    uvec2 correctedGlobalInvocation = gl_GlobalInvocationID.xy + currentInvocation;
    if (any(greaterThanEqual(correctedGlobalInvocation, resolution))) return;
    uint coord1D = calculate1DCoordinate(correctedGlobalInvocation);
    uint coord1DRight = calculate1DCoordinate(correctedGlobalInvocation + uvec2(1,0));
    uint coord1DTop = calculate1DCoordinate(correctedGlobalInvocation + uvec2(0,1));

    gradients[coord1D].x =
        (heights[coord1D] <= 0.0 || heights[coord1DRight] <= 0.0) ?
            -2.0 :
            heights[coord1DRight] - heights[coord1D];

    gradients[coord1D].y =
        (heights[coord1D] <= 0.0 || heights[coord1DTop] <= 0.0) ?
            -2.0 :
            heights[coord1DTop] - heights[coord1D];
}

