#pragma once

#include <iostream>
#include <array>

#include "Shader.hpp"
#include "Camera.hpp"
#include "InputHandler.hpp"
#include "Objects/Object.hpp"
#include "LightSource.hpp"
#include "Renderer.hpp"
#include "DebugOutput.hpp"
#include "ModelLoader.hpp"
#include "FileBrowser.hpp"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

namespace GUI
{
    void NormalRendererShaderManager(RenderManager &renderMnager)
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
            renderMnager.normalRenderer.reloadShaders(
                Shader(path_buf1, path_buf2),
                Shader(path_buf3, path_buf4));
            DebugOutput::AddLog("Execute Shaders Reload\n");
        }
    }
    void LightHandle(LightSource &light_source)
    {
        static glm::vec3 lightIntensity = light_source.intensity;
        static glm::vec3 lightPosition = light_source.position;
        ImGui::DragFloat("LightIntensitiy.X", &lightIntensity.x, 1.f);
        ImGui::DragFloat("LightIntensitiy.Y", &lightIntensity.y, 1.f);
        ImGui::DragFloat("LightIntensitiy.Z", &lightIntensity.z, 1.f);

        ImGui::DragFloat("LightPosition.X", &lightPosition.x, 0.1f);
        ImGui::DragFloat("LightPosition.Y", &lightPosition.y, 0.1f);
        ImGui::DragFloat("LightPosition.Z", &lightPosition.z, 0.1f);
        light_source.intensity = lightIntensity;
        light_source.position = lightPosition;
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
        static const char *modes[] = {"Normal", "Debug_Depth", "Texture"};
        static int current_mode = 0;
        static int prev_mode = current_mode;
        ImGui::Combo("Mode", &current_mode, modes, IM_ARRAYSIZE(modes));

        if (current_mode != prev_mode)
        {
            prev_mode = current_mode;
            switch (current_mode)
            {
            case 0:
                renderManager.switchMode(RenderManager::Mode::normal);
                break;
            case 1:
                renderManager.switchMode(RenderManager::Mode::debug_depth);

                break;
            case 2:
                renderManager.switchMode(RenderManager::Mode::simple_texture);
                break;
            }
        }
    }

    void displaySceneHierarchy(std::vector<std::unique_ptr<Object>> &scene, int &selectedIndex)
    {
        ImGui::Begin("Scene Hierarchy");

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

        ImGui::End();
    }

    void loadModel(std::vector<std::unique_ptr<Object>> &scene)
    {
        static char textBuffer[256] = "Resource/LiveHouse/Studio/Bass.obj";
        ImGui::InputText("Files Path", textBuffer, IM_ARRAYSIZE(textBuffer));
        if (ImGui::Button("Load Files"))
        {
            auto &&model = ModelLoader::loadFile(std::string(textBuffer));
            // auto &&model = ModelLoader::loadFile("Resource/LiveHouse/Marshall Half Stack/stack.obj");
            model->setName("Bass");
            scene.push_back(std::move(model));
        }
    }

    void ShowFileBrowserExample()
    {
        static bool init = false;
        if (!init)
        {
            ImGuiMultiFileSelector::Initialize();
            init = true;
        }

        // 显示文件选择器窗口
        static bool showWindow = true;
        if (ImGuiMultiFileSelector::Show("File Selector", &showWindow))
        {
        }
    }
}
