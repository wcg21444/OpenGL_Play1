#pragma once
#include "Camera.hpp"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

class InputHandler
{
    // 内部状态
private:
    // 控制状态枚举
    enum ControlMode
    {
        APP_CONTROL, // 程序控制模式
        UI_CONTROL   // ImGui控制模式
    };

    // 存储鼠标位置状态
    struct MouseState
    {
        double uiX, uiY;   // UI 模式下的鼠标位置
        double appX, appY; // APP 模式下的虚拟鼠标位置
    };

    struct ScrollState
    {
        double xoffset, yoffset;
    };

    struct MouseButtonState
    {
        int button;
        int action;
        int mods;
    };

    inline static MouseButtonState mouseButtonState{};
    inline static MouseState mouseState{};
    inline static ScrollState scrollState{};
    inline static ControlMode currentMode = APP_CONTROL;

    // 内部逻辑
private:
    InputHandler() {}
    static void toggleControlMode(GLFWwindow *window)
    {
        if (currentMode == APP_CONTROL) // APP TO UI
        {

            // 显示鼠标指针
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            // 恢复 UI 模式的鼠标位置
            glfwSetCursorPos(window, mouseState.uiX, mouseState.uiY);
            // ImGui接管输入
            ImGuiIO &io = ImGui::GetIO();
            io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
            currentMode = UI_CONTROL;
        }
        else // UI TO APP
        {
            // 隐藏鼠标指针
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            // 恢复 APP 模式的虚拟鼠标位置
            glfwSetCursorPos(window, mouseState.appX, mouseState.appY);
            // ImGui忽略输入
            ImGuiIO &io = ImGui::GetIO();
            currentMode = APP_CONTROL;
        }
    }

    static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
        {
            toggleControlMode(window);
            if (currentMode == UI_CONTROL)
            {
                // ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
            }
        }
        else
        {
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(window, true);
        }
    }

    static void mouse_callback(GLFWwindow *window, double xpos, double ypos)
    {
        if (currentMode == UI_CONTROL)
        {
            mouseState.uiX = xpos;
            mouseState.uiY = ypos;
        }
        else
        {
            mouseState.appX = xpos;
            mouseState.appY = ypos;
        }
    }

    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
    {
        if (currentMode == UI_CONTROL)
        {
            // ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
        }
        else
        {
            scrollState.xoffset = xoffset;
            scrollState.yoffset = yoffset;
        }
    }

    static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
    {
        if (currentMode == UI_CONTROL)
        {
            // ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
        }
        else
        {
            // 处理程序的鼠标按钮输入
            // ...
            mouseButtonState = {button, action, mods};
        }
    }

public:
    // 对外开放接口
    static void processInput(GLFWwindow *window, Camera &cam)
    {
        static int movement;
        if (currentMode == APP_CONTROL)
        {
            // cam.genPositionfrom(window);

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                movement |= Camera::Movement::forward;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                movement |= Camera::Movement::backward;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                movement |= Camera::Movement::left;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                movement |= Camera::Movement::right;
            cam.genPositionfrom(window, movement);
            movement = Camera::Movement::none;

            cam.genDirectionfrom(mouseState.appX, mouseState.appY);
            cam.genZoomfrom(scrollState.yoffset);
            scrollState = {0, 0};
        }
    }
    static void bindWindow(GLFWwindow *window)
    {
        glfwMakeContextCurrent(window);
        glfwSetKeyCallback(window, key_callback);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // 初始化鼠标指针
        glfwGetCursorPos(window, &mouseState.appX, &mouseState.appY);
        glfwGetCursorPos(window, &mouseState.uiX, &mouseState.uiY);
    }
    static void resetInputState()
    {
        mouseState = {};
        scrollState = {};
    }
};
