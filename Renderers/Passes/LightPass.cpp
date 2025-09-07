#include <glad/glad.h>
#include "../../Shading/Shader.hpp"
#include "../../ShaderGUI.hpp"
#include "../../Utils/Random.hpp"
#include "LightPass.hpp"
#include "../../GUI.hpp"

LightPass::LightPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path)
    : Pass(_vp_width, _vp_height, _vs_path, _fs_path),
      shaderUI(std::make_unique<LightShaderUI>())
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

    lightPassTex.Generate(vp_width, vp_height, GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL);
    shadowNoiseTex.Generate(8, 8, GL_RGBA16F, GL_RGB, GL_FLOAT, NULL);
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

    lightPassTex.Resize(vp_width, vp_height);
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

    auto shadowKernel = Random::GenerateShadowKernel(128);

    static const auto skyboxKernel = Random::GenerateSemiSphereKernel(32);

    auto noise = Random::GenerateNoise();
    shadowNoiseTex.SetData(&noise[0]);

    shaderUI->render();

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
    shaders.setUniform3fv("eyePos", cam.getPosition());
    shaders.setUniform3fv("eyeFront", cam.getFront());
    shaders.setUniform3fv("eyeUp", cam.getUp());
    shaders.setFloat("farPlane", cam.farPlane);
    shaders.setFloat("nearPlane", cam.nearPlane);
    shaders.setFloat("fov", cam.fov);
    cam.setViewMatrix(shaders);
    /****************************************天空设置*****************************************************/
    shaders.setFloat("skyHeight", SkyGUI::skyHeight);
    shaders.setFloat("earthRadius", SkyGUI::earthRadius);
    shaders.setFloat("skyIntensity", SkyGUI::skyIntensity);
    shaders.setInt("maxStep", SkyGUI::maxStep);
    shaders.setFloat("HRayleigh", SkyGUI::HRayleigh);
    shaders.setFloat("HMie", SkyGUI::HMie);
    shaders.setFloat("atmosphereDensity", SkyGUI::atmosphereDensity);
    shaders.setFloat("MieDensity", SkyGUI::MieDensity);
    shaders.setFloat("gMie", SkyGUI::gMie);
    shaders.setFloat("absorbMie", SkyGUI::absorbMie);
    shaders.setFloat("MieIntensity", SkyGUI::MieIntensity);
    shaders.setUniform("betaMie", SkyGUI::betaMie);
    shaders.setTextureAuto(transmittanceLUT, GL_TEXTURE_2D, 0, "transmittanceLUT");

    SkyGUI::render();

    /****************************************采样器设置**************************************************/
    for (unsigned int i = 0; i < shadowKernel.size(); ++i)
    {
        shaders.setUniform3fv(std::format("shadowSamples[{}]", i), shadowKernel[i]);
    }
    for (unsigned int i = 0; i < skyboxKernel.size(); ++i)
    {
        shaders.setUniform3fv(std::format("skyboxSamples[{}]", i), skyboxKernel[i]);
    }
    /****************************************环境光设置**************************************************/
    shaders.setUniform3fv("ambientLight", shaderUI->ambientLight);
    /*****************************************阴影设置************************************************* */
    shaders.setFloat("blurRadius", shaderUI->blurRadius);
    shaders.setInt("n_samples", shaderUI->samplesNumber);
    /*****************************************RayMarching设置************************************************* */

    Renderer::DrawQuad();
}
