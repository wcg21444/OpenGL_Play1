#include "LightPass.hpp"
#include <glad/glad.h>
#include "../Shader.hpp"
#include "../utils/Random.hpp"

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
                       unsigned int skyBox,
                       float shadow_far)
{

    auto &[lights, cam, scene, model, window] = renderParameters;
    auto shadowKernel = Random::GenerateShadowKernel();
    generateShadowNoiseTexture();
    shaderUI.render();
    glViewport(0, 0, vp_width, vp_height);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    shaders.use();

    // 绑定 GBuffer Texture 到Quad
    shaders.setTextureAuto(gPosition, GL_TEXTURE_2D, 0, "gPosition");
    shaders.setTextureAuto(gNormal, GL_TEXTURE_2D, 0, "gNormal");
    shaders.setTextureAuto(gAlbedoSpec, GL_TEXTURE_2D, 0, "gAlbedoSpec");
    shaders.setTextureAuto(ssaoTex, GL_TEXTURE_2D, 0, "ssaoTex");
    shaders.setTextureAuto(shadowNoiseTex, GL_TEXTURE_2D, 0, "shadowNoiseTex");
    shaders.setTextureAuto(skyBox, GL_TEXTURE_CUBE_MAP, 0, "skyBox");

    // shaders.setTextureAuto(cubemapTexture, GL_TEXTURE_CUBE_MAP, 31, "skyBox");

    shaders.setInt("numLights", static_cast<int>(lights.size()));
    for (size_t i = 0; i < MAX_LIGHTS; ++i)
    {
        shaders.setTextureAuto(0, GL_TEXTURE_CUBE_MAP, 0, "shadowCubeMaps[" + std::to_string(i) + "]"); // 给sampler数组赋空
    }
    for (size_t i = 0; i < lights.size(); ++i)
    {
        shaders.setUniform3fv("light_pos[" + std::to_string(i) + "]", lights[i].position);
        shaders.setUniform3fv("light_intensity[" + std::to_string(i) + "]", lights[i].intensity);
        if (lights[i].depthCubemap != 0)
        {
            shaders.setTextureAuto(lights[i].depthCubemap, GL_TEXTURE_CUBE_MAP, i + 3, "shadowCubeMaps[" + std::to_string(i) + "]");
        }
    }

    // sampler location是否会被覆盖?
    // 光照计算在 纹理计算之后,不用担心光照被纹理覆盖
    shaders.setUniform3fv("eye_pos", cam.getPosition());
    shaders.setUniform3fv("eye_front", cam.getFront());
    shaders.setUniform3fv("eye_up", cam.getUp());
    cam.setViewMatrix(shaders);

    for (unsigned int i = 0; i < shadowKernel.size(); ++i)
    {
        shaders.setUniform3fv(std::format("shadowSamples[{}]", i), shadowKernel[i]);
    }
    shaders.setFloat("far_plane", cam.far);
    shaders.setFloat("near_plane", cam.near);
    shaders.setFloat("shadow_far", shadow_far);
    shaders.setFloat("fov", cam.fov);
    shaders.setFloat("skybox_scale", shaderUI.skyBoxScale);
    shaders.setUniform3fv("ambient_light", shaderUI.ambientLight);

    Renderer::DrawQuad();
}
