#pragma once
#include "Pass.hpp"
#include "../ShaderGUI.hpp"

class LightPass : public Pass
{
private:
    const int MAX_LIGHTS = 10;
    unsigned int lightPassTex;
    unsigned int shadowNoiseTex;
    LightShaderUI shaderUI;
    void initializeGLResources();

public:
    LightPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path);
    void contextSetup() override;
    void generateShadowNoiseTexture();
    unsigned int getTextures();
    void render(RenderParameters &renderParameters,
                unsigned int gPosition,
                unsigned int gNormal,
                unsigned int gAlbedoSpec,
                unsigned int ssaoTex,
                unsigned int skybox,
                float pointLightFar);
};
