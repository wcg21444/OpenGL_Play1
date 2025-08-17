#include <glad/glad.h>
#include "../../Shading/Shader.hpp"
#include "../../ShaderGUI.hpp"
#include "../../utils/Random.hpp"
#include "SSAOPass.hpp"

SSAOPass::SSAOPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path)
    : Pass(_vp_width, _vp_height, _vs_path, _fs_path), shaderUI(std::make_unique<SSAOShaderUI>())
{
    initializeGLResources();
    contextSetup();
}
SSAOPass::~SSAOPass() = default;

void SSAOPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);
    SSAOPassTex.Generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
    noiseTex.Generate(8, 8, GL_RGBA16F, GL_RGB, GL_FLOAT, NULL);
}
void SSAOPass::contextSetup()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SSAOPassTex.ID, 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void SSAOPass::resize(int _width, int _height)
{
    vp_width = _width;
    vp_height = _height;
    SSAOPassTex.Resize(_width, _height);
    // noiseTex.Resize(_width, _height);
    contextSetup();
}
unsigned int SSAOPass::getTextures()
{
    return SSAOPassTex.ID;
}
void SSAOPass::render(RenderParameters &renderParameters,
                      unsigned int gPosition,
                      unsigned int gNormal,
                      unsigned int gAlbedoSpec,
                      unsigned int gViewPosition)
{
    auto &[allLights, cam, scene, model, window] = renderParameters;

    auto ssaoKernel = Random::GenerateSSAOKernel();

    auto ssaoNoise = Random::GenerateNoise();
    noiseTex.SetData(&ssaoNoise[0]);

    shaderUI->render();

    glViewport(0, 0, vp_width, vp_height);

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    shaders.use();

    /****************************************视口设置****************************************************/
    shaders.setInt("width", vp_width);
    shaders.setInt("height", vp_height);

    shaders.setInt("kernelSize", shaderUI->kernelSize);
    shaders.setFloat("radius", shaderUI->radius);
    shaders.setFloat("intensity", shaderUI->intensity);
    shaders.setFloat("bias", shaderUI->bias);

    for (unsigned int i = 0; i < 64; ++i)
    {
        shaders.setUniform3fv(std::format("samples[{}]", i), ssaoKernel[i]);
    }
    shaders.setTextureAuto(gPosition, GL_TEXTURE_2D, 0, "gPosition");
    shaders.setTextureAuto(gNormal, GL_TEXTURE_2D, 0, "gNormal");
    // shaders.setTextureAuto(gAlbedoSpec, GL_TEXTURE_2D, 0, "gAlbedoSpec");
    shaders.setTextureAuto(gViewPosition, GL_TEXTURE_2D, 0, "gViewPosition");
    shaders.setTextureAuto(noiseTex.ID, GL_TEXTURE_2D, 0, "texNoise");

    shaders.setUniform3fv("eyePos", cam.getPosition());
    shaders.setFloat("farPlane", cam.farPlane);

    cam.setPerspectiveMatrix(shaders, vp_width, vp_height);
    cam.setViewMatrix(shaders);

    Renderer::DrawQuad();
}

/************************Blur Pass******************************************************************/

SSAOBlurPass::SSAOBlurPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path)
    : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
{
    initializeGLResources();
    contextSetup();
}

SSAOBlurPass::~SSAOBlurPass()
{
    glDeleteFramebuffers(1, &FBO);
}

void SSAOBlurPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);
    blurPassTex.Generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
}

void SSAOBlurPass::resize(int _width, int _height)
{
    vp_width = _width;
    vp_height = _height;

    blurPassTex.Resize(vp_width, vp_height);

    contextSetup();
}

void SSAOBlurPass::contextSetup()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurPassTex.ID, 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SSAOBlurPass::render(unsigned int SSAOTex)
{
    glViewport(0, 0, vp_width, vp_height);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    shaders.use();

    /****************************************视口设置****************************************************/
    shaders.setInt("width", vp_width);
    shaders.setInt("height", vp_height);

    shaders.setTextureAuto(SSAOTex, GL_TEXTURE_2D, 0, "SSAOTex");

    Renderer::DrawQuad();
}