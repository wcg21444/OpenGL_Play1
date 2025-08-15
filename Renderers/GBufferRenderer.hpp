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
#include "Passes/PostProcessPass.hpp"

#include "../../RendererGUI.hpp"

#include "../utils/TextureLoader.hpp"

class GBufferRenderer : public Renderer
{
private:
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
    unsigned int skyboxCube;

    const int MAX_LIGHTS = 10;

    PointShadowPass pointShadowPass;
    DirShadowPass dirShadowPass;
    GBufferPass gBufferPass;
    LightPass lightPass;
    ScreenPass screenPass;
    SSAOPass ssaoPass;
    SSAOBlurPass ssaoBlurPass;
    PostProcessPass postProcessPass;

    bool togglePointShadow;
    bool toggleDirShadow;
    bool toggleGBuffer;
    bool toggleLight;
    bool toggleSSAO;
    bool toggleSSAOBlur;
    bool toggleHDR;
    bool toggleVignetting;
    bool toggleGammaCorrection;

    GBufferRendererGUI rendererGUI;

public:
    GBufferRenderer()
        : gBufferPass(GBufferPass(width, height, "Shaders/GBuffer/gbuffer.vs", "Shaders/GBuffer/gbuffer.fs")),
          lightPass(LightPass(width, height, "Shaders/screenQuad.vs", "Shaders/GBuffer/light.fs")),
          screenPass(ScreenPass(width, height, "Shaders/screenQuad.vs", "Shaders/GBuffer/texture.fs")),
          ssaoPass(SSAOPass(width, height, "Shaders/screenQuad.vs", "Shaders/SSAOPass/ssao.fs")),
          ssaoBlurPass(SSAOBlurPass(width, height, "Shaders/screenQuad.vs", "Shaders/SSAOPass/blur.fs")),
          dirShadowPass(DirShadowPass("Shaders/DirShadow/dirShadow.vs", "Shaders/DirShadow/dirShadow.fs")),
          pointShadowPass(PointShadowPass("Shaders/PointShadow/shadow_depth.vs", "Shaders/PointShadow/shadow_depth.fs", "Shaders/PointShadow/shadow_depth.gs")),
          postProcessPass(PostProcessPass(width, height, "Shaders/screenQuad.vs", "Shaders/PostProcess/postProcess.fs"))
    {
    }
    void reloadCurrentShaders()
    {
        pointShadowPass.reloadCurrentShaders();
        dirShadowPass.reloadCurrentShaders();
        gBufferPass.reloadCurrentShaders();
        lightPass.reloadCurrentShaders();
        screenPass.reloadCurrentShaders();
        ssaoPass.reloadCurrentShaders();
        ssaoBlurPass.reloadCurrentShaders();
        postProcessPass.reloadCurrentShaders();
        contextSetup();
    }

    void contextSetup()
    {
        static bool initialized = false;
        glEnable(GL_DEPTH_TEST); // 深度缓冲

        if (!initialized)
        {
            initialized = true;
            glViewport(0, 0, width, height);
            glGenFramebuffers(1, &FBO);
            skyboxCube = LoadCubemap(faces);
        }
    }

    void render(RenderParameters &renderParameters)
    {
        renderLight(renderParameters);
    }

private:
    void renderLight(RenderParameters &renderParameters)
    {
        auto &[allLights, cam, scene, model, window] = renderParameters;
        auto &[pointLights, dirLights] = allLights;

        toggleDirShadow = rendererGUI.toggleDirShadow;
        togglePointShadow = rendererGUI.togglePointShadow;
        toggleSSAO = rendererGUI.toggleSSAO;
        toggleHDR = rendererGUI.toggleHDR;
        toggleGammaCorrection = rendererGUI.toggleGammaCorrection;
        toggleVignetting = rendererGUI.toggleVignetting;

        rendererGUI.render();

        /****************************阴影贴图渲染*********************************************/
        for (auto &light : pointLights)
        {
            light.generateShadowTexResource();
            if (togglePointShadow)
            {
                pointShadowPass.renderToTexture(
                    light,
                    scene,
                    model,
                    light.texResolution,
                    light.texResolution);
            }
        }

        for (auto &light : dirLights)
        {
            light.generateShadowTexResource();
            if (toggleDirShadow)
            {
                dirShadowPass.renderToTexture(
                    light,
                    scene,
                    model,
                    light.texResolution,
                    light.texResolution);
            }
        }

        /****************************GBuffer渲染*********************************************/
        gBufferPass.render(renderParameters);
        auto [gPosition, gNormal, gAlbedoSpec, gViewPosition] = gBufferPass.getTextures();

        /****************************SSAO渲染*********************************************/
        unsigned int ssaoPassTexture = 0;
        unsigned int ssaoBlurTex = 0;
        if (toggleSSAO)
        {
            ssaoPass.render(renderParameters, gPosition, gNormal, gAlbedoSpec, gViewPosition);
            ssaoPassTexture = ssaoPass.getTextures();
            ssaoBlurPass.render(ssaoPassTexture);
            ssaoBlurTex = ssaoBlurPass.getTextures();
        }
        /****************************光照渲染*********************************************/
        lightPass.setToggle(togglePointShadow, "PointShadow");

        lightPass.setToggle(toggleDirShadow, "DirShadow");

        lightPass.render(renderParameters,
                         gPosition,
                         gNormal,
                         gAlbedoSpec,
                         skyboxCube,
                         pointShadowPass.far);
        auto lightPassTexture = lightPass.getTextures();

        /****************************PostProcess*********************************************/

        postProcessPass.setToggle(toggleSSAO, "SSAO");
        postProcessPass.setToggle(toggleGammaCorrection, "GammaCorrection");
        postProcessPass.setToggle(toggleHDR, "HDR");
        postProcessPass.setToggle(toggleVignetting, "Vignetting");

        postProcessPass.render(lightPassTexture, ssaoBlurTex);
        auto postProcessTex = postProcessPass.getTextures();

        /****************************Screen渲染*********************************************/
        screenPass.render(postProcessTex);
    }
};
