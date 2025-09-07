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
#include "Utils/DebugOutput.hpp"
#include "ModelLoader.hpp"
#include "FileBrowser.hpp"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/imgui_internal.h"

#include "imguizmo/ImGuizmo.h"

namespace GUI
{
    inline static std::shared_ptr<RenderParameters> ptrRenderParameters;
    inline static std::shared_ptr<RenderManager> ptrRenderManager;

    inline static ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::OPERATION(ImGuizmo::TRANSLATE);
    inline static ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::MODE(ImGuizmo::WORLD);
    inline static bool useSnap = false;
    inline static float snap[3] = {1.f, 1.f, 1.f};

    // 状态管理变量
    inline static bool modelLoadView = false; // 控制显示状态
    inline static int selectedIndex = -1;

    static void BindRenderApplication(
        std::shared_ptr<RenderParameters> _ptrRenderParameters,
        std::shared_ptr<RenderManager> _ptrRenderManager)
    {
        ptrRenderManager = _ptrRenderManager;
        ptrRenderParameters = _ptrRenderParameters;
    }

    static void RendererShaderManager(RenderManager &renderManager)
    {
        if (ImGui::Button("Reload Current Shaders"))
        {
            renderManager.reloadCurrentShaders();
            DebugOutput::AddLog("Execute Current Shaders Reload\n");
        }
    }

    static void ShadowRendererShaderManager(RenderManager &renderManager)
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

    static void EditTransform(Camera &camera, glm::mat4 &_matrix)
    {
        auto view = camera.getViewMatrix();
        auto projection = camera.getPerspectiveMatrix();
        float *cameraView = glm::value_ptr(view);
        float *cameraProjection = glm::value_ptr(projection);
        float *matrix = glm::value_ptr(_matrix);

        ImVec2 size;
        ImVec2 pos;
        ImGui::Begin("Scene", 0);
        {
            ImGui::BeginChild("GameRender");
            size = ImGui::GetWindowSize();
            pos = ImGui::GetWindowPos();
            ImGui::EndChild();
        }
        ImGui::End();
        ImGui::SetNextWindowSize(size);
        ImGui::SetNextWindowPos(pos);

        ImGuiIO &io = ImGui::GetIO();
        ImGui::Begin("Scene", 0);
        {

            ImGui::BeginChild("GameRender");
            ImGuizmo::SetAlternativeWindow(ImGui::GetCurrentWindow());

            float windowWidth = (float)ImGui::GetWindowWidth();
            float windowHeight = (float)ImGui::GetWindowHeight();
            ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
            // ImGuizmo::SetRect(ImGui::GetMainViewport()->Pos.x, ImGui::GetMainViewport()->Pos.y, io.DisplaySize.x, io.DisplaySize.y);
            ImGuizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL, useSnap ? &snap[0] : NULL);
            ImGui::EndChild();
        }
        ImGui::End();

        float matrixTranslation[3], matrixRotation[3], matrixScale[3];

        ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
        ImGui::SetNextItemWidth(150.f);
        ImGui::DragFloat3("Tr", matrixTranslation);
        ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);
        /*         if (mCurrentGizmoOperation != ImGuizmo::SCALE)
                {
                    if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
                        mCurrentGizmoMode = ImGuizmo::LOCAL;
                    ImGui::SameLine();
                    if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
                        mCurrentGizmoMode = ImGuizmo::WORLD;
                }
                if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
                    mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
                ImGui::SameLine();
                if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
                    mCurrentGizmoOperation = ImGuizmo::ROTATE; */
    }

    static void Transform(glm::mat4 &_matrix)
    {

        ImGui::Begin("Transform");
        {
            float *matrix = glm::value_ptr(_matrix);

            static float bounds[] = {-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f};
            static float boundsSnap[] = {0.1f, 0.1f, 0.1f};
            static bool boundSizing = false;
            static bool boundSizingSnap = false;

            if (ImGui::IsKeyPressed(ImGuiKey_T))
                mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
            if (ImGui::IsKeyPressed(ImGuiKey_E))
                mCurrentGizmoOperation = ImGuizmo::ROTATE;
            if (ImGui::IsKeyPressed(ImGuiKey_R)) // r Key
                mCurrentGizmoOperation = ImGuizmo::SCALE;
            if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
                mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
            ImGui::SameLine();
            if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
                mCurrentGizmoOperation = ImGuizmo::ROTATE;
            ImGui::SameLine();
            if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
                mCurrentGizmoOperation = ImGuizmo::SCALE;
            float matrixTranslation[3], matrixRotation[3], matrixScale[3];
            ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
            ImGui::InputFloat3("Tr", matrixTranslation);
            ImGui::InputFloat3("Rt", matrixRotation);
            ImGui::InputFloat3("Sc", matrixScale);
            ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);

            if (mCurrentGizmoOperation != ImGuizmo::SCALE)
            {
                if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
                    mCurrentGizmoMode = ImGuizmo::LOCAL;
                ImGui::SameLine();
                if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
                    mCurrentGizmoMode = ImGuizmo::WORLD;
            }

            if (ImGui::IsKeyPressed(ImGuiKey_S) && ImGui::IsKeyPressed(ImGuiKey_LeftAlt))
                useSnap = !useSnap;
            ImGui::Checkbox(" ", &useSnap);
            ImGui::SameLine();
            switch (mCurrentGizmoOperation)
            {
            case ImGuizmo::TRANSLATE:
                ImGui::InputFloat3("Snap", &snap[0]);
                break;
            case ImGuizmo::ROTATE:
                ImGui::InputFloat("Angle Snap", &snap[0]);
                break;
            case ImGuizmo::SCALE:
                ImGui::InputFloat("Scale Snap", &snap[0]);
                break;
            }
            ImGui::End();
        }
    }

    static void LightHandle(LightSource &light_source)
    {
        auto &[lightColor, lightIntensity] = light_source.colorIntensity;
        glm::vec3 lightPosition = light_source.getPosition();

        ImGui::ColorEdit3("LightColor", glm::value_ptr(lightColor));
        ImGui::PushItemWidth(100.f);
        ImGui::DragFloat("Intensitiy", &lightIntensity, 0.1f, 0.f);
        ImGui::PopItemWidth();

        glm::mat4 translate = glm::translate(glm::identity<glm::mat4>(), light_source.getPosition());
        // Transform(translate);
        EditTransform(ptrRenderParameters->cam, translate);
        lightPosition = translate[3];
        light_source.setPosition(lightPosition);
    }
    static void LightSourceManage(Lights &lights)
    {
        auto &[pointLights, dirLights] = lights;
        static LightSource *selectedLight = nullptr;

        if (ImGui::BeginChild("##tree", ImVec2(300, 300), ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders))
        {
            static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable;
            if (ImGui::BeginTable("table1", 1, flags))
            {
                int i = 0;

                for (auto &pointLight : pointLights)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(std::format("PL-{}", i).c_str(), selectedLight == &pointLight))
                    {
                        selectedLight = &pointLight;
                    }
                    ++i;
                }
                i = 0;
                for (auto &dirLight : dirLights)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (ImGui::Selectable(std::format("DL-{}", i).c_str(), selectedLight == &dirLight))
                    {
                        selectedLight = &dirLight;
                    }
                    ++i;
                }
                ImGui::EndTable();
            }
            ImGui::EndChild();
        }

        ImGui::SameLine();
        ImGui::BeginGroup();
        if (selectedLight)
        {
            LightHandle(*selectedLight);
        }
        ImGui::EndGroup();
    }

    static glm::mat4 MatrixInputWithImGui(const char *label, const glm::mat4 &initial)
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

    static void RenderSwitchCombo(RenderManager &renderManager)
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

    static void displaySceneHierarchy(Scene &scene, int &selectedIndex)
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

    static void ModelLoadView()
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

    static void ShowSidebarToolbar(Scene &scene, RenderManager &renderManager, Lights &lights, glm::mat4 &model)
    {

        // 开始侧边栏窗口
        ImGui::Begin("Sidebar", nullptr);

        // 1. 灯光控制部分
        if (ImGui::CollapsingHeader("Light Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            auto &light = lights.pointLights[0];
            // GUI::LightHandle(light);
            GUI::LightSourceManage(lights);
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
        ImGui::Text("DeltaTime: %.3f ms", ImGui::GetIO().DeltaTime * 1e3);
        ImGui::End();
    }

    static void RenderTextureInspector(const std::vector<GLuint> &passTextures)
    {
        ImGui::Begin("TextureInspector", nullptr);
        {
            ImGui::BeginChild("TextureDisplay");

            // 获取可用内容区域的尺寸
            ImVec2 contentSize = ImGui::GetContentRegionAvail();

            // 计算行和列
            // 简单地假设我们以方形布局排列纹理，并找到最接近的整数开方根
            int numTextures = passTextures.size();
            if (numTextures == 0)
            {
                ImGui::Text("No textures to display.");
                ImGui::EndChild();
                ImGui::End();
                return;
            }

            int cols = static_cast<int>(std::ceil(std::sqrt(numTextures)));
            int rows = static_cast<int>(std::ceil(static_cast<float>(numTextures) / cols));

            // 计算每个纹理的显示尺寸
            float textureDisplayWidth = contentSize.x / cols;
            float textureDisplayHeight = contentSize.y / rows;
            ImVec2 textureSize = ImVec2(textureDisplayWidth, textureDisplayHeight);

            // 获取绘制列表
            ImDrawList *drawList = ImGui::GetWindowDrawList();

            // 遍历所有纹理并绘制
            for (int i = 0; i < numTextures; ++i)
            {
                GLuint textureID = passTextures[i];

                // 计算当前纹理的屏幕位置
                int col = i % cols;
                int row = i / cols;

                ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
                ImVec2 pos = ImVec2(
                    cursorScreenPos.x + col * textureDisplayWidth,
                    cursorScreenPos.y + row * textureDisplayHeight);

                // 绘制纹理
                drawList->AddImage(
                    (ImTextureID)(intptr_t)textureID,
                    pos,
                    ImVec2(pos.x + textureSize.x, pos.y + textureSize.y),
                    ImVec2(0, 1), // UV 坐标：左上角
                    ImVec2(1, 0)  // UV 坐标：右下角
                );
                // 在纹理的左上角添加文本标签
                // 文本内容为纹理在向量中的索引
                std::string label = "Index: " + std::to_string(i);

                // 调整文本位置，稍微留出一些边距
                ImVec2 textPos = ImVec2(pos.x + 5, pos.y + 5);

                // 绘制文本
                drawList->AddText(textPos, IM_COL32_WHITE, label.c_str());
            }

            ImGui::EndChild();
        }
        ImGui::End();
    }

    inline bool useVSSM = false;
    static bool DebugToggleUseVSSM()
    {
        ImGui::Begin("Debug");
        {
            ImGui::Checkbox("UseVSSM", &useVSSM);

            ImGui::End();
        }
        return useVSSM;
    }
    inline bool useBias = false;
    static bool DebugToggleUseBias()
    {
        ImGui::Begin("Debug");
        {
            ImGui::Checkbox("UseBias", &useBias);

            ImGui::End();
        }
        return useBias;
    }
    inline bool useComputeShaderAccelerate = true;
    static bool DebugToggleUseComputeShaderAccelerate()
    {
        ImGui::Begin("Debug");
        {
            ImGui::Checkbox("UseCSAccelerate", &useComputeShaderAccelerate);

            ImGui::End();
        }
        return useComputeShaderAccelerate;
    }
    static float DebugVSSMKernelSize()
    {
        static float VSSMKernelSize = 0.01f;
        ImGui::Begin("Debug");
        {
            ImGui::DragFloat("VSSM Kernel Size", &VSSMKernelSize, 0.01f, 0.0f);
        }
        ImGui::End();
        return VSSMKernelSize;
    }
}