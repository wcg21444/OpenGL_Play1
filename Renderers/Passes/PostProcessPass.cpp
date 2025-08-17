#include "PostProcessPass.hpp"
#include "../../ShaderGUI.hpp"

PostProcessPass::PostProcessPass(int _vp_width, int _vp_height, std::string _vs_path,
                                 std::string _fs_path)
    : Pass(_vp_width, _vp_height, _vs_path, _fs_path),
      shaderUI(std::make_unique<PostProcessShaderUI>())
{
    initializeGLResources();
    contextSetup();
}

PostProcessPass::~PostProcessPass()
{
}

void PostProcessPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);
    postProcessPassTex.Generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
}

void PostProcessPass::contextSetup()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postProcessPassTex.ID, 0);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessPass::resize(int _width, int _height)
{
    vp_width = _width;
    vp_height = _height;
    postProcessPassTex.Resize(_width, _height);
    contextSetup();
}

unsigned int PostProcessPass::getTextures()
{
    return postProcessPassTex.ID;
}

void PostProcessPass::render(unsigned int screenTex, unsigned int ssaoTex, unsigned int bloomTex)
{
    shaderUI->render();
    glViewport(0, 0, vp_width, vp_height);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    shaders.use();

    shaders.setInt("width", vp_width);
    shaders.setInt("height", vp_height);

    shaders.setUniform("gamma", shaderUI->gamma);
    shaders.setUniform("HDRExposure", shaderUI->HDRExposure);
    shaders.setUniform("vignettingStrength", shaderUI->vignettingStrength);
    shaders.setUniform("vignettingPower", shaderUI->vignettingPower);

    shaders.setTextureAuto(ssaoTex, GL_TEXTURE_2D, 0, "ssaoTex");
    shaders.setTextureAuto(screenTex, GL_TEXTURE_2D, 0, "screenTex");
    shaders.setTextureAuto(bloomTex, GL_TEXTURE_2D, 0, "bloomTex");

    Renderer::DrawQuad();
}