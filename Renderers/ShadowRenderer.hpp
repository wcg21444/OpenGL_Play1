#pragma once
#include "Renderer.hpp"
#include "Pass.hpp"

class PointShadowPass
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
    PointShadowPass(int _SHADOW_WIDTH = 1024, int _SHADOW_HEIGHT = 1024, float _near = 1.0f, float _far = 250.f)
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

    void render(LightSource &light, std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model)
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
            depthShader.setFloat("far_plane", far);
            depthShader.setUniform3fv("lightPos", light.position);

            DrawScene(scene, model, depthShader);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Render the depth cubemap texture
    // [in] depthCubemap,light, scene, model
    // [out] a new depthCubemap
    // [side effect] **change current viewport size**
    void renderToTexture(unsigned int &_depthCubemap, LightSource &light, std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model)
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
    int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

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
    void render(LightSource &light, std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model, glm::mat4 &lightSpaceMatrix)
    {
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        DrawScene(scene, model, depthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

class ShadowTexPass : public Pass
{
private:
    const int MAX_LIGHTS = 10;
    unsigned int shadowTex;

private:
    void initializeGLResources()
    {
        glGenFramebuffers(1, &FBO);
        glGenTextures(1, &shadowTex);
    }

public:
    ShadowTexPass(int _vp_width, int _vp_height, std::string _vs_path,
                  std::string _fs_path)
        : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
    {
        initializeGLResources();
        contextSetup();
    }
    void contextSetup()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        {
            glBindTexture(GL_TEXTURE_2D, shadowTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, vp_width, vp_height, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowTex, 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    unsigned int getTextures()
    {
        return shadowTex;
    }
    void render(RenderParameters &renderParameters,
                unsigned int gPosition,
                unsigned int gNormal,
                unsigned int gAlbedoSpec,
                unsigned int ssaoTex, float shadow_far)
    {

        auto &[lights, cam, scene, model, window] = renderParameters;

        glViewport(0, 0, vp_width, vp_height);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        shaders.use();

        // 绑定 GBuffer Texture 到Quad
        shaders.setTextureAuto(gPosition, GL_TEXTURE_2D, 0, "gPosition");
        shaders.setTextureAuto(gNormal, GL_TEXTURE_2D, 0, "gNormal");
        shaders.setTextureAuto(gAlbedoSpec, GL_TEXTURE_2D, 0, "gAlbedoSpec");
        shaders.setTextureAuto(ssaoTex, GL_TEXTURE_2D, 0, "ssaoTex");

        // shaders.setTextureAuto(cubemapTexture, GL_TEXTURE_CUBE_MAP, 31, "skyBox");

        shaders.setInt("numLights", static_cast<int>(lights.size()));
        for (size_t i = 0; i < MAX_LIGHTS; ++i)
        {
            shaders.setTextureAuto(0, GL_TEXTURE_CUBE_MAP, 0, "shadowCubeMaps[" + std::to_string(i) + "]"); // 给sampler数组赋空
        }
        for (size_t i = 0; i < lights.size(); ++i)
        {
            shaders.setUniform3fv("light_pos[" + std::to_string(i) + "]", lights[i].position);
            shaders.setUniform3fv("light_intensity[" + std::to_string(i) + "]", lights[i].intensity);
            if (lights[i].depthCubemap != 0)
            {
                shaders.setTextureAuto(lights[i].depthCubemap, GL_TEXTURE_CUBE_MAP, i + 3, "shadowCubeMaps[" + std::to_string(i) + "]");
            }
        }

        shaders.setUniform3fv("eye_pos", cam.getPosition());
        shaders.setUniform3fv("eye_front", cam.getFront());
        shaders.setUniform3fv("eye_up", cam.getUp());

        shaders.setFloat("far_plane", cam.far);
        shaders.setFloat("near_plane", cam.near);
        shaders.setFloat("shadow_far", shadow_far);
        shaders.setFloat("fov", cam.fov);

        DrawQuad();
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
    PointShadowPass pointShadowPass;
    ParrllelShadowPass parrllelShadowPass;
    int width = 1600;
    int height = 900;

public:
    ShadowMode render_mode;
    void reloadCurrentShaders()
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
    void contextSetup()
    {
        glEnable(GL_DEPTH_TEST); // 深度缓冲
        glViewport(0, 0, width, height);
    }

    void render(RenderParameters &renderParameters)
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
        auto &[lights, cam, scene, model, window] = renderParameters;
        // temporary light source variable
        LightSource &light = lights[0]; // Assuming the first light is the one we want to use for point shadow
        pointShadowPass.render(light, scene, model);

        glEnable(GL_DEPTH_TEST); // 深度缓冲
        glViewport(0, 0, width, height);

        ps_shaders.use();
        ps_shaders.setFloat("far_plane", pointShadowPass.far);
        ps_shaders.setTextureAuto(pointShadowPass.depthCubemap, GL_TEXTURE_CUBE_MAP, 0, "depthMap");

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 深度缓冲和Color缓冲一样需要交换链,清理他

        light.setToShader(ps_shaders);

        // camera/view transformation
        cam.setViewMatrix(ps_shaders);
        cam.setPerspectiveMatrix(ps_shaders, width, height);

        DrawScene(scene, model, ps_shaders);
    }
    void renderParallelShadow(RenderParameters &renderParameters)
    {
        auto &[lights, cam, scene, model, window] = renderParameters;

        // temporary light source variable
        LightSource &light = lights[0]; // Assuming the first light is the one we want to use for shadow

        static float near_plane = 1.f, far_plane = 700.f;
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(light.position,
                                          glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        ShowGLMMatrixAsTable(lightSpaceMatrix);

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

        DrawScene(scene, model, pls_shaders);
    }
};
