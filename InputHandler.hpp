#pragma once

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include <memory>

class RenderParameters;
class RenderManager;
class Camera;

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

    inline static std::shared_ptr<RenderParameters> ptrRenderParameters;
    inline static std::shared_ptr<RenderManager> ptrRenderManager;

    // 内部逻辑
private:
    InputHandler() {}
    static void ToggleControlMode(GLFWwindow *window);
    static void WindowResizeCallback(GLFWwindow *window, int resizeWidth, int resizeHeight);
    static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void MouseCallback(GLFWwindow *window, double xpos, double ypos);
    static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

public:
    // 对外开放接口
    static void ProcessInput(GLFWwindow *window, Camera &cam);
    static void BindWindow(GLFWwindow *window);
    static void BindRenderApplication(
        std::shared_ptr<RenderParameters> _ptrRenderParameters,
        std::shared_ptr<RenderManager> _ptrRenderManager);
    static void ResetInputState();
};