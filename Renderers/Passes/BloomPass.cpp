#include "BloomPass.hpp"
#include "../../ShaderGUI.hpp"


BloomPass::BloomPass(int _vp_width, int _vp_height, std::string _vs_path,
                     std::string _fs_path)
    : Pass(_vp_width, _vp_height, _vs_path, _fs_path),
      blurPass(_vp_width, _vp_height, "Shaders/screenQuad.vs", "Shaders/PostProcess/gaussianBlur.fs"),
      blurPass1(_vp_width / 2, _vp_height / 2, "Shaders/screenQuad.vs", "Shaders/PostProcess/gaussianBlur.fs"),
      blurPass2(_vp_width / 4, _vp_height / 4, "Shaders/screenQuad.vs", "Shaders/PostProcess/gaussianBlur.fs"),
      blurPass3(_vp_width / 8, _vp_height / 8, "Shaders/screenQuad.vs", "Shaders/PostProcess/gaussianBlur.fs"),
      blurPass4(_vp_width / 16, _vp_height / 16, "Shaders/screenQuad.vs", "Shaders/PostProcess/gaussianBlur.fs"),
      shaderUI(std::make_unique<BloomShaderUI>())

{
    initializeGLResources();
    contextSetup();
}

BloomPass::~BloomPass()
{
    cleanUpGLResources();
}

void BloomPass::initializeGLResources()
{
    // 生成帧缓冲对象 (FBO)
    glGenFramebuffers(1, &FBO);
    // 生成用于 Bloom 预处理的浮点纹理，以支持 HDR
    bloomPassTex0.SetFilterMin(GL_LINEAR);
    bloomPassTex0.Generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);

    bloomPassTex1.SetFilterMin(GL_LINEAR);
    bloomPassTex1.Generate(vp_width / 2, vp_height / 2, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);

    bloomPassTex2.SetFilterMin(GL_LINEAR);
    bloomPassTex2.Generate(vp_width / 4, vp_height / 4, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);

    bloomPassTex3.SetFilterMin(GL_LINEAR);
    bloomPassTex3.Generate(vp_width / 8, vp_height / 8, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);

    bloomPassTex4.SetFilterMin(GL_LINEAR);
    bloomPassTex4.Generate(vp_width / 16, vp_height / 16, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
}

void BloomPass::cleanUpGLResources()
{
    glDeleteFramebuffers(1, &FBO);
}

void BloomPass::contextSetup()
{

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomPassTex0.ID, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "ERROR::BLOOM_PASS::Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    blurPass.contextSetup();
    blurPass1.contextSetup();
    blurPass2.contextSetup();
    blurPass3.contextSetup();
    blurPass4.contextSetup();
}

void BloomPass::resize(int _width, int _height)
{
    vp_width = _width;
    vp_height = _height;

    bloomPassTex0.Resize(vp_width, vp_height);
    bloomPassTex1.Resize(vp_width / 2, vp_height / 2);
    bloomPassTex2.Resize(vp_width / 4, vp_height / 4);
    bloomPassTex3.Resize(vp_width / 8, vp_height / 8);
    bloomPassTex4.Resize(vp_width / 16, vp_height / 16);

    contextSetup();
    blurPass.resize(vp_width, vp_height);
    blurPass1.resize(vp_width / 2, vp_height / 2);
    blurPass2.resize(vp_width / 4, vp_height / 4);
    blurPass3.resize(vp_width / 8, vp_height / 8);
    blurPass4.resize(vp_width / 16, vp_height / 16);
}

void BloomPass::reloadCurrentShaders()
{
    // 重新加载 Bloom 预处理着色器
    shaders = Shader(vs_path.c_str(), fs_path.c_str(), gs_path.c_str());
    // 重新设置上下文
    contextSetup();
    // 递归调用，重新加载高斯模糊着色器
    blurPass.reloadCurrentShaders();
    blurPass1.reloadCurrentShaders();
    blurPass2.reloadCurrentShaders();
    blurPass3.reloadCurrentShaders();
    blurPass4.reloadCurrentShaders();
}

void BloomPass::render(unsigned int screenTex)
{

    shaderUI->render();

    // 渲染bloomPassTex0
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomPassTex0.ID, 0);

    glViewport(0, 0, vp_width, vp_height);

    shaders.use();

    // 传递视口尺寸 uniform
    shaders.setInt("width", vp_width);
    shaders.setInt("height", vp_height);

    shaders.setUniform("threshold", shaderUI->threshold);
    shaders.setUniform("bloomIntensity", shaderUI->bloomIntensity);

    shaders.setTextureAuto(screenTex, GL_TEXTURE_2D, 0, "screenTex");

    Renderer::DrawQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 多重降采样高斯模糊
    blurPass1.render(bloomPassTex0.ID, shaderUI->blurAmount, shaderUI->radius);
    blit(blurPass1.getOutputFBO(), bloomPassTex1.ID, vp_width / 2, vp_height / 2); // 将模糊降采样后的结果输出到下一级纹理

    blurPass2.render(bloomPassTex1.ID, shaderUI->blurAmount, shaderUI->radius); // 1/2sz  -down sample-> 1/4sz
    blit(blurPass2.getOutputFBO(), bloomPassTex2.ID, vp_width / 4, vp_height / 4);

    blurPass3.render(bloomPassTex2.ID, shaderUI->blurAmount, shaderUI->radius);
    blit(blurPass3.getOutputFBO(), bloomPassTex3.ID, vp_width / 8, vp_height / 8);

    blurPass4.render(bloomPassTex3.ID, shaderUI->blurAmount, shaderUI->radius);
    blit(blurPass4.getOutputFBO(), bloomPassTex4.ID, vp_width / 16, vp_height / 16);
}