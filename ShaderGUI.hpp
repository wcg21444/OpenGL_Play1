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
    float bias = -0.2f;
    int kernelSize = 64;

    void render()
    {
        ImGui::Begin("SSAOShaders");
        {
            ImGui::SliderInt("KernelSize", &kernelSize, 1, 64);
            ImGui::SliderFloat("radius", &radius, 0.f, 5.f);
            ImGui::SliderFloat("intensity", &intensity, 0.f, 2.f);
            ImGui::SliderFloat("bias", &bias, -0.5f, 0.5f);
        }
        ImGui::End();
    }
};
class LightShaderUI
{
public:
    glm::vec3 ambientLight{0.0f, 0.0f, 0.0f};
    int samplesNumber = 32;
    float blurRadius = 0.1f;
    void render()
    {
        ImGui::Begin("SSAOShaders");
        {
            ImGui::ColorEdit3("AmbientLight", glm::value_ptr(ambientLight));
            ImGui::SliderInt("Samples", &samplesNumber, 1, 128);
            ImGui::DragFloat("BlurRadius", &blurRadius, 0.01f, 0.0f, 1.0f);
        }
        ImGui::End();
    }
};