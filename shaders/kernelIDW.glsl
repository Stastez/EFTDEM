#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;

layout (binding = EFTDEM_KERNEL_BUFFER) restrict buffer kernelBuffer{
    float kernel[];
};

uniform uvec2 resolution;
uniform uint kernelRadius;
uniform uvec2 currentInvocation;
float scattering = 1.0; // the bigger this Value, the heigher the impact of values that are further away from the pixel

float weightingFunction(float x) {
    //return exp(float(-abs(x/scattering)));
    //return exp(-abs(x / (scattering * float(kernelRadius) * 2.)));
    return (x == 0.) ? 1. : pow(x, -2.);
}

void main() {
    uvec2 correctedGlobalInvocation = gl_GlobalInvocationID.xy + currentInvocation;
    if (any(greaterThanEqual(correctedGlobalInvocation, resolution))) return;
    uint coord1D = correctedGlobalInvocation.x;

    kernel[coord1D] = weightingFunction(float(coord1D));
}