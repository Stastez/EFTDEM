#ifndef EFTDEM_GLHANDLER_H
#define EFTDEM_GLHANDLER_H

#include <GLFW/glfw3.h>
#include <glbinding/gl/gl.h>
#include <glbinding-aux/ContextInfo.h>
#include <glbinding/glbinding.h>
#include <string>

class glHandler {
private:
    GLFWwindow* context;
    bool initialized = false;
    bool isDebug;

public:
    GLFWwindow * initializeGL(bool debug);
    unsigned int getShader(const std::string& shaderFile);
    void uninitializeGL();

    bool isInitialized();
    bool isInitialized(bool debug);
};


#endif //EFTDEM_GLHANDLER_H
