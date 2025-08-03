#pragma once
#include "Pass.hpp"

class GBufferPass : public Pass
{
private:
    unsigned int gPosition, gNormal, gAlbedoSpec, gViewPosition;
    unsigned int depthMap;
    void initializeGLResources();

public:
    GBufferPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path);
    void contextSetup() override;
    void render(RenderParameters &renderParameters);
    auto getTextures()
    {
        return std::make_tuple(gPosition, gNormal, gAlbedoSpec, gViewPosition);
    }
};
