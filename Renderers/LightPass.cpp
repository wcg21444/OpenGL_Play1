#include "LightPass.hpp"
#include <glad/glad.h>
#include "../Shader.hpp"
#include "../utils/Random.hpp"

extern ParallelLight parallelLight; // 临时, 测试用

LightPass::LightPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path)
    : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
{
    initializeGLResources();
    contextSetup();
}
void LightPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);
    glGenTextures(1, &lightPassTex);
}
void LightPass::contextSetup()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glBindTexture(GL_TEXTURE_2D, lightPassTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, vp_width, vp_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightPassTex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void LightPass::generateShadowNoiseTexture()
{
    auto noise = Random::GenerateNoise();
    glBindTexture(GL_TEXTURE_2D, shadowNoiseTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 8, 8, 0, GL_RGB, GL_FLOAT, &noise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
unsigned int LightPass::getTextures()
{
    return lightPassTex;
}
void LightPass::render(RenderParameters &renderParameters,
                       unsigned int gPosition,
                       unsigned int gNormal,
                       unsigned int gAlbedoSpec,
                       unsigned int ssaoTex,
                       unsigned int skybox,
                       float pointLightFar)
{

    auto &[lights, cam, scene, model, window] = renderParameters;
    auto shadowKernel = Random::GenerateShadowKernel(128);
    static auto skyboxKernel = Random::GenerateSemiSphereKernel(16);
    generateShadowNoiseTexture();
    shaderUI.render();
    glViewport(0, 0, vp_width, vp_height);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    shaders.use();

    /****************************************GBuffer输入**************************************************/
    shaders.setTextureAuto(gPosition, GL_TEXTURE_2D, 0, "gPosition");
    shaders.setTextureAuto(gNormal, GL_TEXTURE_2D, 0, "gNormal");
    shaders.setTextureAuto(gAlbedoSpec, GL_TEXTURE_2D, 0, "gAlbedoSpec");

    /****************************************SSAO输入**************************************************/

    shaders.setTextureAuto(ssaoTex, GL_TEXTURE_2D, 0, "ssaoTex");
    /****************************************阴影噪声输入**************************************************/
    shaders.setTextureAuto(shadowNoiseTex, GL_TEXTURE_2D, 0, "shadowNoiseTex");
    /****************************************天空盒输入**************************************************/
    shaders.setTextureAuto(skybox, GL_TEXTURE_CUBE_MAP, 0, "skybox");
    shaders.setFloat("skyboxScale", shaderUI.skyboxScale);
    shaders.setInt("n_samples", shaderUI.samplesNumber);

    /****************************************阴影贴图输入**************************************************/
    shaders.setTextureAuto(parallelLight.depthMap, GL_TEXTURE_2D, 0, "dirDepthMap");

    for (size_t i = 0; i < MAX_LIGHTS; ++i)
    {
        shaders.setTextureAuto(0, GL_TEXTURE_CUBE_MAP, 0, "shadowCubeMaps[" + std::to_string(i) + "]"); // 给sampler数组赋空
    }
    // shaders.setTextureAuto(cubemapTexture, GL_TEXTURE_CUBE_MAP, 31, "skybox");

    /****************************************点光源输入**************************************************/
    shaders.setInt("numLights", static_cast<int>(lights.size()));
    for (size_t i = 0; i < lights.size(); ++i)
    {
        shaders.setUniform3fv("lightPos[" + std::to_string(i) + "]", lights[i].position);
        shaders.setUniform3fv("lightIntensity[" + std::to_string(i) + "]", lights[i].intensity);
        if (lights[i].depthCubemap != 0)
        {
            shaders.setTextureAuto(lights[i].depthCubemap, GL_TEXTURE_CUBE_MAP, i + 3, "shadowCubeMaps[" + std::to_string(i) + "]");
        }
    }
    /****************************************方向光源设置**************************************************/
    shaders.setUniform3fv("dirLightPos", parallelLight.position);
    shaders.setUniform3fv("dirLightIntensity", parallelLight.intensity);
    shaders.setMat4("dirLightSpaceMatrix", parallelLight.lightSpaceMatrix);

    /****************************************摄像机设置**************************************************/
    shaders.setUniform3fv("eyePos", cam.getPosition());
    shaders.setUniform3fv("eyeFront", cam.getFront());
    shaders.setUniform3fv("eyeUp", cam.getUp());
    shaders.setFloat("farPlane", cam.far);
    shaders.setFloat("nearPlane", cam.near);
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
    shaders.setUniform3fv("ambientLight", shaderUI.ambientLight);

    Renderer::DrawQuad();
}
