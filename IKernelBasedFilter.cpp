#include "IKernelBasedFilter.h"

IKernelBasedFilter::~IKernelBasedFilter() noexcept = default;

/**
 * Allocates the specified buffer for size bytes WITHOUT initializing the memory.
 * @param buffer The GLHandler bufferIndices member to be allocated
 * @param size The size of the space to allocate in bytes
 */
void IKernelBasedFilter::allocBuffer(GLHandler::bufferIndices buffer, long size) {
    glHandler->dataToBuffer(buffer, size, nullptr, gl::GL_STATIC_DRAW);
}