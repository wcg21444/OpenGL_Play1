#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <array>
#include <string>
#include <iostream>

struct FrustumCorners
{
    glm::vec3 nearTopLeft, nearTopRight, nearBottomRight, nearBottomLeft;
    glm::vec3 farTopLeft, farTopRight, farBottomRight, farBottomLeft;
};
class Frustum
{
public:
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;

    float m_nearPlane;
    float m_farPlane;
    float m_fov;
    float m_aspect;
    Frustum(const glm::vec3 &position, const glm::vec3 &front, const glm::vec3 &up,
            float nearPlane, float farPlane, float fov, float aspect)
        : m_position(position), m_front(front), m_up(up),
          m_nearPlane(nearPlane), m_farPlane(farPlane), m_fov(fov), m_aspect(aspect)
    {
    }
    // 矩阵二元组定义
    Frustum(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix)
    {
        // 逆矩阵
        glm::mat4 invView = glm::inverse(viewMatrix);
        glm::mat4 invProjection = glm::inverse(projectionMatrix);

        // 提取位置
        m_position = glm::vec3(invView[3]);

        // 提取方向
        m_front = glm::normalize(glm::vec3(invView * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
        m_up = glm::normalize(glm::vec3(invView * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));

        // 提取近平面和远平面
        m_nearPlane = -invProjection[3][2] / invProjection[2][2];
        m_farPlane = invProjection[3][2] / (1.0f - invProjection[2][2]);

        // 提取视场角和宽高比
        float tanHalfFovY = 1.0f / invProjection[1][1];
        m_fov = glm::degrees(2.0f * atan(tanHalfFovY));
        m_aspect = invProjection[1][1] / invProjection[0][0];
    }

    virtual Frustum getSubFrustum(float nearPlane, float farPlane) const
    {
        return Frustum(m_position, m_front, m_up, nearPlane, farPlane, m_fov, m_aspect);
    }

    virtual glm::mat4 getViewMatrix() const
    {
        return glm::lookAt(m_position, m_position + m_front, m_up);
    }

    virtual glm::mat4 getProjectionMatrix() const
    {
        return glm::perspective(glm::radians(m_fov), m_aspect, m_nearPlane, m_farPlane);
    }

    virtual FrustumCorners getCornersWorldSpace() const
    {
        // 1. 计算近平面和远平面的宽高
        float tanHalfFov = glm::tan(glm::radians(m_fov / 2.0f));
        float nearHalfHeight = tanHalfFov * m_nearPlane;
        float nearHalfWidth = nearHalfHeight * m_aspect;
        float farHalfHeight = tanHalfFov * m_farPlane;
        float farHalfWidth = farHalfHeight * m_aspect;

        // 2. 计算摄像机局部空间的八个角点，并按统一顺序
        const glm::vec3 nearTopLeft(-nearHalfWidth, nearHalfHeight, -m_nearPlane);
        const glm::vec3 nearTopRight(nearHalfWidth, nearHalfHeight, -m_nearPlane);
        const glm::vec3 nearBottomRight(nearHalfWidth, -nearHalfHeight, -m_nearPlane);
        const glm::vec3 nearBottomLeft(-nearHalfWidth, -nearHalfHeight, -m_nearPlane);
        const glm::vec3 farTopLeft(-farHalfWidth, farHalfHeight, -m_farPlane);
        const glm::vec3 farTopRight(farHalfWidth, farHalfHeight, -m_farPlane);
        const glm::vec3 farBottomRight(farHalfWidth, -farHalfHeight, -m_farPlane);
        const glm::vec3 farBottomLeft(-farHalfWidth, -farHalfHeight, -m_farPlane);

        // 3. 计算世界矩阵
        const glm::mat4 worldMatrix = glm::inverse(getViewMatrix());

        // 4. 将局部空间的角点变换到世界空间并返回 FrustumCorners
        FrustumCorners corners;
        const auto transform = [&](const glm::vec3 &localPos)
        {
            return glm::vec3(worldMatrix * glm::vec4(localPos, 1.0f));
        };

        corners.nearTopLeft = transform(nearTopLeft);
        corners.nearTopRight = transform(nearTopRight);
        corners.nearBottomRight = transform(nearBottomRight);
        corners.nearBottomLeft = transform(nearBottomLeft);
        corners.farTopLeft = transform(farTopLeft);
        corners.farTopRight = transform(farTopRight);
        corners.farBottomRight = transform(farBottomRight);
        corners.farBottomLeft = transform(farBottomLeft);

        return corners;
    }

    static FrustumCorners GetCornersWorldSpace( const glm::mat4 &view,const glm::mat4 &proj)
    {
        Frustum frustum(view, proj);
        return frustum.getCornersWorldSpace();
    }
};

class OrthoFrustum
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

    OrthoFrustum(const glm::vec3 &position, const glm::vec3 &front, const glm::vec3 &up,
                 float left, float right, float bottom, float top,
                 float nearPlane, float farPlane)
        : m_position(position), m_front(front), m_up(up),
          m_left(left), m_right(right), m_bottom(bottom), m_top(top),
          m_nearPlane(nearPlane), m_farPlane(farPlane) {}

    OrthoFrustum(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix)
    {
        glm::mat4 invView = glm::inverse(viewMatrix);
        glm::mat4 invProjection = glm::inverse(projectionMatrix);

        // 提取位置和方向 (这部分是正确的)
        m_position = glm::vec3(invView[3]);
        m_front = glm::normalize(glm::vec3(invView * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
        m_up = glm::normalize(glm::vec3(invView * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));

        // 提取正交投影参数 (修正后)
        m_left = invProjection[3][0] - invProjection[0][0];
        m_right = invProjection[0][0] + invProjection[3][0];
        m_top = invProjection[1][1] + invProjection[3][1];
        m_bottom = invProjection[3][1] - invProjection[1][1];
        m_nearPlane = invProjection[3][2] - invProjection[2][2];
        m_farPlane = invProjection[2][2] + invProjection[3][2];
    }

    glm::mat4 getViewMatrix() const
    {
        return glm::lookAt(m_position, m_position + m_front, m_up);
    }

    glm::mat4 getProjectionMatrix() const
    {
        return glm::ortho(m_left, m_right, m_bottom, m_top, m_nearPlane, m_farPlane);
    }

    void scale(float factor)
    {
        m_left *= factor;
        m_right *= factor;
        m_bottom *= factor;
        m_top *= factor;
    }

    FrustumCorners getCornersWorldSpace(){
        const glm::vec3 nearTopLeft(m_left, m_top, -m_nearPlane);
        const glm::vec3 nearTopRight(m_right, m_top, -m_nearPlane);
        const glm::vec3 nearBottomRight(m_right, m_bottom, -m_nearPlane);
        const glm::vec3 nearBottomLeft(m_left, m_bottom, -m_nearPlane);
        const glm::vec3 farTopLeft(m_left, m_top, -m_farPlane);
        const glm::vec3 farTopRight(m_right, m_top, -m_farPlane);
        const glm::vec3 farBottomRight(m_right, m_bottom, -m_farPlane);
        const glm::vec3 farBottomLeft(m_left, m_bottom, -m_farPlane);

        const glm::mat4 worldMatrix = glm::inverse(getViewMatrix());

        FrustumCorners corners;
        const auto transform = [&](const glm::vec3 &localPos)
        {
            return glm::vec3(worldMatrix * glm::vec4(localPos, 1.0f));
        };

        corners.nearTopLeft = transform(nearTopLeft);
        corners.nearTopRight = transform(nearTopRight);
        corners.nearBottomRight = transform(nearBottomRight);
        corners.nearBottomLeft = transform(nearBottomLeft);
        corners.farTopLeft = transform(farTopLeft);
        corners.farTopRight = transform(farTopRight);
        corners.farBottomRight = transform(farBottomRight);
        corners.farBottomLeft = transform(farBottomLeft);

        return corners;
    }

    static FrustumCorners GetCornersWorldSpace( const glm::mat4 &view,const glm::mat4 &proj)
    {
        OrthoFrustum frustum(view, proj);
        return frustum.getCornersWorldSpace();
    }
};