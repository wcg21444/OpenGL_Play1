#pragma once
#include "Pass.hpp"
#include "../Shading/Texture.hpp"
class SSAOShaderSetting;

class SSAOPass : public Pass
{
private:
    Texture2D SSAOPassTex;
    Texture2D noiseTex;

    std::unique_ptr<SSAOShaderSetting> shaderSetting;
    void initializeGLResources();
    void cleanUpGLResources() override;

public:
    SSAOPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path);
    ~SSAOPass();

    void contextSetup() override;

    void resize(int _width, int _height) override;

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
    unsigned int FBO;

    Texture2D blurPassTex;

private:
    void initializeGLResources() override;
    void cleanUpGLResources() override;

    void contextSetup();

public:
    SSAOBlurPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path);
    ~SSAOBlurPass();

    void resize(int _width, int _height) override;
    void render(unsigned int SSAOTex);

    unsigned int getTextures() { return blurPassTex.ID; }
};
