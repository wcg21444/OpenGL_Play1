#pragma once
#include "../Shading/Shader.hpp"
#include <string>

class Object
{
public:
    std::string name;
    glm::mat4 modelMatrix = glm::identity<glm::mat4>();
    Object();
    virtual void draw(glm::mat4 modelMatrix, Shader &shaders) = 0;
    virtual ~Object();
    void setName(const std::string &_name);
    void setModelTransform(glm::mat4 &_transform);
};
using Scene = std::vector<std::unique_ptr<Object>>;