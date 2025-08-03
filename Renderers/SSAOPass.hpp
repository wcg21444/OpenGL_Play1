#pragma once
#include "Pass.hpp"
#include "../ShaderGUI.hpp"

class SSAOPass : public Pass
{
private:
    unsigned int SSAOPassTex;
    unsigned int noiseTexture;
    SSAOShaderUI shaderUI;
    void initializeGLResources();

public:
    SSAOPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path);
    void contextSetup() override;
    void generateNoiseTexture();
    unsigned int getTextures();
    void render(RenderParameters &renderParameters,
                unsigned int gPosition,
                unsigned int gNormal,
                unsigned int gAlbedoSpec,
                unsigned int gViewPosition);
};
