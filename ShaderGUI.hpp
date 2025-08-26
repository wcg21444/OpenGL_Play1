#pragma once
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <glm/glm.hpp>

/*******************************************************************************/
// Shader 参数 用户 交互界面
// 使用指针指向UI
// 参数变量名规则: 与 shader中 uniform变量名保持一致
// 组件注册到"ShadersGUI"

class SSAOShaderUI
{
public:
    float radius = 1.f;
    float intensity = 0.5f;
    float bias = -0.2f;
    int kernelSize = 64;
    void render()
    {
        ImGui::Begin("ShadersGUI");
        {
            if (ImGui::CollapsingHeader("SSAO", ImGuiTreeNodeFlags_None))
            {
                ImGui::PushItemWidth(100.f);
                ImGui::SliderInt("KernelSize", &kernelSize, 1, 64);
                ImGui::SliderFloat("Radius", &radius, 0.f, 5.f);
                ImGui::SliderFloat("Intensity", &intensity, 0.f, 2.f);
                ImGui::SliderFloat("AOBias", &bias, -0.5f, 0.5f);
                ImGui::PopItemWidth();
            }
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
        ImGui::Begin("ShadersGUI");
        {
            if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_None))
            {
                ImGui::Text("AmbientLight");
                ImGui::ColorEdit3("##AmbientLight", glm::value_ptr(ambientLight));
                ImGui::PushItemWidth(100.f);
                ImGui::SliderInt("Samples", &samplesNumber, 1, 128);
                ImGui::DragFloat("BlurRadius", &blurRadius, 0.01f, 0.0f, 1.0f);
                ImGui::PopItemWidth();
            }
        }
        ImGui::End();
    }
};

class PostProcessShaderUI
{
public:
    float gamma = 1.8f;
    float HDRExposure = 1.7f;
    float vignettingStrength = 1.3f;
    float vignettingPower = 0.1f;
    void render()
    {
        ImGui::Begin("ShadersGUI");
        {
            if (ImGui::CollapsingHeader("PostProcess", ImGuiTreeNodeFlags_None))
            {
                ImGui::PushItemWidth(100.f);
                ImGui::SliderFloat("Gamma", &gamma, 0.0, 3);
                ImGui::SliderFloat("HDRExposure", &HDRExposure, 0.f, 3.f);
                ImGui::SliderFloat("VignettingStrength", &vignettingStrength, 0.f, 10.f);
                ImGui::SliderFloat("VignettingPower", &vignettingPower, 0.01f, 1.f);
                ImGui::PopItemWidth();
            }
        }
        ImGui::End();
    }
};

class BloomShaderUI
{
public:
    float radius = 2.0f;
    int blurAmount = 10;
    float bloomIntensity = 1.0f;
    float threshold = 0.9f;
    void render()
    {
        ImGui::Begin("ShadersGUI");
        {
            if (ImGui::CollapsingHeader("Bloom", ImGuiTreeNodeFlags_None))
            {
                ImGui::PushItemWidth(100.f);
                ImGui::SliderFloat("BloomIntensity", &bloomIntensity, 0.01f, 5.f);
                ImGui::SliderFloat("Radius", &radius, 0.01f, 5.f);
                ImGui::SliderFloat("Threshold", &threshold, 0.01f, 1.f);
                ImGui::SliderInt("BlurAmount", &blurAmount, 1, 30);
                ImGui::PopItemWidth();
            }
        }
        ImGui::End();
    }
};

class SkyGUI
{
public:
    inline static float skyHeight = 1e5;       // 大气层高度
    inline static float earthRadius = 6.371e6; // 地球半径
    inline static float skyIntensity = 1.f;    // 天空光强度
    inline static float HRayleigh = 8.5e3;
    inline static float HMie = 1e3;
    inline static float atmosphereDensity = 1.f; // 大气密度
    inline static float MieDensity = 1.0f;
    inline static float gMie = 0.56f;
    inline static float absorbMie = 0.1f;
    inline static float MieIntensity = 1e-1;
    inline static glm::vec4 betaMie = glm::vec4(21e-6, 21e-6, 21e-6, 1.0f);
    inline static int maxStep = 32;
    inline static void render()
    {
        ImGui::Begin("ShadersGUI");
        {
            if (ImGui::CollapsingHeader("Sky", ImGuiTreeNodeFlags_None))
            {

                ImGui::PushItemWidth(100.f);
                ImGui::Text("BetaMie");
                ImGui::PushID("BetaMie");
                ImGui::DragFloat("R", &betaMie.r, 1.0e-7f, 1e-7f, 1e-4f, "%.5e");
                ImGui::DragFloat("G", &betaMie.g, 1.0e-7f, 1e-7f, 1e-4f, "%.5e");
                ImGui::DragFloat("B", &betaMie.b, 1.0e-7f, 1e-7f, 1e-4f, "%.5e");
                ImGui::PopID();
                ImGui::DragFloat("skyHeight", &skyHeight, 1e3f, 1e1f, 1e7f);
                ImGui::DragFloat("earthRadius", &earthRadius, 1e4f, 1e1f, 1e7f);
                ImGui::DragFloat("skyIntensity", &skyIntensity, 1e-1f, 0.0f, 1e3);
                ImGui::DragInt("maxStep", &maxStep, 1, 1, 128);
                ImGui::DragFloat("HRayleigh", &HRayleigh, 10.f, 0.0f, 1e5);
                ImGui::DragFloat("HMie", &HMie, 2.f, 0.0f, 1e4);
                ImGui::DragFloat("AtmosphereDensity", &atmosphereDensity, 0.05f, 0.0f, 1e2);
                ImGui::DragFloat("MieDensity", &MieDensity, 0.05f, 0.0f, 1e2);
                ImGui::DragFloat("gMie", &gMie, 0.01f, 0.0f, 1.f);
                ImGui::DragFloat("absorbMie", &absorbMie, 0.01f, 1e-3, 1e1);
                ImGui::DragFloat("MieIntensity", &MieIntensity, 0.01f, 1e-2, 1e2);
                ImGui::PopItemWidth();
            }
        }
        ImGui::End();
    }
};