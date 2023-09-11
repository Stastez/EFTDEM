#version 430 core

layout (local_size_x = 8, local_size_y = 4, local_size_z = 1) in;
layout (binding = EFTDEM_GRADIENT_BUFFER) restrict buffer gradientBuffer {
    vec2 gradients[];
};
layout (binding = EFTDEM_HEIGHTMAP_BUFFER) restrict buffer heightMapBuffer1 {
    float firstHeightMap[];
};
layout (binding = EFTDEM_SECOND_HEIGHTMAP_BUFFER) restrict buffer heightMapBuffer2 {
    float secondHeightMap[];
};
uniform uvec2 resolution;
uniform bool flipped;

uint calculate1DCoordinate(uvec2 pos, uvec2 referenceResolution) {
    return pos.y * referenceResolution.x + pos.x;
}

/*uint calculate1DCoordinateGradientBuffer(uvec2 pos, uvec2 referenceResolution) {
    return (pos.y * referenceResolution.x + pos.x) * 2u;
}*/

void main() {
    if (any(greaterThanEqual(gl_GlobalInvocationID.xy, resolution))) return;

    uint ownCoordinate = calculate1DCoordinate(gl_GlobalInvocationID.xy, resolution);

    float sum = 0.;
    float count = 0.;

    for (int xOffset = -1; xOffset <= 1; ++xOffset) {
        for (int yOffset = -1; yOffset <= 1; ++yOffset) {
            uvec2 actualPosition = gl_GlobalInvocationID.xy + uvec2(xOffset, yOffset);
            bool isOnCanvas = all(greaterThanEqual(actualPosition, uvec2(0, 0))) && all(lessThan(actualPosition, resolution));

            uint actual1DCoordinate = calculate1DCoordinate(actualPosition, resolution);
            //uint actual1DCoordinateGradient = calculate1DCoordinateGradientBuffer(actualPosition, resolution);
            float originalHeight = (!flipped) ? firstHeightMap[actual1DCoordinate] : secondHeightMap[actual1DCoordinate];
            vec2 gradient = gradients[actual1DCoordinate];

            float distanceFromKernelOrigin = length(vec2(xOffset, yOffset));
            sum += float(isOnCanvas) * float(originalHeight > 0.)
                * ((float(xOffset < 0) * (originalHeight + gradient.x)))
                /*+ float(xOffset > 0) * (originalHeight - gradient.x)
                + float(yOffset < 0) * (originalHeight + gradient.y)
                + float(yOffset > 0) * (originalHeight - gradient.y))
                / distanceFromKernelOrigin)*/;
            count += float(isOnCanvas) * float(originalHeight > 0.);
        }
    }

    float previousValue = (!flipped) ? firstHeightMap[ownCoordinate] : secondHeightMap[ownCoordinate];

    bool isVoidPixel = (previousValue <= 0.);
    count = max(count, 1.);
    float average = mix(previousValue, sum / count, float(isVoidPixel));

    average = sum;

    if (!flipped) secondHeightMap[ownCoordinate] = average;
    else firstHeightMap[ownCoordinate] = average;
}