#pragma once
#include "Pass.hpp"

class LightShaderUI;

class LightPass : public Pass
{
private:
    const int MAX_LIGHTS = 10;
    unsigned int lightPassTex;
    unsigned int shadowNoiseTex;
    std::unique_ptr<LightShaderUI> shaderUI;
    void initializeGLResources();

public:
    LightPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path);
    ~LightPass();
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
