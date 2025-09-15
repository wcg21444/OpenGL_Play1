#pragma once
#include "Object.hpp"
#include "../Math/Frustum.hpp"

class FrustumWireframe : public Object
{
private:
    GLuint VAO, VBO, EBO;

public:
    FrustumWireframe(const std::string _name = "FrustumWireframe")
    {
        setName(_name);

        // 1. 创建 VAO, VBO, EBO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        // 2. 绑定 VAO
        glBindVertexArray(VAO);

        // 3. 绑定 VBO 并设置顶点属性
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // 这里的 buffer data 初始是空的，我们将在 draw 方法中动态更新
        glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
        glEnableVertexAttribArray(0);

        // 4. 定义并绑定 EBO
        // 视锥体有12条边，八个顶点
        GLuint indices[] = {
            0, 1, 1, 2, 2, 3, 3, 0, // 近平面
            4, 5, 5, 6, 6, 7, 7, 4, // 远平面
            0, 4, 1, 5, 2, 6, 3, 7  // 连接近平面和远平面的线
        };
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // 5. 解绑 VAO 和 VBO
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void setFrustum(const Frustum &frustum)
    {
        auto corners = frustum.getCornersWorldSpace();
        setFrustum(corners);
    }
    void setFrustum(const FrustumCorners &corners)
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(FrustumCorners), &corners);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    // 渲染方法
    void draw(glm::mat4 modelMatrix, Shader &shaders)
    {
        shaders.use();
        shaders.setMat4("model", modelMatrix);

        glBindVertexArray(VAO);
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0); // 12条线，共24个顶点
        glBindVertexArray(0);
    }

};