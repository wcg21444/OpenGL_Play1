
#pragma once
#include "Object.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

class Cone : public Object
{
private:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    GLuint VAO = 0, VBO = 0, EBO = 0;
    float m_radius = 1.0f, m_height = 1.0f;
    int m_segments = 32;
    glm::vec3 m_color = glm::vec3(1, 0, 0);

public:
    Cone(float radius = 1.0f, float height = 1.0f, int segments = 32, const glm::vec3 &color = glm::vec3(1, 0, 0), const std::string &_name = "Cone")
        : m_radius(radius), m_height(height), m_segments(segments), m_color(color)
    {
        name = _name;
        generateCone(m_radius, m_height, m_segments);
        setupMesh();
    }
    void draw(glm::mat4 modelMatrix, Shader &shaders) override
    {
        shaders.use();
        shaders.setMat4("model", modelMatrix);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    ~Cone()
    {
        if (VBO)
            glDeleteBuffers(1, &VBO);
        if (EBO)
            glDeleteBuffers(1, &EBO);
        if (VAO)
            glDeleteVertexArrays(1, &VAO);
    }

private:
void generateCone(float radius, float height, int segments)
{
    m_vertices.clear();
    m_indices.clear();

    // 顶点 (Apex)
    glm::vec3 apexPos = glm::vec3(0, 0, height);

    // 底面圆心
    glm::vec3 bottomCenterPos = glm::vec3(0, 0, 0);

    // --- 生成底面 ---
    glm::vec3 bottomNormal = glm::vec3(0, 0, -1);
    int bottomCenterIndex = m_vertices.size(); // 记录底部圆心顶点的起始索引

    // 底部圆心
    m_vertices.push_back({bottomCenterPos, bottomNormal, glm::vec2(0.5f, 0.5f)});

    // 底部圆周顶点
    for (int i = 0; i < segments; ++i)
    {
        float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
        float x = radius * cos(theta);
        float y = radius * sin(theta);
        m_vertices.push_back({glm::vec3(x, y, 0), bottomNormal, glm::vec2(0.5f * (x / radius) + 0.5f, 0.5f * (y / radius) + 0.5f)});
    }

    // 底面三角形索引 (顺时针排列，以确保法线朝外)
    for (int i = 0; i < segments; ++i)
    {
        m_indices.push_back(bottomCenterIndex); // 底部圆心
        m_indices.push_back(bottomCenterIndex + i + 1); // 当前圆周点
        m_indices.push_back(bottomCenterIndex + (i + 1) % segments + 1); // 下一个圆周点
    }

    // --- 生成侧面 ---
    int sideBaseIndex = m_vertices.size(); // 记录侧面顶点的起始索引

    // 侧面顶点和法线
    for (int i = 0; i < segments; ++i)
    {
        float theta1 = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
        float theta2 = 2.0f * glm::pi<float>() * static_cast<float>(i + 1) / static_cast<float>(segments);

        glm::vec3 v1 = glm::vec3(radius * cos(theta1), radius * sin(theta1), 0);
        glm::vec3 v2 = glm::vec3(radius * cos(theta2), radius * sin(theta2), 0);
        
        // 计算法线：逆时针顶点顺序为 v1 -> v2 -> apexPos。
        // 这将得到朝外的法线。
        glm::vec3 edge1 = v2 - v1;
        glm::vec3 edge2 = apexPos - v1;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        // 推入三角形的三个独立顶点，每个顶点拥有自己的法线
        m_vertices.push_back({v1, normal, glm::vec2(static_cast<float>(i) / segments, 0.0f)});
        m_vertices.push_back({v2, normal, glm::vec2(static_cast<float>(i + 1) / segments, 0.0f)});
        m_vertices.push_back({apexPos, normal, glm::vec2(0.5f, 1.0f)});

        // 推入索引 (连续索引)
        m_indices.push_back(sideBaseIndex + i * 3);
        m_indices.push_back(sideBaseIndex + i * 3 + 1);
        m_indices.push_back(sideBaseIndex + i * 3 + 2);
    }
}
    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }
};