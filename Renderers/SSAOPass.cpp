#include "SSAOPass.hpp"
#include <glad/glad.h>
#include "../Shader.hpp"
#include "../utils/Random.hpp"

SSAOPass::SSAOPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path)
    : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
{
    initializeGLResources();
    contextSetup();
}
void SSAOPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);
    glGenTextures(1, &SSAOPassTex);
    glGenTextures(1, &noiseTexture);
}
void SSAOPass::contextSetup()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    {
        glBindTexture(GL_TEXTURE_2D, SSAOPassTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, vp_width, vp_height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SSAOPassTex, 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void SSAOPass::generateNoiseTexture()
{
    auto ssaoNoise = Random::GenerateNoise();
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 8, 8, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
unsigned int SSAOPass::getTextures()
{
    return SSAOPassTex;
}
void SSAOPass::render(RenderParameters &renderParameters,
                      unsigned int gPosition,
                      unsigned int gNormal,
                      unsigned int gAlbedoSpec,
                      unsigned int gViewPosition)
{
    auto &[lights, cam, scene, model, window] = renderParameters;
    shaderUI.render();
    auto ssaoKernel = Random::GenerateSSAOKernel();
    generateNoiseTexture();
    glViewport(0, 0, vp_width, vp_height);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    shaders.use();

    shaders.setInt("kernelSize", shaderUI.kernelSize);
    shaders.setFloat("radius", shaderUI.radius);
    shaders.setFloat("intensity", shaderUI.intensity);
    shaders.setFloat("bias", shaderUI.bias);

    for (unsigned int i = 0; i < 64; ++i)
    {
        shaders.setUniform3fv(std::format("samples[{}]", i), ssaoKernel[i]);
    }
    shaders.setTextureAuto(gPosition, GL_TEXTURE_2D, 0, "gPosition");
    shaders.setTextureAuto(gNormal, GL_TEXTURE_2D, 0, "gNormal");
    shaders.setTextureAuto(gAlbedoSpec, GL_TEXTURE_2D, 0, "gAlbedoSpec");
    shaders.setTextureAuto(gViewPosition, GL_TEXTURE_2D, 0, "gViewPosition");
    shaders.setTextureAuto(noiseTexture, GL_TEXTURE_2D, 0, "texNoise");

    shaders.setUniform3fv("eye_pos", cam.getPosition());
    shaders.setFloat("far_plane", cam.far);

    cam.setPerspectiveMatrix(shaders, vp_width, vp_height);
    cam.setViewMatrix(shaders);

    Renderer::DrawQuad();
}
