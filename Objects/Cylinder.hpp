
#pragma once
#include "Object.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

class Cylinder : public Object
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
    Cylinder(float radius = 1.0f, float height = 1.0f, int segments = 32, const glm::vec3 &color = glm::vec3(1, 0, 0), const std::string &_name = "Cylinder")
        : m_radius(radius), m_height(height), m_segments(segments), m_color(color)
    {
        name = _name;
        generateCylinder(m_radius, m_height, m_segments);
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
    ~Cylinder()
    {
        if (VBO)
            glDeleteBuffers(1, &VBO);
        if (EBO)
            glDeleteBuffers(1, &EBO);
        if (VAO)
            glDeleteVertexArrays(1, &VAO);
    }

private:
    void generateCylinder(float radius, float height, int segments)
    {
        m_vertices.clear();
        m_indices.clear();
        // 顶部和底部圆心
        m_vertices.push_back({glm::vec3(0, 0, height * 0.5f), glm::vec3(0, 0, 1), glm::vec2(0.5f, 0.5f)});   // 索引0: 顶圆心
        m_vertices.push_back({glm::vec3(0, 0, -height * 0.5f), glm::vec3(0, 0, -1), glm::vec2(0.5f, 0.5f)}); // 索引1: 底圆心

        int sideStartIndex = 2; // 侧面顶点的起始索引

        // 侧面与圆顶/圆底顶点
        for (int i = 0; i <= segments; ++i)
        {
            float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
            float x = radius * cos(theta);
            float y = radius * sin(theta);
            glm::vec3 sideNormal = glm::normalize(glm::vec3(x, y, 0));

            // 顶面圆环顶点
            m_vertices.push_back({glm::vec3(x, y, height * 0.5f), glm::vec3(0, 0, 1), glm::vec2(0.5f * (x / radius) + 0.5f, 0.5f * (y / radius) + 0.5f)});
            // 底面圆环顶点
            m_vertices.push_back({glm::vec3(x, y, -height * 0.5f), glm::vec3(0, 0, -1), glm::vec2(0.5f * (x / radius) + 0.5f, 0.5f * (y / radius) + 0.5f)});
            // 侧面顶点（顶）
            m_vertices.push_back({glm::vec3(x, y, height * 0.5f), sideNormal, glm::vec2(static_cast<float>(i) / segments, 1.0f)});
            // 侧面顶点（底）
            m_vertices.push_back({glm::vec3(x, y, -height * 0.5f), sideNormal, glm::vec2(static_cast<float>(i) / segments, 0.0f)});
        }

        // 顶面三角形索引
        for (int i = 0; i < segments; ++i)
        {
            m_indices.push_back(0);                            // 顶面圆心
            m_indices.push_back(2 + i * 4);                    // 顶面圆环顶点
            m_indices.push_back(2 + ((i + 1) % segments) * 4); // 下一个顶面圆环顶点
        }

        // 底面三角形索引
        for (int i = 0; i < segments; ++i)
        {
            m_indices.push_back(1);                            // 底面圆心
            m_indices.push_back(3 + ((i + 1) % segments) * 4); // 下一个底面圆环顶点
            m_indices.push_back(3 + i * 4);                    // 底面圆环顶点
        }

        // 侧面三角形索引
        for (int i = 0; i < segments; ++i)
        {
            int v0 = sideStartIndex + 2 + (i * 4);
            int v1 = sideStartIndex + 3 + (i * 4);
            int v2 = sideStartIndex + 2 + ((i + 1) * 4);
            int v3 = sideStartIndex + 3 + ((i + 1) * 4);

            // 侧面第一个三角形
            m_indices.push_back(v0);
            m_indices.push_back(v3);
            m_indices.push_back(v1);

            // 侧面第二个三角形
            m_indices.push_back(v0);
            m_indices.push_back(v2);
            m_indices.push_back(v3);
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