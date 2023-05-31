#ifndef EFTDEM_GLHANDLER_H
#define EFTDEM_GLHANDLER_H

#include <GLFW/glfw3.h>
#include <glbinding/gl/gl.h>
#include <glbinding-aux/ContextInfo.h>
#include <glbinding/glbinding.h>
#include <string>

struct shaderProgram {
    unsigned int ID;

};

class glHandler {
private:
    GLFWwindow* context;

public:
    GLFWwindow * initializeGL(bool debug);
    shaderProgram getShader(const std::string& shaderFile);
};


#endif //EFTDEM_GLHANDLER_H
