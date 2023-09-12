#include "GradientBasedFiller.h"
#include <iostream>
#include <cmath>

GradientBasedFiller::GradientBasedFiller(GLHandler *glHandler, unsigned int kernelRadius, unsigned int batchSize = 0) {
    GradientBasedFiller::glHandler = glHandler;
    GradientBasedFiller::kernelRadius = kernelRadius;
    GradientBasedFiller::batchSize = batchSize;
    GradientBasedFiller::stageUsesGPU = true;
}

GradientBasedFiller::~GradientBasedFiller() {
    //TODO: delete used buffers
    /*glHandler->deleteBuffer(GLHandler::EFTDEM_CLOSING_MASK_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_HORIZONTAL_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_TOTAL_WEIGHT_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_SUM_BUFFER);
    glHandler->deleteBuffer(GLHandler::EFTDEM_AVERAGE_BUFFER);*/
}

void GradientBasedFiller::dispatchCompute(gl::GLint flippedLocation, bool isDilation, heightMap * map) const {
    using namespace gl;

    GLsync previousSync = nullptr;
    for (auto i = 0u; i < kernelRadius; i++) {
        auto flipped = (bool) (isDilation ? (i % 2u) : (kernelRadius + i) % 2u);
        glUniform1i(flippedLocation, flipped);
        glDispatchCompute((GLuint) std::ceil((double) map->resolutionX / 8.), (GLuint) std::ceil((double) map->resolutionY / 4.), 1);

        if (previousSync != nullptr) {
            glClientWaitSync(previousSync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
            glDeleteSync(previousSync);
        }

        auto currentSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        std::swap(previousSync, currentSync);
    }
    if (previousSync != nullptr) {
        glClientWaitSync(previousSync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
        glDeleteSync(previousSync);
    }
}

/**
 * Applies the InverseDistanceWeightedFilter
 * @param map the Heightmap that should be closed (if the Heightmap is already in the EFTDEM_HEIGHTMAP_BUFFER, the given map won't be used and may be an empty heightMap)
 * @param generateOutput specifies whether to return a heightMap Object with the filled map or just an empty heightMap. This improves efficiency, by only moving Data from and to graphics memory when necessary.
 * @return a heightMap with the filled height-data, or an empty map.
 */
heightMap * GradientBasedFiller::apply(heightMap *map, bool generateOutput) {
    using namespace gl;

    std::cout << "Applying gradient based filter using OpenGL..." << std::endl << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    //The shaders in this vector will be executed
    shaderPaths = std::vector<std::string>();
    shaderPaths.emplace_back("gradient.glsl");
    shaderPaths.emplace_back("kernelIDW.glsl");
    shaderPaths.emplace_back("gradientHorizontalTotalWeight.glsl");
    shaderPaths.emplace_back("gradientTotalWeights.glsl");
    shaderPaths.emplace_back("gradientHorizontalSum.glsl");
    shaderPaths.emplace_back("gradientSum.glsl");
    shaderPaths.emplace_back("gradientFiller.glsl");

    shaderPaths.emplace_back("discretization.glsl");
    shaderPaths.emplace_back("horizontalSumIDW.glsl");
    shaderPaths.emplace_back("sumIDW.glsl");
    shaderPaths.emplace_back("horizontalTotalWeights.glsl");
    shaderPaths.emplace_back("totalWeights.glsl");

    auto pixelCount = (long) (map->resolutionX * map->resolutionY);

    /*This vector should contain a vector of bufferSpecifications for every Shader specified in shaderPaths,
     * containing the Specifications for all Buffers that need to be initialized before the respective shader is executed.*/
    auto bufferSpecs = std::vector<std::vector<bufferSpecifications>>();
    bufferSpecs.reserve(shaderPaths.size());
    // gradient
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_GRADIENT_BUFFER, long(sizeof(GLfloat) * pixelCount * 2)}});
    // kernel
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_KERNEL_BUFFER, long(sizeof(GLfloat) * kernelRadius)}});
    // gradientHorizontalTotalWeights
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_HORIZONTAL_BUFFER, long(sizeof(GLfloat) * pixelCount * 2)}});
    // totalWeights
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_TOTAL_WEIGHT_BUFFER, long(sizeof(GLfloat) * pixelCount * 2)}});
    // horizontalSum
    bufferSpecs.emplace_back();
    // sum
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_SUM_BUFFER, long(sizeof(GLfloat) * pixelCount * 2)}});
    // gradientFiller
    bufferSpecs.emplace_back();
    // discretisation
    bufferSpecs.emplace_back(std::vector<bufferSpecifications>{bufferSpecifications{GLHandler::EFTDEM_CLOSING_MASK_BUFFER, long(sizeof(GLfloat) * pixelCount)}});
    // IDW on heights, buffers already exist
    bufferSpecs.emplace_back();
    bufferSpecs.emplace_back();
    bufferSpecs.emplace_back();
    bufferSpecs.emplace_back();

    auto shader = glHandler->getShaderPrograms(shaderPaths, true);

    glHandler->setProgram(shader.at(0));

    //if the height-data is already in the EFTDEM_HEIGHTMAP_BUFFER the coherent-buffer-mask will indicate that and the data does not need to be put on the Buffer
    if (!glHandler->getCoherentBufferMask().at(GLHandler::EFTDEM_HEIGHTMAP_BUFFER)){
        glHandler->dataToBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                                (long) (sizeof(GLfloat) * pixelCount),
                                map->heights.data(), GL_STATIC_DRAW);
    }

    for (auto i = 0ul; i < shader.size(); i++) {
        for (auto spec: bufferSpecs.at(i)) allocBuffer(spec.buffer, long(spec.size));

        glHandler->setProgram(shader.at(i));
        glUniform2ui(glGetUniformLocation(shader.at(i), "resolution"), map->resolutionX, map->resolutionY);
        glUniform1ui(glGetUniformLocation(shader.at(i), "kernelRadius"), kernelRadius);
        glHandler->dispatchShader(batchSize, map->resolutionX, map->resolutionY);
        GLHandler::waitForShaderStorageIntegrity();
    }

    /*auto tmp = new float[pixelCount];
    glHandler->dataFromBuffer(GLHandler::EFTDEM_HORIZONTAL_BUFFER, 0, pixelCount * sizeof(GLfloat ), tmp);
    for (auto i = 0; i < pixelCount; i++) {
        if (tmp[i] != 0) {
            std::cout << i << ": " << tmp[i] << std::endl;
        }
    }*/

    // dilation
    auto program = glHandler->getShaderPrograms({"gradientRadialDilation.glsl"}, true).at(0);
    auto resolutionLocation = gl::glGetUniformLocation(program, "resolution");
    auto flippedLocation = gl::glGetUniformLocation(program, "flipped");

    glHandler->setProgram(program);

    if (!glHandler->getCoherentBufferMask().at(GLHandler::EFTDEM_SECOND_HEIGHTMAP_BUFFER)) {
        auto initialState = new std::vector<GLfloat>(map->resolutionX * map->resolutionY, 0);
        glHandler->dataToBuffer(GLHandler::EFTDEM_SECOND_HEIGHTMAP_BUFFER,
                                map->dataSize,
                                initialState->data(), GL_STATIC_DRAW);
        delete initialState;
    }

    glUniform2ui(resolutionLocation, map->resolutionX, map->resolutionY);

    dispatchCompute(flippedLocation, true, map);

    // erosion
    program = glHandler->getShaderPrograms({"radialErosion.glsl"}, true).at(0);
    resolutionLocation = gl::glGetUniformLocation(program, "resolution");
    flippedLocation = gl::glGetUniformLocation(program, "flipped");

    glHandler->setProgram(program);

    glUniform2ui(resolutionLocation, map->resolutionX, map->resolutionY);

    dispatchCompute(flippedLocation, false, map);

    auto filledMap = emptyHeightMapFromHeightMap(map);

    glHandler->dataFromBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                              0, map->dataSize, filledMap->heights.data());

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed time for closing: " << duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    if (!generateOutput) return emptyHeightMapFromHeightMap(map);

    //TODO warum auskommenriert
    /*auto filledMap = emptyHeightMapFromHeightMap(map);
    glHandler->dataFromBuffer(GLHandler::EFTDEM_HEIGHTMAP_BUFFER,
                              0,
                              (long) (sizeof(GLfloat) * filledMap->resolutionX * filledMap->resolutionY),
                              filledMap->heights.data());*/

    return filledMap;
}
