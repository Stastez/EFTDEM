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
    void initializeGL();
    shaderProgram getShader(const std::string& shaderFile);
};


#endif //EFTDEM_GLHANDLER_H
