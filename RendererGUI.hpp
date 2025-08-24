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
};