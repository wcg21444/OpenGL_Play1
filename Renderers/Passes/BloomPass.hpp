#pragma once
#include "Pass.hpp"
#include "GaussianBlurPass.hpp"
class BloomShaderUI;

class BloomPass : public Pass
{
private:
    Texture bloomPassTex;
    GaussianBlurPass blurPass;

    std::unique_ptr<BloomShaderUI> shaderUI;

    void initializeGLResources() override;
    void contextSetup() override;

public:
    BloomPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path);
    ~BloomPass() override;

    void resize(int _width, int _height) override;
    void reloadCurrentShaders() override;
    unsigned int getTextures();
    void render(unsigned int screenTex);
};