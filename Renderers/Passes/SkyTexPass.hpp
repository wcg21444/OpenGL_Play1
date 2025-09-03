
#pragma once
#include "Pass.hpp"
#include "../../Shading/Texture.hpp"
class SkyTexPass : public Pass
{
private:
    glm::mat4 camProj;
    float aspect;
    float nearPlane;
    int cubemapSize;
    TextureCube skyCubemapTex;

private:
    void initializeGLResources();

public:
    float farPlane;

public:
    SkyTexPass(std::string _vs_path, std::string _fs_path, /*  std::string _gs_path, */ int _cubemapSize);
    ~SkyTexPass();
    void contextSetup() override;

    void render(PointLight &light, std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model, glm::mat4 &lightSpaceMatrix);

    void resize(int _width, int _height) override;

    void render(
        RenderParameters &renderParameters,
        unsigned int TransmittanceLUT);
    unsigned int getCubemap();
};

class TransmittanceLUTPass : public Pass
{
private:
    Texture lutTex;

private:
    void initializeGLResources() override
    {
        glGenFramebuffers(1, &FBO);
        lutTex.SetFilterMin(GL_LINEAR);
        lutTex.SetWrapMode(GL_CLAMP_TO_EDGE);
        lutTex.Generate(vp_width, vp_height, GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL, false); // 使用32F,否则地平线出走样严重
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
        lutTex.Resize(_width, _height);
        contextSetup();
    }
    unsigned int getTextures()
    {
        return lutTex.ID;
    }
    void render();
};
