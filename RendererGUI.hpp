#pragma once
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <glm/glm.hpp>
#include "imguizmo/ImGuizmo.h"
/*******************************************************************************/
// Renderer 用户 交互界面
// 效果的开关设置交互
// 使用指针指向UI
// 参数变量名规则: 与 shader中 uniform 变量名保持一致
// 组件注册到"RendererGUI"
class GBufferRendererGUI
{
private:
    inline static ImVec2 renderWindowPMin;
    inline static ImVec2 renderWindowPMax;

public:
    bool togglePointShadow = true;
    bool toggleDirShadow = true;
    bool toggleGBuffer = true;
    bool toggleLight = true;
    bool toggleSSAO = true;
    bool toggleSSAOBlur = true;
    bool toggleHDR = true;
    bool toggleVignetting = true;
    bool toggleGammaCorrection = true;
    bool toggleBloom = true;
    bool toggleSkybox = true;
    bool toggleVSM = true;

    void render()
    {
        ImGui::Begin("RendererGUI");
        {
            ImGui::Checkbox("PointShadow", &togglePointShadow);
            ImGui::Checkbox("DirShadow", &toggleDirShadow);
            ImGui::Checkbox("SSAO", &toggleSSAO);
            ImGui::Checkbox("HDR", &toggleHDR);
            ImGui::Checkbox("Vignetting", &toggleVignetting);
            ImGui::Checkbox("GammaCorrection", &toggleGammaCorrection);
            ImGui::Checkbox("Bloom", &toggleBloom);
            ImGui::Checkbox("SkyBox", &toggleSkybox);
            ImGui::Checkbox("VSM", &toggleVSM);
        }
        ImGui::End();
    }

    ImVec2 getRenderWindowSize()
    {
        static ImVec2 size;
        ImGui::Begin("Scene");
        {
            ImGui::BeginChild("GameRender");
            size = ImGui::GetContentRegionAvail();
            ImGui::EndChild();
            ImGui::End();
        }
        return size;
    }
    void renderToDockingWindow(GLuint postProcessPassTex)
    {
        static ImVec2 size;
        ImGui::Begin("Scene", 0);
        {

            ImGui::BeginChild("GameRender");

            size = ImGui::GetContentRegionAvail();

            ImVec2 pos = ImGui::GetCursorScreenPos();
            renderWindowPMin = ImVec2(pos.x, pos.y);
            renderWindowPMax = ImVec2(pos.x + size.x, pos.y + size.y);

            ImGui::GetWindowDrawList()->AddImage(
                (ImTextureID)(intptr_t)postProcessPassTex,
                renderWindowPMin,
                renderWindowPMax,
                ImVec2(0, 1),
                ImVec2(1, 0));
            ImGui::EndChild();
        }
        ImGui::End();
    }
    void renderPassInspector(GLuint passTex)
    {
        static ImVec2 size;
        // 修改这个UI,使其能够显示多张Texture
        // 输入一个Texture vector
        // 根据数量调节每个texture尺寸.
        // 将所有texture加入drawlist, 并在正确位置展示
        ImGui::Begin("PassInspector", 0);
        {

            ImGui::BeginChild("PassRender");

            size = ImGui::GetContentRegionAvail();

            ImVec2 pos = ImGui::GetCursorScreenPos();
            renderWindowPMin = ImVec2(pos.x, pos.y);
            renderWindowPMax = ImVec2(pos.x + size.x, pos.y + size.y);

            ImGui::GetWindowDrawList()->AddImage(
                (ImTextureID)(intptr_t)passTex,
                renderWindowPMin,
                renderWindowPMax,
                ImVec2(0, 1),
                ImVec2(1, 0));
            ImGui::EndChild();
        }
        ImGui::End();
    }
    // 无法渲染Cubemap
    void renderPassInspector(const std::vector<GLuint> &passTextures)
    {
        ImGui::Begin("PassInspector", nullptr);
        {
            ImGui::BeginChild("PassRender");

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
};