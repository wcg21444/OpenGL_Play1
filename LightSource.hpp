#pragma once
#include "Shader.hpp"

class LightSource
{
private:
public:
    glm::vec3 intensity;
    glm::vec3 position;

public:
    LightSource(const glm::vec3 &_intensity, const glm::vec3 &_position)
        : intensity(_intensity), position(_position)
    {
    }
    void set(Shader &shaders) // 不同于object,不受model变换影响
    {
        shaders.setUniform3fv("light_intensity", intensity);
        shaders.setUniform3fv("light_pos", position);
    }
};
