#pragma once
#include "RendererManager.hpp"
#include "Renderer.hpp"
#include "ShadowRenderer.hpp"

class CubemapUnfoldRenderer : public Renderer
{
    Shader unfoldShaders = Shader("Shaders/GBuffer/cubemap_unfold_debug.vs", "Shaders/GBuffer/cubemap_unfold_debug.fs");
    Shader quadShader = Shader("Shaders/GBuffer/texture.vs", "Shaders/GBuffer/texture.fs");
    int width = 1600;
    int height = 900;

    unsigned int quadVAO = 0;
    unsigned int quadVBO;
    unsigned int unfoldFBO = 0;
    unsigned int unfoldedCubemap = 0;

    // 十字形布局：
    //    +Y
    // -X +Z +X -Z
    //    -Y
    // 宽度 = 4 * FACE_SIZE, 高度 = 3 * FACE_SIZE
    const int CUBEMAP_FACE_SIZE = 1024; // 假设 Cubemap 每个面的边长

    PointShadowPass pointShadowPass;

public:
    void reloadCurrentShaders()
    {
        unfoldShaders = Shader("Shaders/GBuffer/cubemap_unfold_debug.vs", "Shaders/GBuffer/cubemap_unfold_debug.fs");
        quadShader = Shader("Shaders/GBuffer/texture.vs", "Shaders/GBuffer/texture.fs");
        pointShadowPass.reloadCurrentShader();
        contextSetup();
    }
    void contextSetup()
    {
        static bool initialized = false;

        if (!initialized)
        {
            glGenFramebuffers(1, &unfoldFBO);
            glGenTextures(1, &unfoldedCubemap);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, unfoldFBO);
        glBindTexture(GL_TEXTURE_2D, unfoldedCubemap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, CUBEMAP_FACE_SIZE * 4, CUBEMAP_FACE_SIZE * 3, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, unfoldedCubemap, 0);

        // set up VAO of Demo Quad Plane
        if (quadVAO == 0)
        {
            static float quadVertices[] = {
                // positions       // texCoords
                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 0.0f, 1.0f, 1.0f};
            // setup plane VAO
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        }
    }
    // 设置cubemap; 展开cubemap存储至unfoldedCubemap
    //[out] this.unfoldedCubemap
    void unfoldCubemap(unsigned int cubemap)
    {

        // 将cubemap 绑定到sampler

        glBindFramebuffer(GL_FRAMEBUFFER, unfoldFBO);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        unfoldShaders.use();
        unfoldShaders.setTextureAuto(cubemap, GL_TEXTURE_CUBE_MAP, 5, "u_cubemap");

        glBindVertexArray(quadVAO); // 绑定 Quad VAO

        std::vector<std::tuple<int, int, int>> faceLayout = {
            // +X (Right)
            {2 * CUBEMAP_FACE_SIZE, 1 * CUBEMAP_FACE_SIZE, 0},
            // -X (Left)
            {0 * CUBEMAP_FACE_SIZE, 1 * CUBEMAP_FACE_SIZE, 1},
            // +Y (Top)
            {1 * CUBEMAP_FACE_SIZE, 2 * CUBEMAP_FACE_SIZE, 2},
            // -Y (Bottom)
            {1 * CUBEMAP_FACE_SIZE, 0 * CUBEMAP_FACE_SIZE, 3},
            // +Z (Front)
            {1 * CUBEMAP_FACE_SIZE, 1 * CUBEMAP_FACE_SIZE, 4},
            // -Z (Back)
            {3 * CUBEMAP_FACE_SIZE, 1 * CUBEMAP_FACE_SIZE, 5}};

        for (const auto &faceInfo : faceLayout)
        {
            int xOffset = std::get<0>(faceInfo);
            int yOffset = std::get<1>(faceInfo);
            int faceIndex = std::get<2>(faceInfo);

            glViewport(xOffset, yOffset, CUBEMAP_FACE_SIZE, CUBEMAP_FACE_SIZE);
            unfoldShaders.setInt("u_faceIndex", faceIndex);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0); // 解绑 FBO，回到默认帧缓冲
    }
    void render(RenderParameters &renderParameters)
    {
        auto &[lights, cam, scene, model, window] = renderParameters;

        static bool initialized = false;
        static std::vector<std::string> faces{
            "Resource/skybox/right.jpg",
            "Resource/skybox/left.jpg",
            "Resource/skybox/top.jpg",
            "Resource/skybox/bottom.jpg",
            "Resource/skybox/front.jpg",
            "Resource/skybox/back.jpg"};
        static auto skyboxCubemap = loadCubemap(faces);
        if (!initialized)
        {
            initialized = true;
        }

        pointShadowPass.render(lights[0], scene, model);
        // 展开Cubemap
        unfoldCubemap(skyboxCubemap);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);

        quadShader.use();
        quadShader.setTextureAuto(unfoldedCubemap, GL_TEXTURE_2D, 10, "tex_sampler");
        // 绘制Quad
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
        glBindVertexArray(0);
    }
};

class GBufferRenderer : public Renderer
{
private:
    Shader gShaders = Shader("Shaders/GBuffer/gbuffer.vs", "Shaders/GBuffer/gbuffer.fs");
    Shader quadShader = Shader("Shaders/GBuffer/texture.vs", "Shaders/GBuffer/texture.fs");
    Shader lightShaders = Shader("Shaders/GBuffer/light.vs", "Shaders/GBuffer/light.fs");
    std::vector<std::string> faces{
        "Resource/skybox/right.jpg",
        "Resource/skybox/left.jpg",
        "Resource/skybox/top.jpg",
        "Resource/skybox/bottom.jpg",
        "Resource/skybox/front.jpg",
        "Resource/skybox/back.jpg"};
    int width = 1600;
    int height = 900;

    unsigned int quadVAO = 0;
    unsigned int quadVBO;

    unsigned int gBuffer;
    unsigned int gPosition, gNormal, gAlbedoSpec;
    unsigned int depthMap;
    unsigned int cubemapTexture;

    const int MAX_LIGHTS = 10;

    PointShadowPass pointShadowPass;

public:
    void reloadCurrentShaders()
    {
        gShaders = Shader("Shaders/GBuffer/gbuffer.vs", "Shaders/GBuffer/gbuffer.fs");
        lightShaders = Shader("Shaders/GBuffer/light.vs", "Shaders/GBuffer/light.fs");
        quadShader = Shader("Shaders/GBuffer/texture.vs", "Shaders/GBuffer/texture.fs");
        pointShadowPass.reloadCurrentShader();
        contextSetup();
    }
    // contextSetup 资源生成应当只生成一次
    void contextSetup()
    {
        static bool initialized = false;
        glEnable(GL_DEPTH_TEST); // 深度缓冲

        if (!initialized)
        {
            initialized = true;
            glViewport(0, 0, width, height);
            glGenFramebuffers(1, &gBuffer);
            glGenTextures(1, &gPosition);
            glGenTextures(1, &gNormal);
            glGenTextures(1, &gAlbedoSpec);
            glGenRenderbuffers(1, &depthMap);
            cubemapTexture = loadCubemap(faces);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        // create depth texture
        // - position color buffer
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

        // - normal color buffer
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

        // - color + specular color buffer
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

        // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
        unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, attachments);

        glBindRenderbuffer(GL_RENDERBUFFER, depthMap);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthMap);

        // set up VAO of Demo Quad Plane
        if (quadVAO == 0)
        {
            static float quadVertices[] =
                {
                    -1.0f,
                    1.0f,
                    0.0f,
                    0.0f,
                    1.0f,
                    -1.0f,
                    -1.0f,
                    0.0f,
                    0.0f,
                    0.0f,
                    1.0f,
                    1.0f,
                    0.0f,
                    1.0f,
                    1.0f,
                    1.0f,
                    -1.0f,
                    0.0f,
                    1.0f,
                    0.0f,
                };
            // setup plane VAO
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        }
    }

    void render(RenderParameters &renderParameters)
    {

        // #define DEBUG_GBUFFER
#define RENDER_GBUFFER

#ifdef DEBUG_GBUFFER
        renderDebug(renderParameters);
#endif
#ifdef RENDER_GBUFFER
        renderLight(renderParameters);
#endif
    }

private:
    void
    renderGBuffer(RenderParameters &renderParameters)
    {
        auto &[lights, cam, scene, model, window] = renderParameters;
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            gShaders.use();

            cam.setViewMatrix(gShaders);
            cam.setPerspectiveMatrix(gShaders, width, height);
            renderScene(scene, model, gShaders);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void renderDebug(RenderParameters &renderParameters)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderGBuffer(renderParameters);
        // 绑定 GBuffer Texture 到Quad

        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_2D, gNormal);
        // glActiveTexture(GL_TEXTURE2);
        // glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        quadShader.use();
        quadShader.setTextureAuto(gPosition, GL_TEXTURE_2D, 0, "tex_sampler");
        // quadShader.setInt("tex_sampler", 1); // gNormal
        // quadShader.setInt("tex_sampler", 2); // gAlbedoSpec

        // 绘制Quad
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }

    void renderLight(RenderParameters &renderParameters)
    {
        auto &[lights, cam, scene, model, window] = renderParameters;

        // 渲染阴影贴图
        for (auto &light : lights)
        {
            pointShadowPass.renderToTexture(light.depthCubemap, light, scene, model);
        }
        glViewport(0, 0, width, height);
        renderGBuffer(renderParameters);

        lightShaders.use();

        // 绑定 GBuffer Texture 到Quad
        lightShaders.setTextureAuto(gPosition, GL_TEXTURE_2D, 0, "gPosition");
        lightShaders.setTextureAuto(gNormal, GL_TEXTURE_2D, 1, "gNormal");
        lightShaders.setTextureAuto(gAlbedoSpec, GL_TEXTURE_2D, 2, "gAlbedoSpec");

        // lightShaders.setTextureAuto(cubemapTexture, GL_TEXTURE_CUBE_MAP, 31, "skyBox");

        lightShaders.setInt("numLights", static_cast<int>(lights.size()));
        for (size_t i = 0; i < MAX_LIGHTS; ++i)
        {
            lightShaders.setTextureAuto(0, GL_TEXTURE_CUBE_MAP, 0, "shadowCubeMaps[" + std::to_string(i) + "]"); // 给sampler数组赋空
        }
        for (size_t i = 0; i < lights.size(); ++i)
        {
            lightShaders.setUniform3fv("light_pos[" + std::to_string(i) + "]", lights[i].position);
            lightShaders.setUniform3fv("light_intensity[" + std::to_string(i) + "]", lights[i].intensity);
            if (lights[i].depthCubemap != 0)
            {
                lightShaders.setTextureAuto(lights[i].depthCubemap, GL_TEXTURE_CUBE_MAP, i + 3, "shadowCubeMaps[" + std::to_string(i) + "]");
            }
        }

        // sampler location是否会被覆盖?
        // 光照计算在 纹理计算之后,不用担心光照被纹理覆盖
        lightShaders.setUniform3fv("eye_pos", cam.getPosition());
        lightShaders.setUniform3fv("eye_front", cam.getFront());
        lightShaders.setUniform3fv("eye_up", cam.getUp());

        lightShaders.setFloat("far_plane", cam.far);
        lightShaders.setFloat("near_plane", cam.near);
        lightShaders.setFloat("shadow_far", pointShadowPass.far);
        lightShaders.setFloat("fov", cam.fov);
        // 绘制Quad
        glViewport(0, 0, width, height);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }
};
