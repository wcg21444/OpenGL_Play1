#pragma once
#include "Object.hpp"
#include <vector>
#include <glm/glm.hpp>

class Cube : public Object
{
private:
    std::vector<float> vertices;
    GLuint vao;
    GLuint vbo;
    std::vector<float> generateCubeVertices(glm::vec3 size = glm::vec3(1.0f));

public:
    Cube(const glm::vec3 &size, const std::string _name = "Cube");
    void draw(glm::mat4 modelMatrix, Shader &shaders) override;
    ~Cube();
};