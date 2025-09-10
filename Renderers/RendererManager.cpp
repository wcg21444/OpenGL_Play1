#include "RendererManager.hpp"
#include "Renderer.hpp"
#include "DebugObjectRenderer.hpp"

#include "GBufferRenderer.hpp"
#include "CubemapUnfoldRenderer.hpp"

RenderManager::RenderManager()
{
    gbufferRenderer = std::make_shared<GBufferRenderer>();
    cubemapUnfoldRenderer = std::make_shared<CubemapUnfoldRenderer>();
    DebugObjectRenderer::Initialize(); // camera will be set later
    switchMode(gbuffer); // default
}

void RenderManager::clearContext()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    GLint maxTextureUnits;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
    for (int i = 0; i < maxTextureUnits; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void RenderManager::reloadCurrentShaders()
{
    if (currentRenderer)
    {
        currentRenderer->reloadCurrentShaders();
        DebugObjectRenderer::ReloadCurrentShaders();
    }
}

void RenderManager::switchMode(Mode _mode)
{
    switch (_mode)
    {
    case gbuffer:
        currentRenderer = gbufferRenderer;
        break;
    case cubemap_unfold:
        currentRenderer = cubemapUnfoldRenderer;
        break;
    default:
        throw(std::exception("No Selected Render Mode."));
        break;
    }
    switchContext();
}

void RenderManager::switchContext()
{
    clearContext();
    if (currentRenderer)
    {
        currentRenderer->resize(rendererWidth, rendererHeight);

        currentRenderer->contextSetup();
    }
    else
    {
        throw(std::exception("No Renderer Selected."));
    }
}

void RenderManager::render(std::shared_ptr<RenderParameters> renderParameters)
{
    if (rendererWidth == 0 || rendererHeight == 0)
    {
        return; // minimized
    }
    if (currentRenderer)
    {
        currentRenderer->render(*renderParameters);
        DebugObjectRenderer::Render(renderParameters->cam);
    }
    else
    {
        throw(std::exception("No Renderer Selected."));
    }
}

void RenderManager::resize(int _width, int _height)
{
    if (currentRenderer)
    {
        currentRenderer->resize(_width, _height);
        DebugObjectRenderer::Resize(_width, _height);
        rendererWidth = _width;
        rendererHeight = _height;
    }
    else
    {
        throw(std::exception("No Renderer Selected."));
    }
}
