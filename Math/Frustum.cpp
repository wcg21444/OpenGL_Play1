#include "Frustum.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <limits>
#include "../Renderers/DebugObjectRenderer.hpp"

Frustum::Frustum()
{
}

// Frustum implementations
Frustum::Frustum(const glm::vec3 &position, const glm::vec3 &front, const glm::vec3 &up,
                 float nearPlane, float farPlane, float fov, float aspect)
    : m_position(position), m_front(front), m_up(up),
      m_nearPlane(nearPlane), m_farPlane(farPlane), m_fov(fov), m_aspect(aspect)
{
}

Frustum::Frustum(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix)
{
    glm::mat4 invView = glm::inverse(viewMatrix);

    m_position = glm::vec3(invView[3]);
    m_front = glm::normalize(glm::vec3(invView * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
    m_up = glm::normalize(glm::vec3(invView * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));

    m_fov = glm::degrees(2.0f * atan(1.0f / projectionMatrix[1][1]));
    m_aspect = projectionMatrix[1][1] / projectionMatrix[0][0];
    m_nearPlane = projectionMatrix[3][2] / (projectionMatrix[2][2] - 1.0f);
    m_farPlane = projectionMatrix[3][2] / (projectionMatrix[2][2] + 1.0f);
}

Frustum Frustum::getSubFrustum(float nearPlane, float farPlane) const
{
    return Frustum(m_position, m_front, m_up, nearPlane, farPlane, m_fov, m_aspect);
}

glm::vec3 Frustum::getPosition() const { return m_position; }
glm::vec3 Frustum::getFront() const { return m_front; }
glm::vec3 Frustum::getUp() const { return m_up; }
float Frustum::getNearPlane() const { return m_nearPlane; }
float Frustum::getFarPlane() const { return m_farPlane; }

glm::mat4 Frustum::getViewMatrix() const
{
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Frustum::getProjectionMatrix() const
{
    return glm::perspective(glm::radians(m_fov), m_aspect, m_nearPlane, m_farPlane);
}

glm::mat4 Frustum::getProjViewMatrix() const
{
    return glm::perspective(glm::radians(m_fov), m_aspect, m_nearPlane, m_farPlane) * glm::lookAt(m_position, m_position + m_front, m_up);
}

FrustumCorners Frustum::getCorners() const
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

// OrthoFrustum implementations
OrthoFrustum OrthoFrustum::GenTightFtustum(const FrustumCorners &corners, const glm::vec3 &lightDir, const glm::vec3 &lightUp)
{
    std::vector<glm::vec3> all_corners = {
        corners.nearTopLeft, corners.nearTopRight, corners.nearBottomRight, corners.nearBottomLeft,
        corners.farTopLeft, corners.farTopRight, corners.farBottomRight, corners.farBottomLeft};
    glm::vec3 center = glm::vec3(0.0f); // average position

    for (auto &corner : all_corners)
    {
        center += corner;
        // DebugObjectRenderer::AddDrawCall([corner](Shader &debugObjectShaders) -> void
        //                                  { DebugObjectRenderer::DrawCube(debugObjectShaders, glm::vec4(1.0f),
        //                                                                  glm::translate(
        //                                                                      glm::identity<glm::mat4>(), corner) *
        //                                                                      glm::scale(glm::identity<glm::mat4>(), glm::vec3(0.1f))); });
    }
    center /= all_corners.size();

    // DebugObjectRenderer::AddDrawCall([center](Shader &debugObjectShaders) -> void
    //                                  { DebugObjectRenderer::DrawCube(debugObjectShaders, glm::vec4(1.0f),
    //                                                                  glm::translate(
    //                                                                      glm::identity<glm::mat4>(), center) *
    //                                                                      glm::scale(glm::identity<glm::mat4>(), glm::vec3(0.1f))); });

    auto lightPosition = center - lightDir;

    // DebugObjectRenderer::AddDrawCall([lightPosition](Shader &debugObjectShaders) -> void
    //                                  { DebugObjectRenderer::DrawCube(debugObjectShaders, glm::vec4(1.0f),
    //                                                                  glm::translate(
    //                                                                      glm::identity<glm::mat4>(), lightPosition) *
    //                                                                      glm::scale(glm::identity<glm::mat4>(), glm::vec3(0.5f))); });

    glm::mat4 lightView = glm::lookAt(lightPosition, center, lightUp);

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();

    for (const auto &corner : all_corners)
    {
        glm::vec4 lightSpaceCorner = lightView * glm::vec4(corner, 1.0f);
        minX = glm::min(minX, lightSpaceCorner.x);
        maxX = glm::max(maxX, lightSpaceCorner.x);
        minY = glm::min(minY, lightSpaceCorner.y);
        maxY = glm::max(maxY, lightSpaceCorner.y);
        minZ = glm::min(minZ, -lightSpaceCorner.z);
        maxZ = glm::max(maxZ, -lightSpaceCorner.z);
    }
    float nearPlane = minZ;
    float farPlane = maxZ;
    auto lightProjection = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

    // DebugObjectRenderer::AddDrawCall([lightDir, lightPosition, lightUp](Shader &debugObjectShaders) -> void
    //                                  {
    //     DebugObjectRenderer::DrawArrow(debugObjectShaders,
    //                                    lightPosition,
    //                                    lightPosition + lightDir * 5.0f,
    //                                    0.1f,
    //                                    glm::vec4(1.0,0.f,0.f,1.0f));
    //     DebugObjectRenderer::DrawArrow(debugObjectShaders,
    //                                    lightPosition,
    //                                    lightPosition + lightUp * 5.0f,
    //                                    0.1f,
    //                                    glm::vec4(0.0f,1.0f,0.0f,1.0f)); });

    return OrthoFrustum(lightPosition, lightDir, lightUp, minX, maxX, minY, maxY, nearPlane, farPlane);
}

OrthoFrustum::OrthoFrustum() {}

OrthoFrustum::OrthoFrustum(const glm::vec3 &position, const glm::vec3 &front, const glm::vec3 &up,
                           float left, float right, float bottom, float top,
                           float nearPlane, float farPlane)
    : m_position(position), m_front(front), m_up(up),
      m_left(left), m_right(right), m_bottom(bottom), m_top(top),
      m_nearPlane(nearPlane), m_farPlane(farPlane) {}

OrthoFrustum::OrthoFrustum(const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix)
{
    glm::mat4 invView = glm::inverse(viewMatrix);

    m_position = glm::vec3(invView[3]);
    m_front = glm::normalize(glm::vec3(invView * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f)));
    m_up = glm::normalize(glm::vec3(invView * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));

    float l = -(projectionMatrix[3][0] + 1.0f) / projectionMatrix[0][0];
    float r = (1.0f - projectionMatrix[3][0]) / projectionMatrix[0][0];
    float b = -(projectionMatrix[3][1] + 1.0f) / projectionMatrix[1][1];
    float t = (1.0f - projectionMatrix[3][1]) / projectionMatrix[1][1];
    float n = (projectionMatrix[3][2] + 1.0f) / projectionMatrix[2][2];
    float f = -(1.0f - projectionMatrix[3][2]) / projectionMatrix[2][2];

    m_left = l;
    m_right = r;
    m_bottom = b;
    m_top = t;
    m_nearPlane = n;
    m_farPlane = f;
}

glm::mat4 OrthoFrustum::getViewMatrix() const
{
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 OrthoFrustum::getProjectionMatrix() const
{
    return glm::ortho(m_left, m_right, m_bottom, m_top, m_nearPlane, m_farPlane);
}

glm::mat4 OrthoFrustum::getProjViewMatrix() const
{
    return glm::ortho(m_left, m_right, m_bottom, m_top, m_nearPlane, m_farPlane) * glm::lookAt(m_position, m_position + m_front, m_up);
}

void OrthoFrustum::scale(float factor)
{
    m_left *= factor;
    m_right *= factor;
    m_bottom *= factor;
    m_top *= factor;
}

FrustumCorners OrthoFrustum::getCorners() const
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

float OrthoFrustum::getOrthoScaleArea()
{
    return (m_left - m_right) * (m_top - m_bottom);
}

glm::vec3 OrthoFrustum::getPosition() const
{
    return m_position;
}
glm::vec3 OrthoFrustum::getFront() const
{
    return m_front;
}
glm::vec3 OrthoFrustum::getUp() const
{
    return m_up;
}
float OrthoFrustum::getNearPlane() const
{
    return m_nearPlane;
}
float OrthoFrustum::getFarPlane() const
{
    return m_farPlane;
}