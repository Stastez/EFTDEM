#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;

layout (binding = EFTDEM_KERNEL_BUFFER) restrict buffer kernelBuffer{
    double kernel[];
};

uniform uvec2 resolution;
uniform uint kernelRadius;
uniform uvec2 currentInvocation;
double scattering = 1.0; // the bigger this Value, the heiger the impact of far Values that are further away from the pixel

double weightingFunktion(double x) {
    return exp(float(-abs(x/scattering)));
}

void main() {
    uvec2 correctedGlobalInvocation = gl_GlobalInvocationID.xy + currentInvocation;
    if (any(greaterThanEqual(correctedGlobalInvocation, resolution))) return;
    uint coord1D = correctedGlobalInvocation.x;

    kernel[coord1D] = weightingFunktion(coord1D);
}