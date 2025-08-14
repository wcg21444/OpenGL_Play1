#pragma once
#include "Pass.hpp"

class PostProcessPass : public Pass
{
private:
    void initializeGLResources()
    {
        glGenFramebuffers(1, &FBO);
        postProcessTex.Generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
    }
    Texture postProcessTex;

public:
    PostProcessPass(int _vp_width, int _vp_height, std::string _vs_path,
                    std::string _fs_path)
        : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
    {
        initializeGLResources();
        contextSetup();
    }
    void contextSetup()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postProcessTex.ID, 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    unsigned int getTextures() { return postProcessTex.ID; }
    void render(unsigned int screenTex, unsigned int ssaoTex)
    {
        glViewport(0, 0, vp_width, vp_height);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        shaders.use();
        shaders.setTextureAuto(ssaoTex, GL_TEXTURE_2D, 0, "ssaoTex");
        shaders.setTextureAuto(screenTex, GL_TEXTURE_2D, 0, "screenTex");

        Renderer::DrawQuad();
    }
};