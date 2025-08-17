#include <glad/glad.h>
#include "../../Shading/Shader.hpp"
#include "GBufferPass.hpp"

GBufferPass::GBufferPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path)
    : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
{
    initializeGLResources();
    contextSetup();
}
void GBufferPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);
    // glGenTextures(1, &gPosition);
    gPosition.Generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
    gNormal.Generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
    gAlbedoSpec.Generate(vp_width, vp_height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glGenTextures(1, &gViewPosition);
    glGenRenderbuffers(1, &depthMap);
}
void GBufferPass::contextSetup()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    // create depth texture
    // - position color buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition.ID, 0);

    // - normal color buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal.ID, 0);

    // - color + specular color buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec.ID, 0);

    // BUG 改为Texture SSAO 通道不正常,有大量噪点
    glBindTexture(GL_TEXTURE_2D, gViewPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, vp_width, vp_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gViewPosition, 0);

    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    // 分配多个写入对象.对应Shader input layout
    unsigned int attachments[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, attachments);

    glBindRenderbuffer(GL_RENDERBUFFER, depthMap);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, vp_width, vp_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthMap);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void GBufferPass::resize(int _width, int _height)
{
    vp_width = _width;
    vp_height = _height;

    gPosition.Resize(vp_width, vp_height);
    gNormal.Resize(vp_width, vp_height);
    gAlbedoSpec.Resize(vp_width, vp_height);
    // TODO non Texure Resize
    contextSetup();
}
void GBufferPass::render(RenderParameters &renderParameters)
{
    auto &[allLights, cam, scene, model, window] = renderParameters;

    glViewport(0, 0, vp_width, vp_height);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    shaders.use();

    /****************************************视口设置****************************************************/
    shaders.setInt("width", vp_width);
    shaders.setInt("height", vp_height);

    cam.setViewMatrix(shaders);
    cam.setPerspectiveMatrix(shaders, vp_width, vp_height);
    Renderer::DrawScene(scene, model, shaders);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
