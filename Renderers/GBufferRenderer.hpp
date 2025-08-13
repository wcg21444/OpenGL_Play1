#pragma once
#include <tuple>

#include "Renderer.hpp"
#include "Passes/DirShadowPass.hpp"
#include "Passes/PointShadowPass.hpp"
#include "../utils/Random.hpp"
#include "Passes/Pass.hpp"
#include "Passes/LightPass.hpp"
#include "Passes/GBufferPass.hpp"
#include "Passes/SSAOPass.hpp"

#include "../utils/TextureLoader.hpp"

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
    DirShadowPass dirShadowPass;
    GBufferPass gBufferPass;
    LightPass lightPass;
    ScreenPass screenPass;
    SSAOPass ssaoPass;
    SSAOBlurPass ssaoBlurPass;

public:
    GBufferRenderer()
        : gBufferPass(GBufferPass(width, height, "Shaders/GBuffer/gbuffer.vs", "Shaders/GBuffer/gbuffer.fs")),
          lightPass(LightPass(width, height, "Shaders/GBuffer/light.vs", "Shaders/GBuffer/light.fs")),
          screenPass(ScreenPass(width, height, "Shaders/GBuffer/texture.vs", "Shaders/GBuffer/texture.fs")),
          ssaoPass(SSAOPass(width, height, "Shaders/SSAOPass/ssao.vs", "Shaders/SSAOPass/ssao.fs")),
          ssaoBlurPass(SSAOBlurPass(width, height, "Shaders/SSAOPass/blur.vs", "Shaders/SSAOPass/blur.fs")),
          dirShadowPass(DirShadowPass("Shaders/DirShadow/dirShadow.vs", "Shaders/DirShadow/dirShadow.fs")),
          pointShadowPass(PointShadowPass("Shaders/PointShadow/shadow_depth.vs", "Shaders/PointShadow/shadow_depth.fs", "Shaders/PointShadow/shadow_depth.gs"))
    {
    }
    void reloadCurrentShaders()
    {
        quadShader = Shader("Shaders/GBuffer/texture.vs", "Shaders/GBuffer/texture.fs");
        pointShadowPass.reloadCurrentShaders();
        dirShadowPass.reloadCurrentShaders();
        gBufferPass.reloadCurrentShaders();
        lightPass.reloadCurrentShaders();
        screenPass.reloadCurrentShaders();
        ssaoPass.reloadCurrentShaders();
        ssaoBlurPass.reloadCurrentShaders();
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
        auto [gPosition, gNormal, gAlbedoSpec, gViewPosition] = gBufferPass.getTextures();

        // SSAO Noise Texture
        auto ssaoNoise = Random::GenerateNoise();
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
        Renderer::DrawQuad();
    }

    void renderLight(RenderParameters &renderParameters)
    {
        auto &[allLights, cam, scene, model, window] = renderParameters;
        auto &[pointLights, dirLights] = allLights;
        /****************************阴影贴图渲染*********************************************/
        for (auto &light : pointLights)
        {
            light.generateShadowTexResource();
            pointShadowPass.renderToTexture(
                light,
                scene,
                model,
                light.texResolution,
                light.texResolution);
        }

        for (auto &light : dirLights)
        {
            light.generateShadowTexResource();
            dirShadowPass.renderToTexture(
                light,
                scene,
                model,
                light.texResolution,
                light.texResolution);
        }

        /****************************GBuffer渲染*********************************************/
        gBufferPass.render(renderParameters);
        auto [gPosition, gNormal, gAlbedoSpec, gViewPosition] = gBufferPass.getTextures();

        /****************************SSAO渲染*********************************************/

        ssaoPass.render(renderParameters, gPosition, gNormal, gAlbedoSpec, gViewPosition);
        auto ssaoPassTexture = ssaoPass.getTextures();
        ssaoBlurPass.render(ssaoPassTexture);
        auto ssaoBlurTex = ssaoBlurPass.getTextures();
        /****************************光照渲染*********************************************/
        lightPass.render(renderParameters,
                         gPosition,
                         gNormal,
                         gAlbedoSpec,
                         ssaoBlurTex,
                         cubemapTexture,
                         pointShadowPass.far);
        auto lightPassTexture = lightPass.getTextures();

        /****************************Screen渲染*********************************************/
        screenPass.render(lightPassTexture);
    }
};
