#pragma once
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <glm/glm.hpp>

class SSAOShaderUI
{
public:
    float radius = 1.f;
    float intensity = 0.5f;
    float bias = -0.05f;
    int kernelSize = 64;

    void render()
    {
        ImGui::Begin("SSAOShaders");
        {
            ImGui::SliderInt("KernelSize", &kernelSize, 1, 64);
            ImGui::SliderFloat("radius", &radius, 0.f, 5.f);
            ImGui::SliderFloat("intensity", &intensity, 0.f, 2.f);
            ImGui::SliderFloat("bias", &bias, -0.2f, 0.2f);
        }
        ImGui::End();
    }
};
class LightShaderUI
{
public:
    glm::vec3 ambientLight{0.4f, 0.4f, 0.4f};
    float skyBoxScale = 3.5f;
    void render()
    {
        ImGui::Begin("SSAOShaders");
        {
            ImGui::SliderFloat("SkyBoxScale", &skyBoxScale, 0.f, 50.f);
            ImGui::DragFloat("AmbientLight R", &ambientLight.x, 0.01f);
            ImGui::DragFloat("AmbientLight G", &ambientLight.y, 0.01f);
            ImGui::DragFloat("AmbientLight B", &ambientLight.z, 0.01f);
        }
        ImGui::End();
    }
};