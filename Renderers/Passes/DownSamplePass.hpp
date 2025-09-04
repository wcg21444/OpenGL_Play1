#pragma once
#include "Pass.hpp"

class DownSamplePass : public Pass
{
private:
    Texture downSampleTex;

private:
    void initializeGLResources() override
    {
        glGenFramebuffers(1, &FBO);
        downSampleTex.Generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
    }
    void cleanUpGLResources() override
    {
        glDeleteFramebuffers(1, &FBO);
    }

public:
    DownSamplePass(int _vp_width, int _vp_height, std::string _vs_path,
                   std::string _fs_path)
        : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
    {
        initializeGLResources();
        contextSetup();
    }
    ~DownSamplePass()
    {
        cleanUpGLResources();
    }

    void contextSetup() override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, downSampleTex.ID, 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void resize(int _width, int _height) override
    {
        vp_width = _width;
        vp_height = _height;
        downSampleTex.Resize(_width, _height);
        contextSetup();
    }
    unsigned int getTextures() { return downSampleTex.ID; }
    void render(unsigned int srcTex)
    {

        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glViewport(0, 0, vp_width, vp_height);

        shaders.use();
        shaders.setTextureAuto(srcTex, GL_TEXTURE_2D, 0, "srcTex");

        Renderer::DrawQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};
