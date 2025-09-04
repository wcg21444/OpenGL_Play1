#include "PointShadowPass.hpp"
#include "../../Shading/Cubemap.hpp"
PointShadowPass::PointShadowPass(std::string _vs_path, std::string _fs_path, std::string _gs_path)
    : Pass(0, 0, _vs_path, _fs_path, _gs_path)
{
    initializeGLResources();
    contextSetup();
}

inline void PointShadowPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);
}

inline void PointShadowPass::contextSetup()
{
    glEnable(GL_DEPTH_TEST);
}

inline void PointShadowPass::resize(int _width, int _height)
{
    // void 此处应该是缩放阴影贴图大小
}

void PointShadowPass::attachDepthMap(const unsigned int _depthCubemap)
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// 输入光源的Tex对象,绑定Tex对象到FBO,结果输出到Tex.
void PointShadowPass::renderToTexture(
    const PointLight &light,
    std::vector<std::unique_ptr<Object>> &scene,
    glm::mat4 &model,
    int width,
    int height)
{
    // attachDepthMap(light.depthCubemapID);
    attachDepthMap(light.depthCubemap->ID);

    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    {
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaders.use();
        if (!shaders.used)
            throw(std::exception("Shader failed to setup."));
        for (unsigned int i = 0; i < 6; ++i)
        {
            shaders.setMat4("shadowMatrices[" + std::to_string(i) + "]", light.cubemapParam->projectionMartix * light.cubemapParam->viewMatrices[i]);
        }
        shaders.setFloat("farPlane", light.getFarPlane());
        shaders.setUniform3fv("lightPos", light.getPosition());

        Renderer::DrawScene(scene, model, shaders);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

inline void PointShadowVSMPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);
}
PointShadowVSMPass::PointShadowVSMPass(std::string _vs_path, std::string _fs_path)
    : Pass(0, 0, _vs_path, _fs_path)
{
    initializeGLResources();
    contextSetup();
}

void PointShadowVSMPass::contextSetup()
{
}

void PointShadowVSMPass::resize(int _width, int _height)
{
}

void PointShadowVSMPass::renderToVSMTexture(const PointLight &light)
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glViewport(0, 0, light.texResolution, light.texResolution);
    shaders.use();
    if (!shaders.used)
        throw(std::exception("Shader failed to setup."));

    for (unsigned int i = 0; i < 6; ++i)
    {
        shaders.setMat4("projection", light.cubemapParam->projectionMartix);
        shaders.setMat4("view", light.cubemapParam->viewMatrices[i]);
        shaders.setTextureAuto(light.depthCubemap->ID, GL_TEXTURE_CUBE_MAP, 0, "depthCubemap");
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               TextureCube::FaceTargets[i], light.VSMCubemap->ID, 0);
        Renderer::DrawSphere();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
