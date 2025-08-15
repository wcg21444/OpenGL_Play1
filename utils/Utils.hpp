#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

void CheckGLErrors()
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cout << "OpenGL error: " << err << std::endl;
    }
}