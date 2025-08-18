#pragma once
#include "Object.hpp"

class Plane : public Object
{
private:
    std::vector<float> vertices;
    std::vector<float> indices;
    GLuint VAO, VBO, EBO;
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };
    struct Mesh
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
    };
    Mesh mesh;
    Mesh createPlane(float width, float depth);

public:
    Plane(float width, float depth, const std::string _name = "Plane");
    void draw(glm::mat4 modelMatrix, Shader &shaders) override;
    ~Plane();
};