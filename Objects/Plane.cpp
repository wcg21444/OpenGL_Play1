#include "Plane.hpp"
#include <glad/glad.h>
#include <glm/glm.hpp>

Plane::Mesh Plane::createPlane(float width, float depth)
{
    Mesh mesh;
    float halfWidth = width * 0.5f;
    float halfDepth = depth * 0.5f;
    mesh.vertices = {
        {{-halfWidth, 0.0f, -halfDepth}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{halfWidth, 0.0f, -halfDepth}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{halfWidth, 0.0f, halfDepth}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{-halfWidth, 0.0f, halfDepth}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}};
    mesh.indices = {0, 1, 2, 0, 2, 3};
    return mesh;
}

Plane::Plane(float width, float depth, const std::string _name)
{
    setName(_name);
    mesh = createPlane(width, depth);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));
    glBindVertexArray(0);
}

void Plane::draw(glm::mat4 modelMatrix, Shader &shaders)
{
    shaders.setMat4("model", modelMatrix);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

Plane::~Plane() {}
