#pragma once
#include "Renderer.hpp"
#include "Passes/CubemapUnfoldPass.hpp"
#include "Passes/Pass.hpp"

class CubemapUnfoldRenderer : public Renderer
{
    int width;
    int height;
    CubemapUnfoldPass unfoldPass;
    ScreenPass screenPass;

public:
    CubemapUnfoldRenderer(int _width = 1600, int _height = 900);
    void reloadCurrentShaders() override;
    void contextSetup() override;
    void resize(int _width, int _height) override;
    void render(RenderParameters &renderParameters) override;
};
