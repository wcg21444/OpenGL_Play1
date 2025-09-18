#pragma once
#include "Pass.hpp"
class RenderTarget;
class Texture2D;
class GBufferPass : public Pass
{
private:
    std::shared_ptr<RenderTarget> renderTarget = nullptr;
    std::shared_ptr<Texture2D> gViewPosition = nullptr;
    std::shared_ptr<Texture2D> gPosition = nullptr;
    std::shared_ptr<Texture2D> gNormal = nullptr;
    std::shared_ptr<Texture2D> gAlbedoSpec = nullptr;
    unsigned int depthRenderBuffer;
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
