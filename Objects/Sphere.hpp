#pragma once
#include "Object.hpp"

class Sphere : public Object
{
private:
    std::vector<float> vertices;
    GLuint vao;
    GLuint vbo;
    std::vector<float> generateSphereVertices(float radius = 1.0f, int sectorCount = 36, int stackCount = 18);

public:
    Sphere(float radius, int sectorCount = 36, int stackCount = 18, const std::string _name = "Sphere");
    void draw(glm::mat4 modelMatrix, Shader &shaders) override;
    ~Sphere();
};