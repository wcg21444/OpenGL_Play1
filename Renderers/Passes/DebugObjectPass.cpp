#include <glad/glad.h>
#include "../../Shading/Shader.hpp"
#include "../../Shading/RenderTarget.hpp"
#include "../../Shading/Texture.hpp"
#include "../../GUI.hpp"
#include "../RenderOutputManager.hpp"

#include "DebugObjectPass.hpp"
DebugObjectPass::DebugObjectPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path)
    : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
{
    renderTarget = std::make_shared<RenderTarget>(_vp_width, _vp_height);
    debugObjectPassTex = std::make_shared<Texture>();
    initializeGLResources();
    contextSetup();
}
void DebugObjectPass::initializeGLResources()
{
    debugObjectPassTex->SetFilterMin(GL_NEAREST);
    debugObjectPassTex->SetFilterMax(GL_NEAREST);
    debugObjectPassTex->Generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL, false);
}

void DebugObjectPass::cleanUpGLResources()
{
}

void DebugObjectPass::contextSetup()
{
    renderTarget->bind();
    renderTarget->attachColorTexture2D(debugObjectPassTex->ID, GL_COLOR_ATTACHMENT0);
    renderTarget->enableColorAttachments();
    renderTarget->unbind();
}
void DebugObjectPass::resize(int _width, int _height)
{
    vp_width = _width;
    vp_height = _height;

    renderTarget->resize(vp_width, vp_height);

    debugObjectPassTex->Resize(vp_width, vp_height);

    contextSetup();
}
void DebugObjectPass::render(std::queue<DebugObjectDrawCall> &drawQueue, Camera &cam)
{
    renderTarget->bind();
    renderTarget->setViewport();
    renderTarget->clearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, glm::vec4(0.0f));
    shaders.use();
    cam.setViewMatrix(shaders);
    cam.resize(vp_width, vp_height);
    cam.setPerspectiveMatrix(shaders);
    while (!drawQueue.empty())
    {
        auto &drawCall = drawQueue.front();
        drawCall(shaders, glm::identity<glm::mat4>());
        drawQueue.pop();
    }
    renderTarget->unbind();

    GUI::RenderTextureInspector({debugObjectPassTex->ID});
    auto size = RenderOutputManager::RenderToDockingWindow(debugObjectPassTex->ID, "Scene");
    resize(static_cast<int>(size.x), static_cast<int>(size.y));
}

unsigned int DebugObjectPass::getTexture()
{
    return debugObjectPassTex->ID;
}