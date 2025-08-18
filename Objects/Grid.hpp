#pragma once
#include "Object.hpp"

class Grid : public Object
{
private:
    std::vector<float> vertices;
    GLuint vao;
    GLuint vbo;
    std::vector<float> generateGridVertices(float size, int steps);

public:
    Grid(const std::string _name = "Grid");
    void draw(glm::mat4 modelMatrix, Shader &shaders) override;
    ~Grid();
};