#include "PointShadowPass.hpp"
PointShadowPass::PointShadowPass(std::string _vs_path, std::string _fs_path, std::string _gs_path)
    : Pass(0, 0, _vs_path, _fs_path, _gs_path)
{
    initializeGLResources();
    contextSetup();

    aspect = 1.f;
    nearPlane = 0.1f;
    farPlane = 250.f;
    shadowProj = glm::perspective(glm::radians(90.0f), aspect, nearPlane, farPlane);
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
    attachDepthMap(light.depthCubemap);

    static std::vector<glm::mat4> shadowTransforms;
    // 视图变换需要知道光源位置
    auto position = light.getPosition();
    shadowTransforms.clear();
    shadowTransforms.push_back(shadowProj *
                               glm::lookAt(position, position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.push_back(shadowProj *
                               glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.push_back(shadowProj *
                               glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
    shadowTransforms.push_back(shadowProj *
                               glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
    shadowTransforms.push_back(shadowProj *
                               glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

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
            shaders.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        }
        shaders.setFloat("farPlane", farPlane);
        shaders.setUniform3fv("lightPos", position);

        Renderer::DrawScene(scene, model, shaders);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
