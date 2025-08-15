#pragma once
#include "Renderer.hpp"
#include "Passes/Pass.hpp"
#include "PointShadowPass.hpp"

class PointShadowPassDeprecated
{
    Shader depthShader = Shader("Shaders/PointShadow/shadow_depth.vs", "Shaders/PointShadow/shadow_depth.fs", "Shaders/PointShadow/shadow_depth.gs");

    int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    unsigned int depthMapFBO;

    float aspect;
    float near;

    glm::mat4 shadowProj;

public:
    unsigned int depthCubemap;
    float far;

private:
    unsigned int generateDepthCubemap()
    {
        unsigned int _depthCubemap;
        glGenTextures(1, &_depthCubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, _depthCubemap);
        for (unsigned int i = 0; i < 6; ++i)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                         SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        return _depthCubemap;
    }
    void attachDepthCubemap(unsigned int _depthCubemap, unsigned int &_depthMapFBO)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, _depthMapFBO);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _depthCubemap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

public:
    PointShadowPassDeprecated(int _SHADOW_WIDTH = 1024, int _SHADOW_HEIGHT = 1024, float _near = 1.0f, float _far = 250.f)
        : SHADOW_WIDTH(_SHADOW_WIDTH), SHADOW_HEIGHT(_SHADOW_HEIGHT), aspect((float)_SHADOW_WIDTH / (float)_SHADOW_HEIGHT),
          near(_near), far(_far)
    {
        // 设置OpenGL状态
        glEnable(GL_DEPTH_TEST);
        glGenFramebuffers(1, &depthMapFBO);
        depthCubemap = generateDepthCubemap();
        attachDepthCubemap(depthCubemap, depthMapFBO);
        shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);
    }
    void reloadCurrentShader()
    {
        depthShader = Shader("Shaders/PointShadow/shadow_depth.vs", "Shaders/PointShadow/shadow_depth.fs", "Shaders/PointShadow/shadow_depth.gs");
    }

    void render(PointLight &light, std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model)
    {
        static std::vector<glm::mat4> shadowTransforms;
        // 视图变换需要知道光源位置
        shadowTransforms.clear();
        shadowTransforms.push_back(shadowProj *
                                   glm::lookAt(light.position, light.position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(shadowProj *
                                   glm::lookAt(light.position, light.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(shadowProj *
                                   glm::lookAt(light.position, light.position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
        shadowTransforms.push_back(shadowProj *
                                   glm::lookAt(light.position, light.position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
        shadowTransforms.push_back(shadowProj *
                                   glm::lookAt(light.position, light.position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(light.position, light.position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        {
            glClearColor(0.f, 0.f, 0.f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            depthShader.use();
            if (!depthShader.used)
                throw(std::exception("Shader failed to setup."));
            for (unsigned int i = 0; i < 6; ++i)
            {
                depthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
            }
            depthShader.setFloat("farPlane", far);
            depthShader.setUniform3fv("lightPos", light.position);

            Renderer::DrawScene(scene, model, depthShader);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Render the depth cubemap texture
    // [in] depthCubemap,light, scene, model
    // [out] a new depthCubemap
    // [side effect] **change current viewport size**
    void renderToTexture(unsigned int &_depthCubemap, PointLight &light, std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model)
    {
        // initialize
        if (_depthCubemap == 0)
        {
            _depthCubemap = generateDepthCubemap();
        }
        attachDepthCubemap(_depthCubemap, depthMapFBO);

        render(light, scene, model);
    }
};

class ParrllelShadowPass
{
    Shader depthShader = Shader("Shaders/shadow_depth.vs", "Shaders/shadow_depth.fs");
    int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;

    unsigned int depthMapFBO;

public:
    unsigned int depthMap;

public:
    ParrllelShadowPass()
    {
        glEnable(GL_DEPTH_TEST);
        glGenFramebuffers(1, &depthMapFBO);
        // create depth texture
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0); // 将FBO输出到Texture
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void render(PointLight &light, std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model, glm::mat4 &lightSpaceMatrix)
    {
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        Renderer::DrawScene(scene, model, depthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    unsigned int generateDepthMap()
    {
        unsigned int _depthMap;
        glGenTextures(1, &_depthMap);
        glBindTexture(GL_TEXTURE_2D, _depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        return _depthMap;
    }
    void attachDepthMap(unsigned int _depthMap, unsigned int &_depthMapFBO)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void renderToTexture(unsigned int &_depthMap, DirectionLight &light, std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model)
    {
        // initialize
        if (_depthMap == 0)
        {
            _depthMap = generateDepthMap();
        }
        attachDepthMap(_depthMap, depthMapFBO);

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", light.lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        Renderer::DrawScene(scene, model, depthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

class ShadowRenderer : public Renderer
{

public:
    enum class ShadowMode
    {
        parallel_shadow,
        point_shadow
    };

private:
    Shader pls_shaders = Shader("Shaders/VertShader.vs", "Shaders/FragmShader.fs");
    Shader ps_shaders = Shader("Shaders/PointShadow/point_shadow.vs", "Shaders/PointShadow/point_shadow.fs");
    PointShadowPassDeprecated pointShadowPass;
    ParrllelShadowPass parrllelShadowPass;
    int width = 1600;
    int height = 900;

public:
    ShadowMode render_mode;
    void reloadCurrentShaders() override
    {
        if (render_mode == ShadowMode::parallel_shadow)
            pls_shaders = std::move(Shader("Shaders/VertShader.vs", "Shaders/FragmShader.fs"));
        else if (render_mode == ShadowMode::point_shadow)
            ps_shaders = std::move(Shader("Shaders/PointShadow/point_shadow.vs", "Shaders/PointShadow/point_shadow.fs"));
        pointShadowPass.reloadCurrentShader();
        contextSetup();
    }

    void reloadShaders(Shader &&_shaders, Shader &&_ps_shaders)
    {
        if (render_mode == ShadowMode::parallel_shadow)
            pls_shaders = std::move(_shaders);
        else if (render_mode == ShadowMode::point_shadow)
            ps_shaders = std::move(_ps_shaders);
        contextSetup();
    }

    void contextSetup() override
    {
        glEnable(GL_DEPTH_TEST); // 深度缓冲
        glViewport(0, 0, width, height);
    }

    void resize(int _width, int _height) override
    {

        width = _width;
        height = _height;

        contextSetup();
    }

    void render(RenderParameters &renderParameters) override
    {
        if (render_mode == ShadowMode::point_shadow)
        {
            renderPointShadow(renderParameters);
        }
        else if (render_mode == ShadowMode::parallel_shadow)
        {
            renderParallelShadow(renderParameters);
        }
        else
        {
            throw(std::exception("Unknown shadow mode."));
        }
    }

private:
    void renderPointShadow(RenderParameters &renderParameters)
    {
        auto &[allLights, cam, scene, model, window] = renderParameters;
        auto &[pointLights, dirLights] = allLights;
        // temporary light source variable
        PointLight &light = pointLights[0]; // Assuming the first light is the one we want to use for point shadow
        pointShadowPass.render(light, scene, model);

        glEnable(GL_DEPTH_TEST); // 深度缓冲
        glViewport(0, 0, width, height);

        ps_shaders.use();
        ps_shaders.setFloat("farPlane", pointShadowPass.far);
        ps_shaders.setTextureAuto(pointShadowPass.depthCubemap, GL_TEXTURE_CUBE_MAP, 0, "depthMap");

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 深度缓冲和Color缓冲一样需要交换链,清理他

        light.setToShader(ps_shaders);

        // camera/view transformation
        cam.setViewMatrix(ps_shaders);
        cam.setPerspectiveMatrix(ps_shaders, width, height);

        Renderer::DrawScene(scene, model, ps_shaders);
    }
    void renderParallelShadow(RenderParameters &renderParameters)
    {
        auto &[allLights, cam, scene, model, window] = renderParameters;
        auto &[pointLights, dirLights] = allLights;

        // temporary light source variable
        PointLight &light = pointLights[0]; // Assuming the first light is the one we want to use for shadow

        // 怎样设置正交投影farplane,正交比例,光源位置,使得阴影覆盖最优?
        static float ortho_scale = 10.f;
        ImGui::Begin("ParellLightMatrix");
        {
            ImGui::SliderFloat("OrthoScale", &ortho_scale, 1, 500.f);
        }
        ImGui::End();
        static float nearPlane = 0.1f, farPlane = 1000.f;
        glm::mat4 lightProjection = glm::ortho(-1.0f * ortho_scale, 1.0f * ortho_scale, -1.0f * ortho_scale, 1.0f * ortho_scale, nearPlane, farPlane); //
        glm::mat4 lightView = glm::lookAt(light.position,
                                          glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        parrllelShadowPass.render(light, scene, model, lightSpaceMatrix);

        glEnable(GL_DEPTH_TEST); // 深度缓冲
        glViewport(0, 0, width, height);

        pls_shaders.use();
        pls_shaders.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        pls_shaders.setTextureAuto(parrllelShadowPass.depthMap, GL_TEXTURE_2D, 0, "shdaowDepthMap");

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 深度缓冲和Color缓冲一样需要交换链,清理他

        light.setToShader(pls_shaders);

        // camera/view transformation
        cam.setViewMatrix(pls_shaders);
        cam.setPerspectiveMatrix(pls_shaders, width, height);

        Renderer::DrawScene(scene, model, pls_shaders);
    }
};
