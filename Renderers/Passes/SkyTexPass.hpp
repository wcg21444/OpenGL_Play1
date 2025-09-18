
#pragma once
#include "Pass.hpp"
#include "../../Shading/Texture.hpp"
class CubemapParameters;

class SkyTexPass : public Pass
{
    TextureCube skyCubemapTex;

private:
    void initializeGLResources();
    void cleanUpGLResources() override;

public:
    int cubemapSize;
    std::shared_ptr<CubemapParameters> cubemapParam;

public:
    SkyTexPass(std::string _vs_path, std::string _fs_path, /*  std::string _gs_path, */ int _cubemapSize);
    ~SkyTexPass();
    void contextSetup() override;
    
    void resize(int _width, int _height) override;

    void render(
        RenderParameters &renderParameters,
        unsigned int TransmittanceLUT);
    unsigned int getCubemap();
};

class TransmittanceLUTPass : public Pass
{
private:
    Texture2D lutTex;

private:
    void initializeGLResources() override
    {
        glGenFramebuffers(1, &FBO);
        lutTex.setFilterMin(GL_LINEAR);
        lutTex.setWrapMode(GL_CLAMP_TO_EDGE);
        lutTex.generate(vp_width, vp_height, GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL, false); // 使用32F,否则地平线出走样严重
    }
    void cleanUpGLResources() override
    {
        glDeleteFramebuffers(1, &FBO);
    }

public:
    TransmittanceLUTPass(int lutWidth, int lutHeight, std::string _vs_path,
                         std::string _fs_path)
        : Pass(lutWidth, lutHeight, _vs_path, _fs_path)
    {
        initializeGLResources();
        contextSetup();
    }
    ~TransmittanceLUTPass()
    {
        cleanUpGLResources();
    }
    void contextSetup() override
    {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lutTex.ID, 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    // 一般固定LUT尺寸
    void resize(int _width, int _height) override
    {
        vp_width = _width;
        vp_height = _height;
        lutTex.resize(_width, _height);
        contextSetup();
    }
    unsigned int getTextures()
    {
        return lutTex.ID;
    }
    void render();
};

class SkyEnvmapPass : public Pass
{
private:
    int cubemapSize;
    TextureCube skyEnvmapTex;

private:
    void initializeGLResources()
    {
        glGenFramebuffers(1, &FBO);
        skyEnvmapTex.generate(cubemapSize, cubemapSize, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_LINEAR, GL_LINEAR, false);
    }
    void cleanUpGLResources() override { glDeleteFramebuffers(1, &FBO); }

public:
    SkyEnvmapPass(std::string _vs_path, std::string _fs_path, int _cubemapSize)
        : Pass(0, 0, _vs_path, _fs_path), cubemapSize(_cubemapSize)
    {
        initializeGLResources();
        contextSetup();
    }
    ~SkyEnvmapPass() { cleanUpGLResources(); }
    void contextSetup() override {}

    void resize(int _width, int _height) override {}

    unsigned int getTextures()
    {
        return skyEnvmapTex.ID;
    }

    void render(
        unsigned int &skyTexture, std::shared_ptr<CubemapParameters> cubemapParam);
};
