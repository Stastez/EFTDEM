#ifndef EFTDEM_RADIALERODER_H
#define EFTDEM_RADIALERODER_H

#include "GLHandler.h"
#include "IRadialFillerComponent.h"

class RadialEroder : public IRadialFillerComponent {
private:
    bool flipped;
    GLHandler * glHandler;
    unsigned int batchSize;
    bool batched, ignoreOutput;
    gl::GLuint shaderProgram;
    gl::GLint resolutionLocation, currentInvocationLocation, flippedLocation;

public:
    explicit RadialEroder(GLHandler *glHandler, bool flipped, unsigned int batchSize, bool batched,
                          gl::GLuint shaderProgram, gl::GLint resolutionLocation, gl::GLint currentInvocationLocation,
                          gl::GLint flippedLocation, bool ignoreOutput);
    heightMap * apply(heightMap *map, bool generateOutput) override;
};


#endif //EFTDEM_RADIALERODER_H
