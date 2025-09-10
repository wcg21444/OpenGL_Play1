#pragma once
#include "../imgui/imgui.h"
#include "../imgui/backends/imgui_impl_glfw.h"
#include "../imgui/backends/imgui_impl_opengl3.h"
#include <glm/glm.hpp>
#include "../imguizmo/ImGuizmo.h"

#include "../Shading/Texture.hpp"

//这是一个静态类. 管理渲染最终输出
class RenderOutputManager{
public:
    //渲染到GUIDocking窗口. 返回窗口当前大小
    inline static ImVec2 RenderToDockingWindow(const TextureID outputTexture,const std::string& windowName)
    {
        static ImVec2 size;
        ImGui::Begin(windowName.c_str(), 0);
        {

            ImGui::BeginChild("Output");

            size = ImGui::GetContentRegionAvail();

            ImVec2 pos = ImGui::GetCursorScreenPos();
            auto renderWindowPMin = ImVec2(pos.x, pos.y);
            auto renderWindowPMax = ImVec2(pos.x + size.x, pos.y + size.y);

            ImGui::GetWindowDrawList()->AddImage(
                (ImTextureID)(intptr_t)outputTexture,
                renderWindowPMin,
                renderWindowPMax,
                ImVec2(0, 1),
                ImVec2(1, 0));
            ImGui::EndChild();
        }
        ImGui::End();
        return size;
    }
    

};