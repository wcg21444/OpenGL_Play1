#pragma once
#include "Pass.hpp"
#include "GaussianBlurPass.hpp"
class BloomShaderUI;

class BloomPass : public Pass
{
private:
    Texture bloomPassTex0;
    Texture bloomPassTex1; // 1 / 2 sz
    Texture bloomPassTex2; // 1 / 4 sz
    Texture bloomPassTex3; // 1 / 8 sz
    Texture bloomPassTex4; // 1 / 16 sz

    GaussianBlurPass blurPass;
    GaussianBlurPass blurPass1; // 1 / 2 sz
    GaussianBlurPass blurPass2; // 1 / 4 sz
    GaussianBlurPass blurPass3; // 1 / 8 sz
    GaussianBlurPass blurPass4; // 1 / 16 sz

    std::unique_ptr<BloomShaderUI> shaderUI;

    void initializeGLResources() override;
    void contextSetup() override;

public:
    BloomPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path);
    ~BloomPass() override;

    void resize(int _width, int _height) override;
    void reloadCurrentShaders() override;

    auto getTextures()
    {
        return std::make_tuple(bloomPassTex0.ID,
                               bloomPassTex1.ID,
                               bloomPassTex2.ID,
                               bloomPassTex3.ID,
                               bloomPassTex4.ID);
    }
    unsigned int getBlurOutput()
    {
        return blurPass.getTextures();
    }
    void render(unsigned int screenTex);

    void blit(unsigned int srcFBO, unsigned int dstTex, int width, int height)
    {
        auto dstFBO = FBO;
        glBindFramebuffer(GL_FRAMEBUFFER, dstFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dstTex, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFBO);
        glBlitFramebuffer(
            0, 0, width, height, // 源矩形区域
            0, 0, width, height, // 目标矩形区域
            GL_COLOR_BUFFER_BIT, // 要复制的缓冲区
            GL_LINEAR            // 过滤模式
        );
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
};