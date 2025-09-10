#pragma once
#include "Shader.hpp"
#include "Frustum.hpp"
class Camera
{
public:
    enum Movement
    {
        none = 0,          // 0b0000
        forward = 1 << 0,  // 0b0001
        left = 1 << 1,     // 0b0010
        right = 1 << 2,    // 0b0100
        backward = 1 << 3, // 0b1000
        spirit = 1 << 4    // 0b10000
    }; // 每种枚举可以任意组合,总共15种状态

private:
    Frustum camFrustum = Frustum(glm::vec3(0.0f, 4.0f, 3.0f),
                                 glm::vec3(0.0f, 0.0f, -1.0f),
                                 glm::vec3(0.0f, 1.0f, 0.0f),
                                 0.1f,
                                 1e6f,
                                 60.f,
                                 16.f / 9.f);

    float deltaTime = 0.0f; // Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame

    float lastX = 800, lastY = 450;
    float yaw = -90.0f;
    float pitch = 0.f;
    bool firstMouse = true;
    float cameraSpeed = 14.f;
    float sensitivity = 0.05f;

    int width;
    int height;
public:
    Camera(int width,
           int height,
           float _cameraSpeed,
           float _sensitivity) : width(width),
                                 height(height),
                                 lastX((float)width / 2),
                                 lastY((float)height / 2),
                                 cameraSpeed(_cameraSpeed),
                                 sensitivity(_sensitivity)
    {
    }

public:
    void genDirectionfrom(double xpos, double ypos)
    {
        if (firstMouse)
        {
            lastX = static_cast<float>(xpos);
            lastY = static_cast<float>(ypos);
            firstMouse = false;
        }

        float xoffset = static_cast<float>(xpos - lastX);
        float yoffset = static_cast<float>(lastY - ypos);
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);

        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        camFrustum.m_front = glm::normalize(direction);
    }
    
    void genPositionfrom(GLFWwindow *window, int &movement)
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        float reletive_speed = cameraSpeed * deltaTime;
        if (movement & spirit)
            reletive_speed *= 1e3;
        if (movement & forward)
            camFrustum.m_position += reletive_speed * camFrustum.m_front;
        if (movement & backward)
            camFrustum.m_position -= reletive_speed * camFrustum.m_front;
        if (movement & left)
            camFrustum.m_position -= glm::normalize(glm::cross(camFrustum.m_front, camFrustum.m_up)) * reletive_speed;
        if (movement & right)
            camFrustum.m_position += glm::normalize(glm::cross(camFrustum.m_front, camFrustum.m_up)) * reletive_speed;

    }
    
    void genZoomfrom(double yoffset)
    {
        auto &fov = camFrustum.m_fov;
        fov -= (float)yoffset * 2;
        if (fov < 1.0f)
            fov = 1.0f;
    }
    
    void setViewMatrix(Shader &shaders)
    {
        auto view = camFrustum.getViewMatrix();
        shaders.setMat4("view", view);
        shaders.setUniform3fv("eyePos", camFrustum.m_position);
    }
    
    void setPerspectiveMatrix(Shader &shaders)
    {
        auto projection = camFrustum.getProjectionMatrix();
        shaders.setMat4("projection", projection);
    }
    
    void resize(int width, int height)
    {
        this->width = width;
        this->height = height;
        camFrustum.m_aspect = (float)this->width / (float)this->height;
    }

    glm::mat4 getPerspectiveMatrix()
    {
        return camFrustum.getProjectionMatrix();
    }

    glm::mat4 getViewMatrix()
    {
        return camFrustum.getViewMatrix();
    }
    glm::vec3 getPosition() const
    {
        return camFrustum.m_position;
    }
    glm::vec3 getFront() const
    {
        return camFrustum.m_front;
    }
    glm::vec3 getUp() const
    {
        return camFrustum.m_up;
    }

    float getFov() const
    {
        return camFrustum.m_fov;
    }

    float getNearPlane() const
    {
        return camFrustum.m_nearPlane;
    }

    float getFarPlane() const
    {
        return camFrustum.m_farPlane;
    }

    float getAspect() const
    {
        return camFrustum.m_aspect;
    }   

    void speedUp(float factor)
    {
        cameraSpeed *= factor;
    }
    
    void speedDown(float factor)
    {
        cameraSpeed /= factor;
    }

    const Frustum& getFrustum() const
    {
        return camFrustum;
    }
};