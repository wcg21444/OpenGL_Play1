
#include "Renderer.hpp"
#include "DebugObjectRenderer.hpp"
#include "Passes/DebugObjectPass.hpp"
#include "../Objects/FrustumWireframe.hpp"

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

void DebugObjectRenderer::DrawFrustum(const FrustumBase &frustum, Shader &shaders, glm::vec4 color, glm::mat4 modelMatrix)
{
    static FrustumWireframe frustumWireframe;
    frustumWireframe.setFrustum(frustum);

    shaders.setUniform("color", color);
    frustumWireframe.draw(modelMatrix, shaders);
}

void DebugObjectRenderer::DrawCube(Shader &shaders, glm::vec4 color, glm::mat4 modelMatrix)
{
    static Cube cube(glm::vec3(1.0f), "CubeTmp");

    shaders.setUniform("color", color);
    cube.draw(modelMatrix, shaders);
}

