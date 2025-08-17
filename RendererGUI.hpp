#pragma once
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <glm/glm.hpp>
/*******************************************************************************/
// Renderer 用户 交互界面
// 效果的开关设置交互
// 使用指针指向UI
// 参数变量名规则: 与 shader中 uniform 变量名保持一致
// 组件注册到"RendererGUI"
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
    bool toggleBloom = true;

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
        }
        ImGui::End();
    }
};