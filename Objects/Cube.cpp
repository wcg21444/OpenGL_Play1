#include "Cube.hpp"
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <vector>

std::vector<float> Cube::generateCubeVertices(glm::vec3 size)
{
    const glm::vec3 halfSize = size * 0.5f;
    const glm::vec3 vertices[8] = {
        {-halfSize.x, -halfSize.y, halfSize.z},
        {halfSize.x, -halfSize.y, halfSize.z},
        {halfSize.x, halfSize.y, halfSize.z},
        {-halfSize.x, halfSize.y, halfSize.z},
        {-halfSize.x, -halfSize.y, -halfSize.z},
        {halfSize.x, -halfSize.y, -halfSize.z},
        {halfSize.x, halfSize.y, -halfSize.z},
        {-halfSize.x, halfSize.y, -halfSize.z}};
    const int indices[36] = {
        0, 1, 2, 0, 2, 3,
        1, 5, 6, 1, 6, 2,
        5, 4, 7, 5, 7, 6,
        4, 0, 3, 4, 3, 7,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7};
    const glm::vec3 normals[6] = {
        {0, 0, 1}, {1, 0, 0}, {0, 0, -1}, {-1, 0, 0}, {0, -1, 0}, {0, 1, 0}};
    const glm::vec2 texCoords[4] = {
        {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
    std::vector<float> vertexData;
    for (int i = 0; i < 36; ++i)
    {
        int vertexIndex = indices[i];
        int faceIndex = i / 6;
        vertexData.push_back(vertices[vertexIndex].x);
        vertexData.push_back(vertices[vertexIndex].y);
        vertexData.push_back(vertices[vertexIndex].z);
        vertexData.push_back(normals[faceIndex].x);
        vertexData.push_back(normals[faceIndex].y);
        vertexData.push_back(normals[faceIndex].z);
        int texCoordIndex = i % 6;
        if (texCoordIndex >= 4)
            texCoordIndex -= 4;
        vertexData.push_back(texCoords[texCoordIndex].x);
        vertexData.push_back(texCoords[texCoordIndex].y);
    }
    return vertexData;
}

Cube::Cube(const glm::vec3 &size, const std::string _name)
{
    setName(_name);
    vertices = generateCubeVertices(size);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void Cube::draw(glm::mat4 modelMatrix, Shader &shaders)
{
    glBindVertexArray(vao);
    shaders.setMat4("model", modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

Cube::~Cube() {}
