#pragma once
#include "Shader.hpp"
#include <string>

class Object
{
private:
    /* data */
public:
    std::string name;

public:
    Object() {}
    virtual void draw(glm::mat4 modelMatrix, Shader &shaders) = 0;
    virtual ~Object() {}
    void setName(const std::string &_name) { name = _name; }
};
