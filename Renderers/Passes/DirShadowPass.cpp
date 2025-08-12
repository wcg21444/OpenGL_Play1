#include "DirShadowPass.hpp"
DirShadowPass::DirShadowPass(std::string _vs_path, std::string _fs_path)
    : Pass(0, 0, _vs_path, _fs_path)
{
    initializeGLResources();
    contextSetup();
}
inline void DirShadowPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);
}
inline void DirShadowPass::contextSetup()
{
}
void DirShadowPass::attachDepthMap(const unsigned int _depthMap)
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
// 输入存在的Tex对象,绑定Tex对象到FBO,结果输出到Tex.
void DirShadowPass::renderToTexture(
    const DirectionLight &light,
    std::vector<std::unique_ptr<Object>> &scene,
    glm::mat4 &model,
    int width,
    int height)
{
    attachDepthMap(light.depthMap);

    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaders.use();
    shaders.setMat4("lightSpaceMatrix", light.lightSpaceMatrix);

    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    Renderer::DrawScene(scene, model, shaders);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
