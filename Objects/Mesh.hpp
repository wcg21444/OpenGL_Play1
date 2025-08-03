#pragma once

#include "Object.hpp"

class Mesh : public Object
{
public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };
    struct Texture
    {
        unsigned int id;
        std::string type;
        std::string path;
    };
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void draw(glm::mat4 modelMatrix, Shader &shaders) override;

private:
    GLuint VAO, VBO, EBO;
    void setupMesh();
};