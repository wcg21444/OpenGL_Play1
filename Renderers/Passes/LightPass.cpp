#include <glad/glad.h>
#include "../../Shading/Shader.hpp"
#include "../../ShaderGUI.hpp"
#include "../../Utils/Random.hpp"
#include "LightPass.hpp"

LightPass::LightPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path)
    : Pass(_vp_width, _vp_height, _vs_path, _fs_path),
      shaderUI(std::make_unique<LightShaderUI>())
{
    initializeGLResources();
    contextSetup();
}
LightPass::~LightPass() = default;
void LightPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);

    lightPassTex.Generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
    shadowNoiseTex.Generate(8, 8, GL_RGBA16F, GL_RGB, GL_FLOAT, NULL);
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
                       float pointLightFar)
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

    /****************************************阴影贴图输入**************************************************/
    // TODO Shader 多DirLight 渲染

    for (size_t i = 0; i < MAX_LIGHTS; ++i)
    {
        shaders.setTextureAuto(0, GL_TEXTURE_CUBE_MAP, 0, "shadowCubeMaps[" + std::to_string(i) + "]"); // 给sampler数组赋空
    }
    for (size_t i = 0; i < MAX_DIR_LIGHTS; ++i)
    {
        shaders.setTextureAuto(0, GL_TEXTURE_CUBE_MAP, 0, "dirDepthMap[" + std::to_string(i) + "]"); // 给sampler数组赋空
    }
    // shaders.setTextureAuto(cubemapTexture, GL_TEXTURE_CUBE_MAP, 31, "skybox");

    /****************************************点光源输入**************************************************/
    shaders.setInt("numPointLights", static_cast<int>(pointLights.size()));
    for (size_t i = 0; i < pointLights.size(); ++i)
    {
        pointLights[i].setToShaderLightArray(shaders, i);
    }
    /****************************************方向光源输入**************************************************/
    // TODO Shader 多DirLight 渲染

    // shaders.setUniform3fv("dirLightPos", dirLights[0].position);
    // shaders.setUniform3fv("dirLightIntensity", dirLights[0].combIntensity);
    // shaders.setMat4("dirLightSpaceMatrix", dirLights[0].lightSpaceMatrix);
    shaders.setInt("numDirLights", static_cast<int>(dirLights.size()));
    for (size_t i = 0; i < dirLights.size(); ++i)
    {
        dirLights[i].setToShaderLightArray(shaders, i);
    }
    /****************************************视口设置****************************************************/
    shaders.setInt("width", vp_width);
    shaders.setInt("height", vp_height);
    /****************************************摄像机设置**************************************************/
    shaders.setUniform3fv("eyePos", cam.getPosition());
    shaders.setUniform3fv("eyeFront", cam.getFront());
    shaders.setUniform3fv("eyeUp", cam.getUp());
    shaders.setFloat("farPlane", cam.farPlane);
    shaders.setFloat("nearPlane", cam.nearPlane);
    shaders.setFloat("pointLightFar", pointLightFar);
    shaders.setFloat("fov", cam.fov);
    cam.setViewMatrix(shaders);

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

    Renderer::DrawQuad();
}
