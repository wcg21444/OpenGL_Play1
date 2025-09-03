#pragma once
#include "Pass.hpp"
class GBufferPass : public Pass
{
private:
    unsigned int gViewPositionID;
    Texture gViewPosition;
    Texture gPosition;
    Texture gNormal;
    Texture gAlbedoSpec;
    unsigned int depthMap;
    void initializeGLResources();

public:
    GBufferPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path);

    void contextSetup() override;

    void resize(int _width, int _height) override;

    void render(RenderParameters &renderParameters);

    inline auto getTextures()
    {
        return std::make_tuple(gPosition.ID, gNormal.ID, gAlbedoSpec.ID, gViewPosition.ID);
    }
};
