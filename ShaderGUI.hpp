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

    float skyHeight = 1e5;       // 大气层高度
    float earthRadius = 6.371e6; // 地球半径
    float skyIntensity = 2e4;    // 天空光强度
    float HRayleigh = 8.5e3;
    float HMie = 1e3;
    float atmosphereDensity = 2.f; // 大气密度
    float MieDensity = 1.0f;
    float gMie = 0.56f;
    float absorbMie = 0.1f;
    float MieIntensity = 1e-1;
    int maxStep = 32;
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
            if (ImGui::CollapsingHeader("Sky", ImGuiTreeNodeFlags_None))
            {
                ImGui::PushItemWidth(100.f);
                ImGui::DragFloat("skyHeight", &skyHeight, 1e3f, 1e1f, 1e7f);
                ImGui::DragFloat("earthRadius", &earthRadius, 1e4f, 1e1f, 1e7f);
                ImGui::DragFloat("skyIntensity", &skyIntensity, 1e2f, 0.0f, 1e7);
                ImGui::DragInt("maxStep", &maxStep, 1, 1, 128);
                ImGui::DragFloat("HRayleigh", &HRayleigh, 10.f, 0.0f, 1e5);
                ImGui::DragFloat("HMie", &HMie, 2.f, 0.0f, 1e4);
                ImGui::DragFloat("AtmosphereDensity", &atmosphereDensity, 0.05f, 0.0f, 1e2);
                ImGui::DragFloat("MieDensity", &MieDensity, 0.05f, 0.0f, 1e2);
                ImGui::DragFloat("gMie", &gMie, 0.01f, 0.0f, 1.f);
                ImGui::DragFloat("absorbMie", &absorbMie, 0.01f, 1e-3, 1);
                ImGui::DragFloat("MieIntensity", &MieIntensity, 0.01f, 1e-2, 1);
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