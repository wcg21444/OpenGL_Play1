#pragma once
#include "Object.hpp"
#include "Cone.hpp"
#include "Cylinder.hpp"
#include <glm/glm.hpp>
#include <glad/glad.h>

class Arrow : public Object {
private:
    glm::vec3 m_start = glm::vec3(0.0f);
    glm::vec3 m_end = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 m_color = glm::vec3(1.0f, 0.0f, 0.0f);
    float m_thickness = 0.05f;
    Cone m_cone;
    Cylinder m_cylinder;
public:
    Arrow(const glm::vec3& start = glm::vec3(0.0f), const glm::vec3& end = glm::vec3(1.0f,0.0f,0.0f),
          const glm::vec3& color = glm::vec3(1.0f,0.0f,0.0f), float thickness = 0.05f)
        : m_start(start), m_end(end), m_color(color), m_thickness(thickness),
          m_cone(thickness*2.5f, thickness*6.0f, 16, color, "ArrowCone"),
          m_cylinder(thickness, glm::length(end-start)-thickness*6.0f, 16, color, "ArrowCylinder")
    {
        name = "Arrow";
    }

    void setArrow(const glm::vec3& start, const glm::vec3& end, const glm::vec3& color, float thickness = 0.05f) {
        m_start = start;
        m_end = end;
        m_color = color;
        m_thickness = thickness;
        float arrowLen = glm::length(m_end - m_start);
        float coneHeight = m_thickness * 6.0f;
        float coneRadius = m_thickness * 2.5f;
        float shaftLen = arrowLen - coneHeight;
        if (shaftLen < 0.01f) shaftLen = arrowLen * 0.5f;
        m_cone = Cone(coneRadius, coneHeight, 16, color, "ArrowCone");
        m_cylinder = Cylinder(m_thickness, shaftLen, 16, color, "ArrowCylinder");
    }


    void draw(glm::mat4 modelMatrix, Shader& shaders) override {
        shaders.use();
        shaders.setMat4("model", modelMatrix);
        glm::vec3 dir = glm::normalize(m_end - m_start);
        float arrowLen = glm::length(m_end - m_start);
        float coneHeight = m_thickness * 6.0f;
        float coneRadius = m_thickness * 2.5f;
        float shaftLen = arrowLen - coneHeight;
        if (shaftLen < 0.01f) shaftLen = arrowLen * 0.5f;

        // Cylinder
        glm::mat4 shaftModel = glm::translate(modelMatrix, m_start + dir * (shaftLen * 0.5f));
        glm::vec3 defaultDir(0, 0, 1);
        float dot = glm::dot(defaultDir, dir);
        if (dot < 0.999f) {
            glm::vec3 axis = glm::cross(defaultDir, dir);
            float angle = acos(glm::clamp(dot, -1.0f, 1.0f));
            shaftModel = glm::rotate(shaftModel, angle, glm::normalize(axis));
        }
        shaftModel = glm::scale(shaftModel, glm::vec3(1.0f, 1.0f, 1.0f));
        m_cylinder.draw(shaftModel, shaders);

        // Cone
        glm::mat4 coneModel = glm::translate(modelMatrix, m_start + dir * shaftLen);
        dot = glm::dot(defaultDir, dir);
        if (dot < 0.999f) {
            glm::vec3 axis = glm::cross(defaultDir, dir);
            float angle = acos(glm::clamp(dot, -1.0f, 1.0f));
            coneModel = glm::rotate(coneModel, angle, glm::normalize(axis));
        }
        coneModel = glm::scale(coneModel, glm::vec3(1.0f, 1.0f, 1.0f));
        m_cone.draw(coneModel, shaders);
    }

    ~Arrow() = default;
};