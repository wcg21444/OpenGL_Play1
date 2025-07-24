#pragma once
#include "Shader.hpp"
#include <vector>

// Only PointLight is supported
class LightSource
{
private:
public:
    glm::vec3 intensity;
    glm::vec3 position;

    unsigned int depthCubemap = 0; // Depth cubemap texture for point shadows , default is 0, meaning no shadow . Prepared for multiple light sources

public:
    LightSource(const glm::vec3 &_intensity, const glm::vec3 &_position)
        : intensity(_intensity), position(_position)
    {
    }
    void setToShader(Shader &shaders) // 不同于object,不受model变换影响
    {
        shaders.setUniform3fv("light_intensity", intensity);
        shaders.setUniform3fv("light_pos", position);
    }
};

using Lights = std::vector<LightSource>;