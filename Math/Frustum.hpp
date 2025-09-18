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

    Frustum getSubFrustum(float nearPlane, float farPlane) const
    {
        return Frustum(m_position, m_front, m_up, nearPlane, farPlane, m_fov, m_aspect);
    }

    glm::vec3 getPosition() const override { return m_position; }
    glm::vec3 getFront() const override { return m_front; }
    glm::vec3 getUp() const override { return m_up; }
    float getNearPlane() const override { return m_nearPlane; }
    float getFarPlane() const override { return m_farPlane; }

    glm::mat4 getViewMatrix() const override
    {
        return glm::lookAt(m_position, m_position + m_front, m_up);
    }

    glm::mat4 getProjectionMatrix() const override
    {
        return glm::perspective(glm::radians(m_fov), m_aspect, m_nearPlane, m_farPlane);
    }

    glm::mat4 getProjViewMatrix() const override
    {
        return glm::perspective(glm::radians(m_fov), m_aspect, m_nearPlane, m_farPlane) * glm::lookAt(m_position, m_position + m_front, m_up);
    }

    FrustumCorners getCorners() const override
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

    inline static OrthoFrustum GenTightFtustum(const FrustumCorners &corners, const OrthoFrustum &lightOrtho)
    {
        // 1. 定义光源的视图矩阵（为了简单，这里假设光源方向是固定的）
        // 你需要根据你的光源实际位置和方向来构建这个矩阵
        glm::vec3 lightDir = -lightOrtho.getFront();
        glm::vec3 lightUp = lightOrtho.getUp();
        glm::vec3 lightPosition;

        glm::vec3 center = glm::vec3(0.0f); // average position
        center += corners.nearTopLeft;
        center += corners.nearTopRight;
        center += corners.nearBottomRight;
        center += corners.nearBottomLeft;
        center += corners.farTopLeft;
        center += corners.farTopRight;
        center += corners.farBottomRight;
        center += corners.farBottomLeft;
        center /= 8.0f;

        lightPosition = center +lightDir * 1000.0f;

        glm::mat4 lightView = glm::lookAt(lightPosition, center, -lightUp);

        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = std::numeric_limits<float>::lowest();

        std::vector<glm::vec3> all_corners = {
            corners.nearTopLeft, corners.nearTopRight, corners.nearBottomRight, corners.nearBottomLeft,
            corners.farTopLeft, corners.farTopRight, corners.farBottomRight, corners.farBottomLeft};

        for (const auto &corner : all_corners)
        {
            glm::vec4 lightSpaceCorner = lightView * glm::vec4(corner, 1.0f);
            minX = glm::min(minX, lightSpaceCorner.x);
            maxX = glm::max(maxX, lightSpaceCorner.x);
            minY = glm::min(minY, lightSpaceCorner.y);
            maxY = glm::max(maxY, lightSpaceCorner.y);
            minZ = glm::min(minZ, lightSpaceCorner.z);
            maxZ = glm::max(maxZ, lightSpaceCorner.z);
        }
        float nearPlane = minZ;
        float farPlane = maxZ;

        return OrthoFrustum(lightPosition, lightDir, lightUp,
                            minX, maxX, minY, maxY, nearPlane, farPlane);
    }

    OrthoFrustum() {}
    OrthoFrustum(const glm::vec3 &position, const glm::vec3 &front, const glm::vec3 &up,
                 float left, float right, float bottom, float top,
                 float nearPlane, float farPlane)
        : m_position(position), m_front(front), m_up(up),
          m_left(left), m_right(right), m_bottom(bottom), m_top(top),
          m_nearPlane(nearPlane), m_farPlane(farPlane) {}

    OrthoFrustum(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix)
    {
        glm::mat4 invView = glm::inverse(viewMatrix);
        glm::mat4 invProjection = -glm::inverse(projectionMatrix);

        m_position = glm::vec3(invView[3]);
        m_front = glm::normalize(glm::vec3(invView * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
        m_up = glm::normalize(glm::vec3(invView * glm::vec4(0.0f, -1.0f, 0.0f, 0.0f)));

        m_left = invProjection[3][0] - invProjection[0][0];
        m_right = invProjection[0][0] + invProjection[3][0];
        m_top = invProjection[1][1] + invProjection[3][1];
        m_bottom = invProjection[3][1] - invProjection[1][1];
        m_nearPlane = invProjection[3][2] - invProjection[2][2];
        m_farPlane = invProjection[2][2] + invProjection[3][2];
    }

    glm::mat4 getViewMatrix() const override
    {
        return glm::lookAt(m_position, m_position + m_front, m_up);
    }

    glm::mat4 getProjectionMatrix() const override
    {
        return glm::ortho(m_left, m_right, m_bottom, m_top, m_nearPlane, m_farPlane);
    }
    glm::mat4 getProjViewMatrix() const override
    {
        return glm::ortho(m_left, m_right, m_bottom, m_top, m_nearPlane, m_farPlane) * glm::lookAt(m_position, m_position + m_front, m_up);
    }

    void scale(float factor)
    {
        m_left *= factor;
        m_right *= factor;
        m_bottom *= factor;
        m_top *= factor;
    }

    FrustumCorners getCorners() const override
    {
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

    float getOrthoScaleArea()
    {
        return (m_left - m_right) * (m_top - m_bottom);
    }

    glm::vec3 getPosition() const override { return m_position; }
    glm::vec3 getFront() const override { return m_front; }
    glm::vec3 getUp() const override { return m_up; }
    float getNearPlane() const override { return m_nearPlane; }
    float getFarPlane() const override { return m_farPlane; }
};