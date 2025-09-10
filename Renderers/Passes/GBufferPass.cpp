#include <glad/glad.h>
#include "../../Shading/Shader.hpp"
#include "../../Shading/RenderTarget.hpp"
#include "../../Shading/Texture.hpp"

#include "GBufferPass.hpp"
#include "../../Shading/Frustum.hpp"
GBufferPass::GBufferPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path)
    : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
{
    renderTarget = std::make_shared<RenderTarget>(_vp_width,_vp_height);
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

    gPosition->SetFilterMin(GL_NEAREST);
    gPosition->SetFilterMax(GL_NEAREST);
    gPosition->Generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL, false);

    gNormal->SetFilterMin(GL_NEAREST);
    gNormal->SetFilterMax(GL_NEAREST);
    gNormal->Generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL, false);

    gAlbedoSpec->SetFilterMin(GL_NEAREST);
    gAlbedoSpec->SetFilterMax(GL_NEAREST);
    gAlbedoSpec->Generate(vp_width, vp_height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, NULL, false);

    gViewPosition->SetFilterMin(GL_NEAREST);
    gViewPosition->SetFilterMax(GL_NEAREST);
    gViewPosition->Generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL, false);

    glGenRenderbuffers(1, &depthMap);
}

void GBufferPass::cleanUpGLResources()
{
    glDeleteFramebuffers(1, &FBO);
}

void GBufferPass::contextSetup()
{
    renderTarget->bind();
    renderTarget->attachColorTexture2D(gPosition->ID,GL_COLOR_ATTACHMENT0);
    renderTarget->attachColorTexture2D(gNormal->ID,GL_COLOR_ATTACHMENT1);
    renderTarget->attachColorTexture2D(gAlbedoSpec->ID,GL_COLOR_ATTACHMENT2);
    renderTarget->attachColorTexture2D(gViewPosition->ID,GL_COLOR_ATTACHMENT3);
    renderTarget->enableColorAttachments();
    glBindRenderbuffer(GL_RENDERBUFFER, depthMap);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, vp_width, vp_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthMap);
    renderTarget->unbind();
}
void GBufferPass::resize(int _width, int _height)
{
    vp_width = _width;
    vp_height = _height;

    renderTarget->resize(vp_width, vp_height);

    gPosition->Resize(vp_width, vp_height);
    gNormal->Resize(vp_width, vp_height);
    gAlbedoSpec->Resize(vp_width, vp_height);
    gViewPosition->Resize(vp_width, vp_height);

    contextSetup();
}
void GBufferPass::render(RenderParameters &renderParameters)
{
    auto &[allLights, cam, scene, model, window] = renderParameters;


    renderTarget->bind();

    renderTarget->setViewport();
    renderTarget->clearBuffer(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    // glViewport(0, 0, vp_width, vp_height);
    // glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    // glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    shaders.use();

    /****************************************视口设置****************************************************/
    shaders.setInt("width", vp_width);
    shaders.setInt("height", vp_height);

    cam.setViewMatrix(shaders);
    cam.resize(vp_width, vp_height);
    cam.setPerspectiveMatrix(shaders);
    Renderer::DrawScene(scene, model, shaders);

    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    renderTarget->unbind();
}
