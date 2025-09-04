#pragma once
#include "Pass.hpp"
class LightShaderUI;

class LightPass : public Pass
{
private:
    Texture lightPassTex;
    Texture shadowNoiseTex;

    std::unique_ptr<LightShaderUI> shaderUI;
    void initializeGLResources();
    void cleanUpGLResources() override;

public:
    LightPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path);

    ~LightPass();

    void contextSetup() override;

    unsigned int getTextures();

    void resize(int _width, int _height) override;

    void render(RenderParameters &renderParameters,
                unsigned int gPosition,
                unsigned int gNormal,
                unsigned int gAlbedoSpec,
                unsigned int skybox,
                unsigned int transmittanceLUT);
};
