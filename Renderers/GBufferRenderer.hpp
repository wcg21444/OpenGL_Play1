#pragma once
#include "RendererManager.hpp"
#include "Renderer.hpp"
#include "ShadowRenderer.hpp"
#include "Random.hpp"
#include "Pass.hpp"
#include <tuple>

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
            GenerateQuad(quadVAO, quadVBO);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, unfoldFBO);
        {
            glBindTexture(GL_TEXTURE_2D, unfoldedCubemap);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, CUBEMAP_FACE_SIZE * 4, CUBEMAP_FACE_SIZE * 3, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, unfoldedCubemap, 0);
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
        static auto skyboxCubemap = LoadCubemap(faces);
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
        DrawQuad();
    }
};

class GBufferPass : public Pass
{
private:
    unsigned int gPosition, gNormal, gAlbedoSpec; // Output ReadOnly
    unsigned int depthMap;

private:
    void initializeGLResources()
    {
        glGenFramebuffers(1, &FBO);
        glGenTextures(1, &gPosition);
        glGenTextures(1, &gNormal);
        glGenTextures(1, &gAlbedoSpec);
        glGenRenderbuffers(1, &depthMap);
    }

public:
    GBufferPass(int _vp_width, int _vp_height, std::string _vs_path,
                std::string _fs_path)
        : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
    {
        initializeGLResources();
        contextSetup();
    }

    void contextSetup()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        // create depth texture
        // - position color buffer
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, vp_width, vp_height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

        // - normal color buffer
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, vp_width, vp_height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

        // - color + specular color buffer
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, vp_width, vp_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

        // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
        unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, attachments);

        glBindRenderbuffer(GL_RENDERBUFFER, depthMap);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, vp_width, vp_height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthMap);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void render(RenderParameters &renderParameters)
    {
        auto &[lights, cam, scene, model, window] = renderParameters;

        glViewport(0, 0, vp_width, vp_height); // 状态设置内聚
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            shaders.use();

            cam.setViewMatrix(shaders);
            cam.setPerspectiveMatrix(shaders, vp_width, vp_height);
            DrawScene(scene, model, shaders);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    auto getTextures()
    {
        return std::make_tuple(gPosition, gNormal, gAlbedoSpec);
    }
};

class LightPass : public Pass
{
private:
    const int MAX_LIGHTS = 10;
    unsigned int lightPassTex;

private:
    void initializeGLResources()
    {
        glGenFramebuffers(1, &FBO);
        glGenTextures(1, &lightPassTex);
    }

public:
    LightPass(int _vp_width, int _vp_height, std::string _vs_path,
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
            glBindTexture(GL_TEXTURE_2D, lightPassTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, vp_width, vp_height, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightPassTex, 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    unsigned int getTextures()
    {
        return lightPassTex;
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

        // sampler location是否会被覆盖?
        // 光照计算在 纹理计算之后,不用担心光照被纹理覆盖
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

class SSAOPass : public Pass
{
private:
    unsigned int SSAOPassTex;
    unsigned int noiseTexture; // SSAO Noise

private:
    void initializeGLResources()
    {
        glGenFramebuffers(1, &FBO);
        glGenTextures(1, &SSAOPassTex);
        glGenTextures(1, &noiseTexture);
    }

public:
    SSAOPass(int _vp_width, int _vp_height, std::string _vs_path,
             std::string _fs_path)
        : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
    {
        initializeGLResources();
        contextSetup();
    }
    void generateNoiseTexture()
    {
        auto ssaoNoise = Random::GenerateSSAONoise();
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 8, 8, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    void contextSetup()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        {
            glBindTexture(GL_TEXTURE_2D, SSAOPassTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, vp_width, vp_height, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SSAOPassTex, 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    unsigned int getTextures()
    {
        return SSAOPassTex;
    }
    void render(RenderParameters &renderParameters,
                unsigned int gPosition,
                unsigned int gNormal,
                unsigned int gAlbedoSpec)
    {

        auto &[lights, cam, scene, model, window] = renderParameters;
        auto ssaoKernel = Random::GenerateSSAOKernel();
        generateNoiseTexture();
        glViewport(0, 0, vp_width, vp_height);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);

        shaders.use();

        for (unsigned int i = 0; i < 64; ++i)
        {
            shaders.setUniform3fv(std::format("samples[{}]", i), ssaoKernel[i]);
        }
        shaders.setTextureAuto(gPosition, GL_TEXTURE_2D, 0, "gPosition");
        shaders.setTextureAuto(gNormal, GL_TEXTURE_2D, 0, "gNormal");
        shaders.setTextureAuto(gAlbedoSpec, GL_TEXTURE_2D, 0, "gAlbedoSpec");
        shaders.setTextureAuto(noiseTexture, GL_TEXTURE_2D, 0, "texNoise");
        shaders.setUniform3fv("eye_pos", cam.getPosition());
        shaders.setFloat("far_plane", cam.far);
        cam.setPerspectiveMatrix(shaders, vp_width, vp_height);
        cam.setViewMatrix(shaders);

        DrawQuad();
    }
};

class GBufferRenderer : public Renderer
{
private:
    Shader quadShader = Shader("Shaders/GBuffer/texture.vs", "Shaders/GBuffer/texture.fs");
    std::vector<std::string> faces{
        "Resource/skybox/right.jpg",
        "Resource/skybox/left.jpg",
        "Resource/skybox/top.jpg",
        "Resource/skybox/bottom.jpg",
        "Resource/skybox/front.jpg",
        "Resource/skybox/back.jpg"};
    int width = 1600;
    int height = 900;

    unsigned int FBO;
    unsigned int gPosition, gNormal, gAlbedoSpec;
    unsigned int depthMap;
    unsigned int cubemapTexture;
    unsigned int noiseTexture; // SSAO Noise

    const int MAX_LIGHTS = 10;

    PointShadowPass pointShadowPass;
    GBufferPass gBufferPass;
    LightPass lightPass;
    ScreenPass screenPass;
    SSAOPass ssaoPass;

public:
    GBufferRenderer()
        : gBufferPass(GBufferPass(width, height, "Shaders/GBuffer/gbuffer.vs", "Shaders/GBuffer/gbuffer.fs")),
          lightPass(LightPass(width, height, "Shaders/GBuffer/light.vs", "Shaders/GBuffer/light.fs")),
          screenPass(ScreenPass(width, height, "Shaders/GBuffer/texture.vs", "Shaders/GBuffer/texture.fs")),
          ssaoPass(SSAOPass(width, height, "Shaders/SSAOPass/ssao.vs", "Shaders/SSAOPass/ssao.fs"))
    {
    }
    void reloadCurrentShaders()
    {
        quadShader = Shader("Shaders/GBuffer/texture.vs", "Shaders/GBuffer/texture.fs");
        pointShadowPass.reloadCurrentShader();
        gBufferPass.reloadCurrentShaders();
        lightPass.reloadCurrentShaders();
        screenPass.reloadCurrentShaders();
        ssaoPass.reloadCurrentShaders();
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
            glGenFramebuffers(1, &FBO);
            glGenTextures(1, &noiseTexture);
            cubemapTexture = LoadCubemap(faces);
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
    void renderDebug(RenderParameters &renderParameters)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gBufferPass.render(renderParameters);
        auto [gPosition, gNormal, gAlbedoSpec] = gBufferPass.getTextures();

        // SSAO Noise Texture
        auto ssaoNoise = Random::GenerateSSAONoise();
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 8, 8, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_2D, gNormal);
        // glActiveTexture(GL_TEXTURE2);
        // glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        quadShader.use();
        quadShader.setTextureAuto(gPosition, GL_TEXTURE_2D, 0, "tex_sampler");
        quadShader.setTextureAuto(noiseTexture, GL_TEXTURE_2D, 0, "texNoise");
        // quadShader.setInt("tex_sampler", 1); // gNormal
        // quadShader.setInt("tex_sampler", 2); // gAlbedoSpec

        glViewport(0, 0, width, height);
        DrawQuad();
    }

    void renderLight(RenderParameters &renderParameters)
    {
        auto &[lights, cam, scene, model, window] = renderParameters;

        // 渲染阴影贴图
        for (auto &light : lights)
        {
            pointShadowPass.renderToTexture(light.depthCubemap, light, scene, model);
        }
        gBufferPass.render(renderParameters);
        auto [gPosition, gNormal, gAlbedoSpec] = gBufferPass.getTextures();

        ssaoPass.render(renderParameters, gPosition, gNormal, gAlbedoSpec);
        auto ssaoPassTexture = ssaoPass.getTextures();

        lightPass.render(renderParameters, gPosition, gNormal, gAlbedoSpec, ssaoPassTexture, pointShadowPass.far);
        auto lightPassTexture = lightPass.getTextures();

        screenPass.render(lightPassTexture);
    }
};
