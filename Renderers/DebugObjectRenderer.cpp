
#include "Renderer.hpp"
#include "DebugObjectRenderer.hpp"
#include "Passes/DebugObjectPass.hpp"

#define STATICIMPL

void DebugObjectRenderer::Initialize()
{
    if (debugObjectPass)
        throw(std::runtime_error("DebugObjectRenderer already initialized."));
    debugObjectPass = std::make_shared<DebugObjectPass>(width, height, "Shaders/DebugRenderer/debugRenderer.vs", "Shaders/DebugRenderer/debugRenderer.fs");
}

void DebugObjectRenderer::Resize(int _width, int _height)
{
    width = _width;
    height = _height;
    CheckInitialized();
    debugObjectPass->resize(width, height);
}

void DebugObjectRenderer::ReloadCurrentShaders()
{
    CheckInitialized();
    debugObjectPass->reloadCurrentShaders();
}

void DebugObjectRenderer::AddDrawCall(const DebugObjectDrawCall &drawCall)
{
    CheckInitialized();
    drawQueue.push(drawCall);
}

void DebugObjectRenderer::Render(Camera &camera)
{
    CheckInitialized();
    debugObjectPass->render(drawQueue, camera);
}

unsigned int DebugObjectRenderer::GetRenderOutput()
{
    CheckInitialized();
    return debugObjectPass->getTexture();
}

void DebugObjectRenderer::CheckInitialized()
{
    if (!debugObjectPass)
    {
        throw std::runtime_error("DebugObjectRenderer not initialized. Call Initialize() first.");
    }
}
