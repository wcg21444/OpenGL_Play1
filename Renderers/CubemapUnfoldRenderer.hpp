#pragma once
#include "Renderer.hpp"
#include "ShadowRenderer.hpp"
#include "../utils/Random.hpp"
#include "Pass.hpp"
#include "../ShaderGUI.hpp"

class CubemapUnfoldRenderer : public Renderer
{
    Shader unfoldShaders;
    Shader quadShader;
    int width;
    int height;
    unsigned int quadVAO;
    unsigned int quadVBO;
    unsigned int unfoldFBO;
    unsigned int unfoldedCubemap;
    const int CUBEMAP_FACE_SIZE;
    PointShadowPass pointShadowPass;

public:
    CubemapUnfoldRenderer();
    void reloadCurrentShaders() override;
    void contextSetup() override;
    void unfoldCubemap(unsigned int cubemap);
    void render(RenderParameters &renderParameters) override;
    unsigned int getUnfoldedCubemap() const { return unfoldedCubemap; }
};
