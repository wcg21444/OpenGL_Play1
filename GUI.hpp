#pragma once

#include <iostream>
#include <array>
#include <future>
#include <thread>
#include <algorithm>

#include "Shader.hpp"
#include "Camera.hpp"
#include "Objects/Object.hpp"
#include "LightSource/LightSource.hpp"
#include "Renderers/RendererManager.hpp"
#include "utils/DebugOutput.hpp"
#include "ModelLoader.hpp"
#include "FileBrowser.hpp"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

namespace GUI
{
    // 状态管理变量
    inline static bool modelLoadView = false; // 控制显示状态
    int selectedIndex = -1;

    void RendererShaderManager(RenderManager &renderManager)
    {
        if (ImGui::Button("Reload Current Shaders"))
        {
            renderManager.reloadCurrentShaders();
            DebugOutput::AddLog("Execute Current Shaders Reload\n");
        }
    }

    void ShadowRendererShaderManager(RenderManager &renderManager)
    {
        static char path_buf1[256]{"Shaders/VertShader.vs"};
        static char path_buf2[256]{"Shaders/FragmShader.fs"};
        static char path_buf3[256]{"Shaders/PointShadow/point_shadow.vs"};
        static char path_buf4[256]{"Shaders/PointShadow/point_shadow.fs"};
        // if (ImGui::InputText("VS Path", path_buf1, sizeof(path_buf1)))
        // {
        // }
        // if (ImGui::InputText("FS Path", path_buf2, sizeof(path_buf2)))
        // {
        // }
        if (ImGui::InputText("PointShadow VS Path", path_buf3, sizeof(path_buf3)))
        {
        }
        if (ImGui::InputText("PointShadow FS Path", path_buf4, sizeof(path_buf4)))
        {
        }

        if (ImGui::Button("Reload Shaders"))
        {
            renderManager.reloadShadowShaders(
                Shader(path_buf1, path_buf2),
                Shader(path_buf3, path_buf4));
            DebugOutput::AddLog("Execute Shaders Reload\n");
        }
    }
    void LightHandle(PointLight &light_source)
    {
        static glm::vec3 lightColor;
        static float lightIntensity;
        static glm::vec3 lightPosition = light_source.position;

        ImGui::ColorEdit3("LightColor", glm::value_ptr(lightColor));
        ImGui::DragFloat("LightIntensitiy", &lightIntensity, 0.1f, 0.f);

        ImGui::DragFloat("LightPosition.X", &lightPosition.x, 0.1f);
        ImGui::DragFloat("LightPosition.Y", &lightPosition.y, 0.1f);
        ImGui::DragFloat("LightPosition.Z", &lightPosition.z, 0.1f);
        light_source.intensity = lightIntensity * lightColor;
        light_source.position = lightPosition;
    }
    void LightHandle(DirectionLight &light_source)
    {
        static glm::vec3 lightColor;
        static float lightIntensity;
        static glm::vec3 lightPosition = light_source.position;
        ImGui::ColorEdit3("DirLightColor", glm::value_ptr(lightColor));
        ImGui::DragFloat("DirLightIntensitiy", &lightIntensity, 0.01f, 0.f);

        ImGui::DragFloat("DirLightPosition.X", &lightPosition.x, 0.1f);
        ImGui::DragFloat("DirLightPosition.Y", &lightPosition.y, 0.1f);
        ImGui::DragFloat("DirLightPosition.Z", &lightPosition.z, 0.1f);
        light_source.intensity = lightIntensity * lightColor;
        light_source.updatePosition(lightPosition);
    }

    glm::mat4 MatrixInputWithImGui(const char *label, const glm::mat4 &initial)
    {
        // TODO : 将矩阵转置
        //  将GLM矩阵转换为ImGui可编辑的格式
        std::array<std::array<float, 4>, 4> matrix;
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                matrix[i][j] = initial[i][j];
            }
        }

        // ImGui矩阵编辑器
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen))
        {

            // 逐行编辑
            for (int row = 0; row < 4; ++row)
            {
                ImGui::PushID(row);
                if (ImGui::DragFloat4("", matrix[row].data(), 0.01f))
                {
                    ///
                }
                ImGui::PopID();
            }

            // 重置按钮
            if (ImGui::Button("Reset to Identity"))
            {
                for (auto &row : matrix)
                {
                    std::fill(row.begin(), row.end(), 0.0f);
                }
                for (int i = 0; i < 4; ++i)
                {
                    matrix[i][i] = 1.0f;
                }
            }

            ImGui::TreePop();
        }

        // 转换回GLM矩阵
        glm::mat4 result;
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                result[i][j] = matrix[i][j];
            }
        }
        return result;
    }

    void RenderSwitchCombo(RenderManager &renderManager)
    {
        static const char *modes[] = {"PointShadow", "ParrllelShadow", "DebugDepth", "Texture", "DepthPass", "GBuffer", "CubemapUnfold"};
        static int current_mode = 0;
        static int prev_mode = current_mode;
        ImGui::Combo("Mode", &current_mode, modes, IM_ARRAYSIZE(modes));

        if (current_mode != prev_mode)
        {
            prev_mode = current_mode;
            renderManager.switchMode(static_cast<RenderManager::Mode>(current_mode)); // current_mode == 状态触发器 static_cast<RenderManager::Mode>(current_mode) 是一种隐含的触发器->状态映射
        }
    }

    void displaySceneHierarchy(Scene &scene, int &selectedIndex)
    {

        // 列表显示所有对象
        for (int i = 0; i < scene.size(); ++i)
        {
            bool isSelected = (selectedIndex == i);
            if (ImGui::Selectable(scene[i]->name.c_str(), isSelected))
            {
                selectedIndex = i;
            }

            // 右键菜单
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Delete"))
                {
                    if (selectedIndex == i)
                        selectedIndex = -1;
                    scene.erase(scene.begin() + i);
                    ImGui::EndPopup();
                    break;
                }
                ImGui::EndPopup();
            }
        }
    }

    void ModelLoadView()
    {
        static FileSelector fileSelector;
        ImGui::Begin("ModelLoadView", &modelLoadView);
        {
            fileSelector.Show();
            if (ImGui::Button("Import Models"))
            {
                // 加载所有选中路径
                for (auto &path : fileSelector.GetAllPaths())
                {
                    ModelLoader::loadFile(std::string(path)); // UI只负责触发信号
                }
            }
            ImGui::End();
        }
    }

    void ShowSidebarToolbar(Scene &scene, RenderManager &renderManager, PointLight &light, glm::mat4 &model)
    {
        static float sidebar_width = 300.0f;
        static bool is_resizing = false;

        const ImGuiViewport *viewport = ImGui::GetMainViewport();

        auto side_bar_x = viewport->WorkPos.x + viewport->WorkSize.x - sidebar_width;
        auto side_bar_y = viewport->WorkPos.y;

        // 设置侧边栏位置和大小
        ImGui::SetNextWindowPos(ImVec2(side_bar_x, side_bar_y));
        ImGui::SetNextWindowSize(ImVec2(sidebar_width, viewport->WorkSize.y));
        // 设置窗口样式
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);   // 直角窗口
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f); // 无边框
        // 开始侧边栏窗口
        ImGui::Begin("Sidebar", nullptr,
                     ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoTitleBar);
        // 恢复样式
        ImGui::PopStyleVar(2);
        // 1. 灯光控制部分
        if (ImGui::CollapsingHeader("Light Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            GUI::LightHandle(light);
        }

        // 2. 模型矩阵控制
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            model = GUI::MatrixInputWithImGui("Model Matrix", model);
        }

        // 3. 渲染选项
        if (ImGui::CollapsingHeader("Render Options"))
        {
            GUI::RenderSwitchCombo(renderManager);
        }

        // 4. 场景层次结构
        if (ImGui::CollapsingHeader("Scene Hierarchy", ImGuiTreeNodeFlags_DefaultOpen))
        {
            GUI::displaySceneHierarchy(scene, GUI::selectedIndex);
        }

        // 5. 着色器管理
        if (ImGui::CollapsingHeader("Shader Settings"))
        {
            // GUI::ShadowRendererShaderManager(renderManager);
            GUI::RendererShaderManager(renderManager);
        }

        // 6. 调试输出
        if (ImGui::CollapsingHeader("Debug Output"))
        {
            DebugOutput::Draw();
        }

        ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
        ImGui::End();
        // 绘制可拖动的分隔条
        ImGui::SetNextWindowPos(ImVec2(side_bar_x, side_bar_y));
        ImGui::SetNextWindowSize(ImVec2(5, viewport->WorkSize.y));

        ImGui::Begin("Sidebar Resizer", nullptr,
                     ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_NoBackground |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoInputs);

        // 设置鼠标光标样式
        ImGui::End();

        // 处理拖动逻辑
        ImVec2 mouse_pos = ImGui::GetMousePos();
        bool is_hovering = mouse_pos.x >= side_bar_x - 5 &&
                           mouse_pos.x <= side_bar_x + 5 &&
                           mouse_pos.y >= side_bar_y &&
                           mouse_pos.y <= side_bar_y + viewport->WorkSize.y;

        (is_hovering || is_resizing) ? ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW) : ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && (is_hovering || is_resizing))
        {
            is_resizing = true;
            sidebar_width = std::abs(mouse_pos.x - viewport->WorkPos.x - viewport->WorkSize.x);
        }
        else
        {
            is_resizing = false;
        }
    }
}
