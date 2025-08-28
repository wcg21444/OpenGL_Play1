#pragma once
#include "Pass.hpp"

class GaussianBlurPass : public Pass
{
private:
    unsigned int pingpongFBO[2];
    Texture pingpongTex[2];

private:
    void initializeGLResources() override
    {
        glGenFramebuffers(2, pingpongFBO);
        pingpongTex[0].Generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
        pingpongTex[1].Generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
    }

public:
    GaussianBlurPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path)
        : Pass(0, 0, _vs_path, _fs_path)
    {
        initializeGLResources();
        contextSetup();
    }
    ~GaussianBlurPass()
    {
        glDeleteFramebuffers(2, pingpongFBO);
    }

    void contextSetup() override
    {
        for (unsigned int i = 0; i < 2; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);

            glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongTex[i].ID, 0);
            pingpongTex[i].SetFilterMax(GL_LINEAR);
            pingpongTex[i].SetFilterMin(GL_LINEAR);
            pingpongTex[i].SetWrapMode(GL_CLAMP_TO_EDGE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    void resize(int _width, int _height) override
    {
        if (vp_width == _width &&
            vp_height == _height)
        {
            return;
        }
        vp_width = _width;
        vp_height = _height;

        pingpongTex[0].Resize(vp_width, vp_height);
        pingpongTex[1].Resize(vp_width, vp_height);

        contextSetup();
    }

    unsigned int getTextures() { return pingpongTex[1].ID; }

    //[in] originTex 原始Texture
    //      amount  模糊迭代次数
    //      radius  模糊半径
    void render(unsigned int originTex, int amount, float radius)
    {
        glViewport(0, 0, vp_width, vp_height);
        // CheckGLErrors();
        bool horizontal = true, first_iteration = true;
        shaders.use();

        shaders.setUniform("radius", radius);
        shaders.setInt("width", vp_width);
        shaders.setInt("height", vp_height);
        shaders.setTextureAuto(originTex, GL_TEXTURE_2D, 0, "image");

        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            // CheckGLErrors();
            shaders.setInt("horizontal", horizontal);
            glBindTexture(
                GL_TEXTURE_2D, first_iteration ? originTex : pingpongTex[!horizontal].ID); // 初次迭代使用输入BrightTex; 后续迭代使用模糊Tex
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;

            Renderer::DrawQuad();
        }
        // CheckGLErrors();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};