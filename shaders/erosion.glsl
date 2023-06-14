#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = 8) restrict buffer mapBuffer{
    double heights[];
};
layout (binding = 10) restrict buffer resultBuffer{
    double results[];
};

uniform uvec2 resolution;
uniform uint kernelRadius;
uniform uvec2 currentInvocation;

uint calculate1DCoordinate(uvec2 pos) {
    return pos.y * resolution.x + pos.x;
}

void main() {
    uvec2 correctedGlobalInvocation = gl_GlobalInvocationID.xy + currentInvocation;
    if (any(greaterThanEqual(correctedGlobalInvocation, resolution))) return;
    uint coord1D = calculate1DCoordinate(correctedGlobalInvocation);

    bool hasEmptyField = false;

    for (uint kx = 0; kx < 2*kernelRadius; kx++) {
        for (uint ky = 0; ky < 2*kernelRadius; ky++) {
            uint x = min(resolution.x-1u, max(0u, kx - kernelRadius + correctedGlobalInvocation.x));
            uint y = min(resolution.y-1u, max(0u, ky - kernelRadius + correctedGlobalInvocation.y));

            double currentHeight = heights[calculate1DCoordinate(uvec2(x,y))];
            hasEmptyField = hasEmptyField || currentHeight == 0;
        }
    }

    results[coord1D] = (hasEmptyField) ? 0. : heights[coord1D];

}