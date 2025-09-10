#pragma once
#include "Pass.hpp"
class RenderTarget;
class Texture;
class GBufferPass : public Pass
{
private:
    std::shared_ptr<RenderTarget> renderTarget = nullptr;
    std::shared_ptr<Texture> gViewPosition = nullptr;
    std::shared_ptr<Texture> gPosition = nullptr;
    std::shared_ptr<Texture> gNormal = nullptr;
    std::shared_ptr<Texture> gAlbedoSpec = nullptr;
    unsigned int depthMap;
    void initializeGLResources();
    void cleanUpGLResources() override;

public:
    GBufferPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path);
    ~GBufferPass()
    {
        cleanUpGLResources();
    }

    void contextSetup() override;

    void resize(int _width, int _height) override;

    void render(RenderParameters &renderParameters);

    inline auto getTextures()
    {
        return std::make_tuple(gPosition->ID, gNormal->ID, gAlbedoSpec->ID, gViewPosition->ID);
    }
};
