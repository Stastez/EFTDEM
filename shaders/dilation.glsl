#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = 5) restrict buffer mapBuffer{
    double heights[];
};
layout (binding = 6) restrict buffer sumBuffer{
    double horizontalSums[];
};
layout (binding = 7) restrict buffer amountBuffer{
    uint horizontalAmounts[];
};
layout (binding = 8) restrict buffer resultBuffer{
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
    uint coord1D;

    double sum = 0.;
    uint amount = 0;

    for (uint ky = 0; ky <= 2*kernelRadius; ky++) {
        uint x = correctedGlobalInvocation.x;
        uint y = min(resolution.y-1u, max(0u, ky - kernelRadius + correctedGlobalInvocation.y));

        coord1D = calculate1DCoordinate(uvec2(x,y));
        sum += (isnan(horizontalSums[coord1D])) ? 0.0 : horizontalSums[coord1D];
        amount += (isnan(horizontalAmounts[coord1D])) ? 0 : horizontalAmounts[coord1D];
    }

    coord1D = calculate1DCoordinate(correctedGlobalInvocation);
    double average = (amount <= 0) ? 0. : sum / double(amount);
    results[coord1D] = (heights[coord1D] <= 0.) ? average : heights[coord1D];
}