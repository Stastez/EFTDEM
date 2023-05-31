__kernel void averageHeight(const unsigned long resolutionX, const unsigned long resolutionY, __global const double *heights, __global const uint *offsets, __global double *results) {
    size_t id = get_global_id(0);

    double sum = 0;
    for (uint i = offsets[id]; i < offsets[id + 1]; i++) {
        sum += heights[i];
    }

    results[id] = sum / (offsets[id + 1] - offsets[id]);
}