#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>

struct FrustumCorners
{
    glm::vec3 nearTopLeft, nearTopRight, nearBottomRight, nearBottomLeft;
    glm::vec3 farTopLeft, farTopRight, farBottomRight, farBottomLeft;
};

class FrustumBase
{
public:
    virtual glm::mat4 getViewMatrix() const = 0;
    virtual glm::mat4 getProjectionMatrix() const = 0;
    virtual glm::mat4 getProjViewMatrix() const = 0;
    virtual FrustumCorners getCorners() const = 0;
    virtual glm::vec3 getPosition() const = 0;
    virtual glm::vec3 getFront() const = 0;
    virtual glm::vec3 getUp() const = 0;
    virtual float getNearPlane() const = 0;
    virtual float getFarPlane() const = 0;
};

class Frustum : public FrustumBase
{
public:
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    float m_nearPlane;
    float m_farPlane;
    float m_fov;
    float m_aspect;
    
    Frustum();
    Frustum(const glm::vec3 &position, const glm::vec3 &front, const glm::vec3 &up,
            float nearPlane, float farPlane, float fov, float aspect);
    Frustum(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix);
    Frustum getSubFrustum(float nearPlane, float farPlane) const;
    glm::vec3 getPosition() const override;
    glm::vec3 getFront() const override;
    glm::vec3 getUp() const override;
    float getNearPlane() const override;
    float getFarPlane() const override;
    glm::mat4 getViewMatrix() const override;
    glm::mat4 getProjectionMatrix() const override;
    glm::mat4 getProjViewMatrix() const override;
    FrustumCorners getCorners() const override;
};

class OrthoFrustum : public FrustumBase
{
public:
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    float m_left;
    float m_right;
    float m_bottom;
    float m_top;
    float m_nearPlane;
    float m_farPlane;

    static OrthoFrustum GenTightFtustum(const FrustumCorners &corners, const glm::vec3 &lightDir, const glm::vec3 &lightUp);
    OrthoFrustum();
    OrthoFrustum(const glm::vec3 &position, const glm::vec3 &front, const glm::vec3 &up,
                 float left, float right, float bottom, float top,
                 float nearPlane, float farPlane);
    OrthoFrustum(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix);
    glm::mat4 getViewMatrix() const override;
    glm::mat4 getProjectionMatrix() const override;
    glm::mat4 getProjViewMatrix() const override;
    void scale(float factor);
    FrustumCorners getCorners() const override;
    float getOrthoScaleArea();
    glm::vec3 getPosition() const override;
    glm::vec3 getFront() const override;
    glm::vec3 getUp() const override;
    float getNearPlane() const override;
    float getFarPlane() const override;
};