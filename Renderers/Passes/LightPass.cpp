#include <glad/glad.h>
#include "../../Shading/Shader.hpp"
#include "../../ShaderGUI.hpp"
#include "../../Utils/Random.hpp"
#include "LightPass.hpp"
#include "../../GUI.hpp"

LightPass::LightPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path)
    : Pass(_vp_width, _vp_height, _vs_path, _fs_path),
      shaderSetting(std::make_unique<LightShaderSetting>())
{
    initializeGLResources();
    contextSetup();
}
LightPass::~LightPass()
{
    cleanUpGLResources();
}
void LightPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);

    lightPassTex.generate(vp_width, vp_height, GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL);
    shadowNoiseTex.generate(8, 8, GL_RGBA16F, GL_RGB, GL_FLOAT, NULL);
}
void LightPass::cleanUpGLResources()
{
    glDeleteFramebuffers(1, &FBO);
}
void LightPass::contextSetup()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightPassTex.ID, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
unsigned int LightPass::getTextures()
{
    return lightPassTex.ID;
}
void LightPass::resize(int _width, int _height)
{
    vp_width = _width;
    vp_height = _height;

    lightPassTex.resize(vp_width, vp_height);
    // shadowNoiseTex.Resize(_width, _height);
    // TODO non Texure Resize
    contextSetup();
}
void LightPass::render(RenderParameters &renderParameters,
                       unsigned int gPosition,
                       unsigned int gNormal,
                       unsigned int gAlbedoSpec,
                       unsigned int skybox,
                       unsigned int transmittanceLUT,
                       unsigned int skyEnvmap)
{

    auto &[allLights, cam, scene, model, window] = renderParameters;
    auto &[pointLights, dirLights] = allLights;

    static auto shadowKernel = Random::GenerateShadowKernel(128);

    static const auto skyboxKernel = Random::GenerateSemiSphereKernel(32);

    auto noise = Random::GenerateNoise();
    shadowNoiseTex.setData(&noise[0]);

    glViewport(0, 0, vp_width, vp_height);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    shaders.use();

    /****************************************GBuffer输入**************************************************/
    shaders.setTextureAuto(gPosition, GL_TEXTURE_2D, 0, "gPosition");
    shaders.setTextureAuto(gNormal, GL_TEXTURE_2D, 0, "gNormal");
    shaders.setTextureAuto(gAlbedoSpec, GL_TEXTURE_2D, 0, "gAlbedoSpec");

    /****************************************阴影噪声输入**************************************************/
    shaders.setTextureAuto(shadowNoiseTex.ID, GL_TEXTURE_2D, 0, "shadowNoiseTex");
    /****************************************天空盒输入**************************************************/
    shaders.setTextureAuto(skybox, GL_TEXTURE_CUBE_MAP, 0, "skybox");
    shaders.setTextureAuto(skyEnvmap, GL_TEXTURE_CUBE_MAP, 0, "skyEnvmap");

    /****************************************点光源输入**************************************************/
    LightSource::InitialzeShaderLightArray(shaders);
    shaders.setInt("numPointLights", static_cast<int>(pointLights.size()));
    for (size_t i = 0; i < pointLights.size(); ++i)
    {
        pointLights[i].setToShaderLightArray(shaders, i);
    }
    shaders.setInt("usePCSS", GUI::DebugToggleUsePCSS());
    /****************************************方向光源输入**************************************************/
    shaders.setInt("numDirLights", static_cast<int>(dirLights.size()));
    for (size_t i = 0; i < dirLights.size(); ++i)
    {
        dirLights[i].setToShaderLightArray(shaders, i);
    }
    shaders.setInt("useBias", 0);
    if (GUI::DebugToggleUseBias())
    {
        shaders.setInt("useBias", 1);
    }
    shaders.setInt("useVSSM", 0);
    if (GUI::DebugToggleUseVSSM())
    {
        shaders.setInt("useVSSM", 1);
    }
    shaders.setFloat("VSSMKernelSize", GUI::DebugVSSMKernelSize());
    /****************************************视口设置****************************************************/
    shaders.setInt("width", vp_width);
    shaders.setInt("height", vp_height);
    /****************************************摄像机设置**************************************************/
    cam.setToShader(shaders);
    /****************************************天空设置*****************************************************/
    shaders.setTextureAuto(transmittanceLUT, GL_TEXTURE_2D, 0, "transmittanceLUT");
    SkySetting::SetShaderUniforms(shaders);
    SkySetting::RenderUI();

    /****************************************采样器设置**************************************************/
    for (unsigned int i = 0; i < shadowKernel.size(); ++i)
    {
        shaders.setUniform3fv(std::format("shadowSamples[{}]", i), shadowKernel[i]);
    }
    for (unsigned int i = 0; i < skyboxKernel.size(); ++i)
    {
        shaders.setUniform3fv(std::format("skyboxSamples[{}]", i), skyboxKernel[i]);
    }
    shaderSetting->setShaderUniforms(shaders);
    shaderSetting->renderUI();
    /*****************************************RayMarching设置************************************************* */

    Renderer::DrawQuad();
}
