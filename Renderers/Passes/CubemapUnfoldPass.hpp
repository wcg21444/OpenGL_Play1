#pragma once
#include "../../Utils/Random.hpp"
#include "Pass.hpp"
#include "../../Shading/Texture.hpp"
class CubemapUnfoldPass : public Pass
{

    unsigned int quadVAO;
    unsigned int quadVBO;
    const int CUBEMAP_FACE_SIZE;
    Texture unfoldedTex;

private:
    void initializeGLResources() override;

public:
    void contextSetup() override;
    void resize(int _width, int _height) override;

public:
    CubemapUnfoldPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path, int _CUBEMAP_FACE_SIZE = 256);
    ~CubemapUnfoldPass();

    void unfoldCubemap(unsigned int cubemap);
    void render(unsigned int cubemap);
    unsigned int getUnfoldedCubemap() const;
};
