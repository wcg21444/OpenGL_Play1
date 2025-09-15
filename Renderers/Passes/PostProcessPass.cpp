#include "PostProcessPass.hpp"
#include "../../ShaderGUI.hpp"

PostProcessPass::PostProcessPass(int _vp_width, int _vp_height, std::string _vs_path,
                                 std::string _fs_path)
    : Pass(_vp_width, _vp_height, _vs_path, _fs_path),
      shaderSetting(std::make_unique<PostProcessShaderSetting>())
{
    initializeGLResources();
    contextSetup();
}

PostProcessPass::~PostProcessPass()
{
    cleanUpGLResources();
}

void PostProcessPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);
    postProcessPassTex.generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
}

void PostProcessPass::cleanUpGLResources()
{
    glDeleteFramebuffers(1, &FBO);
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
    postProcessPassTex.resize(_width, _height);
    contextSetup();
}

unsigned int PostProcessPass::getTextures()
{
    return postProcessPassTex.ID;
}

void PostProcessPass::render(unsigned int screenTex, unsigned int ssaoTex, const std::vector<unsigned int> &bloomTexArray)
{
    glViewport(0, 0, vp_width, vp_height);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    shaders.use();

    shaders.setInt("width", vp_width);
    shaders.setInt("height", vp_height);

    shaderSetting->setShaderUniforms(shaders);
    shaderSetting->renderUI();

    shaders.setTextureAuto(ssaoTex, GL_TEXTURE_2D, 0, "ssaoTex");
    shaders.setTextureAuto(screenTex, GL_TEXTURE_2D, 0, "screenTex");
    shaders.setTextureAuto(bloomTexArray[0], GL_TEXTURE_2D, 0, "bloomTex0");
    shaders.setTextureAuto(bloomTexArray[1], GL_TEXTURE_2D, 0, "bloomTex1");
    shaders.setTextureAuto(bloomTexArray[2], GL_TEXTURE_2D, 0, "bloomTex2");
    shaders.setTextureAuto(bloomTexArray[3], GL_TEXTURE_2D, 0, "bloomTex3");
    shaders.setTextureAuto(bloomTexArray[4], GL_TEXTURE_2D, 0, "bloomTex4");

    Renderer::DrawQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}