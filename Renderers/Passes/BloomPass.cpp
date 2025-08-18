#include "BloomPass.hpp"
#include "../../ShaderGUI.hpp"

BloomPass::BloomPass(int _vp_width, int _vp_height, std::string _vs_path,
                     std::string _fs_path)
    : Pass(_vp_width, _vp_height, _vs_path, _fs_path),
      blurPass(_vp_width, _vp_height, "Shaders/screenQuad.vs", "Shaders/PostProcess/gaussianBlur.fs"),
      shaderUI(std::make_unique<BloomShaderUI>())

{
    initializeGLResources();
    contextSetup();
}

BloomPass::~BloomPass()
{
    // 通常在此处释放资源，例如 glDeleteFramebuffers 等
    // 但在这个简单的类中，父类和成员的析构函数会处理大部分工作
}

void BloomPass::initializeGLResources()
{
    // 生成帧缓冲对象 (FBO)
    glGenFramebuffers(1, &FBO);
    // 生成用于 Bloom 预处理的浮点纹理，以支持 HDR
    bloomPassTex.Generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
}

void BloomPass::contextSetup()
{
    // 将 BloomPass 的 FBO 绑定为渲染目标
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    // 将生成的浮点纹理附加到 FBO 的颜色附件0上
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomPassTex.ID, 0);
    // 检查 FBO 完整性并解绑
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "ERROR::BLOOM_PASS::Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 递归调用，设置高斯模糊通道的上下文
    blurPass.contextSetup();
}

void BloomPass::resize(int _width, int _height)
{
    vp_width = _width;
    vp_height = _height;

    // 调整 Bloom 纹理的大小以匹配新的视口尺寸
    bloomPassTex.Resize(vp_width, vp_height);

    // 重新设置上下文以更新 FBO 附件
    contextSetup();

    // 递归调用，调整高斯模糊通道的大小
    blurPass.resize(vp_width, vp_height);
}

void BloomPass::reloadCurrentShaders()
{
    // 重新加载 Bloom 预处理着色器
    shaders = Shader(vs_path.c_str(), fs_path.c_str(), gs_path.c_str());
    // 重新设置上下文
    contextSetup();
    // 递归调用，重新加载高斯模糊着色器
    blurPass.reloadCurrentShaders();
}

unsigned int BloomPass::getTextures()
{
    // 返回最终模糊处理后的纹理，供下一个通道使用
    return blurPass.getTextures();
}

void BloomPass::render(unsigned int screenTex)
{
    shaderUI->render();
    // 设置视口
    glViewport(0, 0, vp_width, vp_height);
    // 绑定 BloomPass 的 FBO，准备进行预处理渲染
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    shaders.use();

    // 传递视口尺寸 uniform
    shaders.setInt("width", vp_width);
    shaders.setInt("height", vp_height);

    shaders.setUniform("threshold", shaderUI->threshold);
    shaders.setUniform("bloomIntensity", shaderUI->bloomIntensity);

    shaders.setTextureAuto(screenTex, GL_TEXTURE_2D, 0, "screenTex");

    Renderer::DrawQuad();

    blurPass.render(bloomPassTex.ID, shaderUI->blurAmount, shaderUI->radius);
}