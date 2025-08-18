#include "Sphere.hpp"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <cmath>

std::vector<float> Sphere::generateSphereVertices(float radius, int sectorCount, int stackCount)
{
    std::vector<float> vertices;
    const float PI = acos(-1.0f);
    float x, y, z, xy;
    float nx, ny, nz;
    float s, t;
    for (int i = 0; i <= stackCount; ++i)
    {
        float stackAngle = PI / 2 - i * (PI / stackCount);
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);
        for (int j = 0; j <= sectorCount; ++j)
        {
            float sectorAngle = j * (2 * PI / sectorCount);
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            nx = x / radius;
            ny = y / radius;
            nz = z / radius;
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            vertices.insert(vertices.end(), {x, y, z});
            vertices.insert(vertices.end(), {nx, ny, nz});
            vertices.insert(vertices.end(), {s, t});
        }
    }
    std::vector<int> indices;
    for (int i = 0; i < stackCount; ++i)
    {
        int k1 = i * (sectorCount + 1);
        int k2 = k1 + sectorCount + 1;
        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            if (i != 0)
                indices.insert(indices.end(), {k1, k2, k1 + 1});
            if (i != (stackCount - 1))
                indices.insert(indices.end(), {k1 + 1, k2, k2 + 1});
        }
    }
    std::vector<float> vertexData;
    for (size_t i = 0; i < indices.size(); ++i)
    {
        int index = indices[i] * 8;
        vertexData.insert(vertexData.end(), vertices.begin() + index, vertices.begin() + index + 8);
    }
    return vertexData;
}

Sphere::Sphere(float radius, int sectorCount, int stackCount, const std::string _name)
{
    setName(_name);
    vertices = generateSphereVertices(radius, sectorCount, stackCount);
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

void Sphere::draw(glm::mat4 modelMatrix, Shader &shaders)
{
    glBindVertexArray(vao);
    shaders.setMat4("model", modelMatrix);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 8);
    glBindVertexArray(0);
}

Sphere::~Sphere() {}
