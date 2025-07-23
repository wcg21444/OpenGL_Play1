#pragma once
#include "Shader.hpp"

class Camera
{
public:
    enum Movement
    {
        none = 0,         // 0b0000
        forward = 1 << 0, // 0b0001
        left = 1 << 1,    // 0b0010
        right = 1 << 2,   // 0b0100
        backward = 1 << 3 // 0b1000
    }; // 每种枚举可以任意组合,总共15种状态

private:
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    float deltaTime = 0.0f; // Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame

    float lastX = 800, lastY = 450;
    float yaw = -90.0f;
    float pitch = 0.f;
    bool firstMouse = true;
    float cameraSpeed = 14.f;
    float sensitivity = 0.05f;

public:
    float fov = 60.f;
    float near = 0.1f;
    float far = 1000.f;

public:
    Camera(int width,
           int height,
           float _cameraSpeed,
           float _sensitivity) : lastX((float)width / 2),
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
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;

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
        cameraFront = glm::normalize(direction);
    }
    void genPositionfrom(GLFWwindow *window, int &movement)
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        float reletive_speed = cameraSpeed * deltaTime;

        if (movement & forward)
            cameraPos += reletive_speed * cameraFront;
        if (movement & backward)
            cameraPos -= reletive_speed * cameraFront;
        if (movement & left)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * reletive_speed;
        if (movement & right)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * reletive_speed;

        // if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        //     cameraPos += reletive_speed * cameraFront;
        // if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        //     cameraPos -= reletive_speed * cameraFront;
        // if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        //     cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * reletive_speed;
        // if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        //     cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * reletive_speed;
    }
    void genZoomfrom(double yoffset)
    {
        fov -= (float)yoffset * 2;
        if (fov < 1.0f)
            fov = 1.0f;
    }
    void setViewMatrix(Shader &shaders)
    {
        glm::mat4 view = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        float radius = 2.0f;
        float camX = static_cast<float>(sin(glfwGetTime()) * radius);
        float camZ = static_cast<float>(cos(glfwGetTime()) * radius);
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        shaders.setMat4("view", view);
        shaders.setUniform3fv("eye_pos", cameraPos);
    }
    void setPerspectiveMatrix(Shader &shaders, int width, int height)
    {
        glm::mat4 projection = glm::perspective(glm::radians(fov),
                                                (float)width / (float)height,
                                                near,
                                                far);
        shaders.setMat4("projection", projection);
    }
    glm::vec3 getPosition() const
    {
        return cameraPos;
    }
    glm::vec3 getFront() const
    {
        return cameraFront;
    }
};