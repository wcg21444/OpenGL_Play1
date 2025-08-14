#pragma once
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <glm/glm.hpp>

class GBufferRendererGUI
{
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

    void render()
    {
        ImGui::Begin("GBufferRenderer");
        {
            ImGui::Checkbox("PointShadow", &togglePointShadow);
            ImGui::Checkbox("DirShadow", &toggleDirShadow);
            ImGui::Checkbox("SSAO", &toggleSSAO);
            ImGui::Checkbox("HDR", &toggleHDR);
            ImGui::Checkbox("Vignetting", &toggleVignetting);
            ImGui::Checkbox("GammaCorrection", &toggleGammaCorrection);
        }
        ImGui::End();
    }
};