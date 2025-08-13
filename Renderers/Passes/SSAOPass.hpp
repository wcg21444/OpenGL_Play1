#pragma once
#include "Pass.hpp"
class SSAOShaderUI;

class SSAOPass : public Pass
{
private:
    unsigned int SSAOPassTex;
    unsigned int noiseTexture;
    std::unique_ptr<SSAOShaderUI> shaderUI;
    void initializeGLResources();

public:
    SSAOPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path);
    ~SSAOPass();
    void contextSetup() override;
    void generateNoiseTexture();
    unsigned int getTextures();
    void render(RenderParameters &renderParameters,
                unsigned int gPosition,
                unsigned int gNormal,
                unsigned int gAlbedoSpec,
                unsigned int gViewPosition);
};

class SSAOBlurPass : public Pass
{
private:
    unsigned int blurPassTex;

private:
    void initializeGLResources()
    {
        glGenFramebuffers(1, &FBO);
        glGenTextures(1, &blurPassTex);
    }

public:
    SSAOBlurPass(int _vp_width, int _vp_height, std::string _vs_path,
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
            glBindTexture(GL_TEXTURE_2D, blurPassTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, vp_width, vp_height, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurPassTex, 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    unsigned int getTextures() { return blurPassTex; }
    void render(unsigned int SSAOTex)
    {
        glViewport(0, 0, vp_width, vp_height);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        shaders.use();
        shaders.setTextureAuto(SSAOTex, GL_TEXTURE_2D, 0, "SSAOTex");
        Renderer::DrawQuad();
    }
};