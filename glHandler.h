#ifndef EFTDEM_GLHANDLER_H
#define EFTDEM_GLHANDLER_H

#include <GLFW/glfw3.h>
#include <string>

struct shaderProgram {
    unsigned int ID;

};

class glHandler {
private:
    GLFWwindow* context;

public:
    GLFWwindow * initializeGL();
    shaderProgram getShader(const std::string& shaderFile);
    void runShader(shaderProgram* shaderProgram);
};


#endif //EFTDEM_GLHANDLER_H
