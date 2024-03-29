#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = EFTDEM_CLOSING_MASK_BUFFER) restrict buffer closingMaskBuffer{
    float closingMask[];
};
layout (binding = EFTDEM_AVERAGE_BUFFER) restrict buffer averagesBuffer{
    float average[];
};
layout (binding = EFTDEM_HEIGHTMAP_BUFFER) restrict buffer heightsBuffer{
    float heights[];
};

uniform uvec2 resolution;
uniform uint kernelRadius;
uniform uvec2 currentInvocation;

uint calculate1DCoordinate(uvec2 pos) {
    return pos.y * resolution.x + pos.x;
}

/**
 * Combines the (weighted) average of heights with the closing mask, wich specifies wich pixels should be filled,
 * to produce the finished filled heightMap.
 */
void main() {
    uvec2 correctedGlobalInvocation = gl_GlobalInvocationID.xy + currentInvocation;
    if (any(greaterThanEqual(correctedGlobalInvocation, resolution))) return;
    uint coord1D;

    coord1D = calculate1DCoordinate(correctedGlobalInvocation);
    heights[coord1D] = closingMask[coord1D] * average[coord1D];
}