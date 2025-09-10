#pragma once

#include <memory>

class Shader;
class Renderer;
class RenderParameters;
class DebugDepthRenderer;
class SimpleTextureRenderer;
class DepthPassRenderer;
class GBufferRenderer;
class CubemapUnfoldRenderer;
// fwd declaration

class RenderManager
{
private:
    std::shared_ptr<GBufferRenderer> gbufferRenderer = nullptr;
    std::shared_ptr<CubemapUnfoldRenderer> cubemapUnfoldRenderer = nullptr;
    std::shared_ptr<Renderer> currentRenderer = nullptr;
    void clearContext();

public:
    int rendererWidth = 1600;
    int rendererHeight = 900;

    enum Mode
    {
        gbuffer,
        cubemap_unfold
    };
    RenderManager();
    void reloadCurrentShaders();
    void switchMode(Mode _mode);
    void switchContext();
    void render(std::shared_ptr<RenderParameters> renderParameters);
    void resize(int _width, int _height);
};
