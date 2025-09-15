#include <glad/glad.h>
#include "../../Shading/Shader.hpp"
#include "../../Shading/RenderTarget.hpp"
#include "../../Shading/Texture.hpp"
#include "../DebugObjectRenderer.hpp"
#include "GBufferPass.hpp"
#include "../../Math/Frustum.hpp"

#include "../../GUI.hpp"
GBufferPass::GBufferPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path)
    : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
{
    renderTarget = std::make_shared<RenderTarget>(_vp_width, _vp_height);
    gViewPosition = std::make_shared<Texture>();
    gPosition = std::make_shared<Texture>();
    gNormal = std::make_shared<Texture>();
    gAlbedoSpec = std::make_shared<Texture>();
    initializeGLResources();
    contextSetup();
}
void GBufferPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);

    gPosition->setFilterMin(GL_NEAREST);
    gPosition->setFilterMax(GL_NEAREST);
    gPosition->generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL, false);

    gNormal->setFilterMin(GL_NEAREST);
    gNormal->setFilterMax(GL_NEAREST);
    gNormal->generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL, false);

    gAlbedoSpec->setFilterMin(GL_NEAREST);
    gAlbedoSpec->setFilterMax(GL_NEAREST);
    gAlbedoSpec->generate(vp_width, vp_height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, NULL, false);

    gViewPosition->setFilterMin(GL_NEAREST);
    gViewPosition->setFilterMax(GL_NEAREST);
    gViewPosition->generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL, false);

    glGenRenderbuffers(1, &depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, vp_width, vp_height);
}

void GBufferPass::cleanUpGLResources()
{
    glDeleteFramebuffers(1, &FBO);
    glDeleteRenderbuffers(1, &depthRenderBuffer);
}

void GBufferPass::contextSetup()
{
    renderTarget->bind();
    renderTarget->attachColorTexture2D(gPosition->ID, GL_COLOR_ATTACHMENT0);
    renderTarget->attachColorTexture2D(gNormal->ID, GL_COLOR_ATTACHMENT1);
    renderTarget->attachColorTexture2D(gAlbedoSpec->ID, GL_COLOR_ATTACHMENT2);
    renderTarget->attachColorTexture2D(gViewPosition->ID, GL_COLOR_ATTACHMENT3);
    renderTarget->enableColorAttachments();
    renderTarget->attachDepthRenderBuffer(depthRenderBuffer, GL_DEPTH_ATTACHMENT);
    renderTarget->unbind();
}
void GBufferPass::resize(int _width, int _height)
{
    vp_width = _width;
    vp_height = _height;

    renderTarget->resize(vp_width, vp_height);

    gPosition->resize(vp_width, vp_height);
    gNormal->resize(vp_width, vp_height);
    gAlbedoSpec->resize(vp_width, vp_height);
    gViewPosition->resize(vp_width, vp_height);

    contextSetup();
}
void GBufferPass::render(RenderParameters &renderParameters)
{
    auto &[allLights, cam, scene, model, window] = renderParameters;

    renderTarget->bind();

    renderTarget->setViewport();
    renderTarget->clearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    shaders.use();

    /****************************************视口设置****************************************************/
    shaders.setInt("width", vp_width);
    shaders.setInt("height", vp_height);

    cam.resize(vp_width, vp_height);
    cam.setToShader(shaders);

    if (GUI::DebugToggleDrawWireframe())
    {
        DebugObjectRenderer::AddDrawCall([&](Shader &debugObjectShaders)
                                         {
                                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                                    Renderer::DrawScene(scene, model, debugObjectShaders);
                                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); });
    }
    else
    {
        Renderer::DrawScene(scene, model, shaders);
    }

    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    renderTarget->unbind();
}
