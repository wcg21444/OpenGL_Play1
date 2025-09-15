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

class Shader;

class SSAOShaderSetting
{
public:
    float radius = 1.f;
    float intensity = 0.5f;
    float bias = -0.2f;
    int kernelSize = 64;
    void renderUI()
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

    void setShaderUniforms(Shader &shaders)
    {
        shaders.setFloat("radius", radius);
        shaders.setFloat("bias", bias);
        shaders.setFloat("intensity", intensity);
        shaders.setInt("kernelSize", kernelSize);
    }
};
class LightShaderSetting
{
public:
    glm::vec3 ambientLight{0.0f, 0.0f, 0.0f};
    int samplesNumber = 32;
    float blurRadius = 0.1f;
    void renderUI()
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

    void setShaderUniforms(Shader &shaders)
    {
        /****************************************环境光设置**************************************************/
        shaders.setUniform3fv("ambientLight", ambientLight);
        /*****************************************阴影设置************************************************* */
        shaders.setFloat("blurRadius", blurRadius);
        shaders.setInt("n_samples", samplesNumber);
    }
};

class PostProcessShaderSetting
{
public:
    float gamma = 2.2f;
    float HDRExposure = 1.1f;
    float vignettingStrength = 2.7f;
    float vignettingPower = 0.1f;
    void renderUI()
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
    void setShaderUniforms(Shader &shaders)
    {
        shaders.setUniform("gamma", gamma);
        shaders.setUniform("HDRExposure", HDRExposure);
        shaders.setUniform("vignettingStrength", vignettingStrength);
        shaders.setUniform("vignettingPower", vignettingPower);
    }
};

class BloomShaderSetting
{
public:
    float radius = 2.0f;
    int blurAmount = 10;
    float bloomIntensity = 1.0f;
    float threshold = 0.9f;
    void renderUI()
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

    void setShaderUniforms(Shader &shaders)
    {
        shaders.setUniform("threshold", threshold);
        shaders.setUniform("bloomIntensity",bloomIntensity);
    }
};

class SkySetting
{
public:
    inline static float skyHeight = 1e5;       // 大气层高度
    inline static float earthRadius = 6.371e6; // 地球半径
    inline static float skyIntensity = 1.f;    // 天空光强度
    inline static float HRayleigh = 8.5e3;
    inline static float HMie = 1.2e3;
    inline static float atmosphereDensity = 1.f; // 大气密度
    inline static float MieDensity = 1.0f;
    inline static float gMie = 0.56f;
    inline static float absorbMie = 0.1f;
    inline static float MieIntensity = 1e-1;
    inline static glm::vec4 betaMie = glm::vec4(21e-6, 21e-6, 21e-6, 1.0f);
    inline static glm::vec4 betaOzoneAbsorb = glm::vec4(0.650f, 1.881f, 0.085f, 1.0f) * 1e-6f;
    inline static glm::vec4 betaMieAbsorb = glm::vec4(2.5e-5, 4e-5, 1e-5, 1.0f);
    inline static float ozoneCenterHeight = 2.5e4;
    inline static float ozoneWidth = 1.0e4;
    inline static int maxStep = 72;
    inline static void SetShaderUniforms(Shader &shaders)
    {
        shaders.setFloat("skyHeight", SkySetting::skyHeight);
        shaders.setFloat("earthRadius", SkySetting::earthRadius);
        shaders.setFloat("skyIntensity", SkySetting::skyIntensity);
        shaders.setInt("maxStep", SkySetting::maxStep);
        shaders.setFloat("HRayleigh", SkySetting::HRayleigh);
        shaders.setFloat("HMie", SkySetting::HMie);
        shaders.setFloat("atmosphereDensity", SkySetting::atmosphereDensity);
        shaders.setFloat("MieDensity", SkySetting::MieDensity);
        shaders.setFloat("gMie", SkySetting::gMie);
        shaders.setFloat("absorbMie", SkySetting::absorbMie);
        shaders.setFloat("MieIntensity", SkySetting::MieIntensity);
        shaders.setUniform("betaMie", SkySetting::betaMie);
        shaders.setUniform("betaMieAbsorb", SkySetting::betaMieAbsorb);
        shaders.setUniform("betaOzoneAbsorb", SkySetting::betaOzoneAbsorb);
        shaders.setFloat("ozoneCenterHeight", SkySetting::ozoneCenterHeight);
        shaders.setFloat("ozoneWidth", SkySetting::ozoneWidth);
    }

    inline static void RenderUI()
    {
        ImGui::Begin("ShadersGUI");
        {
            if (ImGui::CollapsingHeader("Sky", ImGuiTreeNodeFlags_None))
            {

                ImGui::PushItemWidth(100.f);

                ImGui::SeparatorText("Mie");
                {
                    ImGui::Text("BetaMieAbsorb");
                    ImGui::PushID("BetaMieAbsorb");
                    ImGui::DragFloat("R", &betaMieAbsorb.r, 1.0e-7f, 1e-7f, 1e-4f, "%.2e");
                    ImGui::SameLine();
                    ImGui::DragFloat("G", &betaMieAbsorb.g, 1.0e-7f, 1e-7f, 1e-4f, "%.2e");
                    ImGui::SameLine();
                    ImGui::DragFloat("B", &betaMieAbsorb.b, 1.0e-7f, 1e-7f, 1e-4f, "%.2e");
                    ImGui::PopID();

                    ImGui::Text("BetaMie");
                    ImGui::PushID("BetaMie");
                    ImGui::DragFloat("R", &betaMie.r, 1.0e-7f, 1e-7f, 1e-4f, "%.2e");
                    ImGui::SameLine();
                    ImGui::DragFloat("G", &betaMie.g, 1.0e-7f, 1e-7f, 1e-4f, "%.2e");
                    ImGui::SameLine();
                    ImGui::DragFloat("B", &betaMie.b, 1.0e-7f, 1e-7f, 1e-4f, "%.2e");
                    ImGui::PopID();

                    ImGui::DragFloat("HMie", &HMie, 2.f, 0.0f, 1e4);

                    ImGui::DragFloat("MieDensity", &MieDensity, 0.05f, 0.0f, 1e2);
                    ImGui::DragFloat("gMie", &gMie, 0.01f, 0.0f, 1.f);
                    ImGui::DragFloat("absorbMie", &absorbMie, 0.01f, 1e-3, 1e1);
                    ImGui::DragFloat("MieIntensity", &MieIntensity, 0.01f, 1e-2, 1e2);
                }

                ImGui::SeparatorText("Ozone");
                {
                    ImGui::Text("BetaOzone");
                    ImGui::PushID("BetaOzone");
                    ImGui::DragFloat("R", &betaOzoneAbsorb.r, 1.0e-7f, 1e-7f, 1e-4f, "%.2e");
                    ImGui::SameLine();
                    ImGui::DragFloat("G", &betaOzoneAbsorb.g, 1.0e-7f, 1e-7f, 1e-4f, "%.2e");
                    ImGui::SameLine();
                    ImGui::DragFloat("B", &betaOzoneAbsorb.b, 1.0e-7f, 1e-7f, 1e-4f, "%.2e");
                    ImGui::PopID();

                    ImGui::DragFloat("OzoneCenterHeight", &ozoneCenterHeight, 1e1f, 1e1, 1e6);
                    ImGui::DragFloat("OzoneWidth", &ozoneWidth, 1e1f, 1e1, 1e5);
                }

                ImGui::SeparatorText("Rayleigh");
                {
                    ImGui::DragFloat("HRayleigh", &HRayleigh, 10.f, 0.0f, 1e5);
                    ImGui::DragFloat("AtmosphereDensity", &atmosphereDensity, 0.05f, 0.0f, 1e2);

                    ImGui::SeparatorText("Atmosphere");
                    ImGui::DragFloat("skyHeight", &skyHeight, 1e3f, 1e1f, 1e7f);
                    ImGui::DragFloat("earthRadius", &earthRadius, 1e4f, 1e1f, 1e7f);
                    ImGui::DragFloat("skyIntensity", &skyIntensity, 1e-1f, 0.0f, 1e3);
                    ImGui::DragInt("maxStep", &maxStep, 1, 1, 128);
                }

                ImGui::PopItemWidth();
            }
        }
        ImGui::End();
    }
};