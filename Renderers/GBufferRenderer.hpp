#pragma once
#include <tuple>

#include "Renderer.hpp"
#include "Passes/DirShadowPass.hpp"
#include "Passes/PointShadowPass.hpp"
#include "../Utils/Random.hpp"
#include "Passes/Pass.hpp"
#include "Passes/LightPass.hpp"
#include "Passes/GBufferPass.hpp"
#include "Passes/SSAOPass.hpp"
#include "Passes/PostProcessPass.hpp"
#include "Passes/GaussianBlurPass.hpp"
#include "Passes/BloomPass.hpp"
#include "Passes/CubemapUnfoldPass.hpp"
#include "../../RendererGUI.hpp"

#include "../Utils/TextureLoader.hpp"
#include "../Utils/Utils.hpp"

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
    BloomPass bloomPass;
    CubemapUnfoldPass unfoldPass;

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
          postProcessPass(PostProcessPass(width, height, "Shaders/screenQuad.vs", "Shaders/PostProcess/postProcess.fs")),
          bloomPass(BloomPass(width, height, "Shaders/screenQuad.vs", "Shaders/PostProcess/bloom.fs")),
          unfoldPass(CubemapUnfoldPass(width, height, "Shaders/GBuffer/cubemap_unfold_debug.vs", "Shaders/GBuffer/cubemap_unfold_debug.fs", 128))
    {
    }
    void reloadCurrentShaders() override
    {
        pointShadowPass.reloadCurrentShaders();
        dirShadowPass.reloadCurrentShaders();
        gBufferPass.reloadCurrentShaders();
        lightPass.reloadCurrentShaders();
        screenPass.reloadCurrentShaders();
        ssaoPass.reloadCurrentShaders();
        ssaoBlurPass.reloadCurrentShaders();
        postProcessPass.reloadCurrentShaders();
        bloomPass.reloadCurrentShaders();
        unfoldPass.reloadCurrentShaders();
        contextSetup();
    }

    void contextSetup() override
    {
        static bool initialized = false;
        glEnable(GL_DEPTH_TEST); // 深度缓冲

        if (!initialized)
        {
            initialized = true;
            glViewport(0, 0, width, height);
            skyboxCube = LoadCubemap(faces);
        }
    }

    void resize(int _width, int _height) override
    {
        // pointShadowPass->resize(_width, _height);
        // dirShadowPass->resize(_width, _height);
        if (width == _width &&
            height == _height)
        {
            return;
        }
        width = _width;
        height = _height;

        gBufferPass.resize(_width, _height);
        lightPass.resize(_width, _height);
        screenPass.resize(_width, _height);
        ssaoPass.resize(_width, _height);
        ssaoBlurPass.resize(_width, _height);
        postProcessPass.resize(_width, _height);
        bloomPass.resize(_width, _height);
    }

    void render(RenderParameters &renderParameters) override
    {
        renderLight(renderParameters);
    }

private:
    void renderLight(RenderParameters &renderParameters)
    {
        auto &[allLights, cam, scene, model, window] = renderParameters;
        auto &[pointLights, dirLights] = allLights;

        rendererGUI.render();

        /****************************阴影贴图渲染*********************************************/
        for (auto &light : pointLights)
        {
            light.generateShadowTexResource();
            if (rendererGUI.togglePointShadow)
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
            if (rendererGUI.toggleDirShadow)
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
        unsigned int ssaoPassTex = 0;
        unsigned int ssaoBlurTex = 0;
        if (rendererGUI.toggleSSAO)
        {
            ssaoPass.render(renderParameters, gPosition, gNormal, gAlbedoSpec, gViewPosition);
            ssaoPassTex = ssaoPass.getTextures();
            ssaoBlurPass.render(ssaoPassTex);
            ssaoBlurTex = ssaoBlurPass.getTextures();
        }
        /****************************光照渲染*********************************************/
        lightPass.setToggle(rendererGUI.toggleSkybox, "Skybox");
        lightPass.setToggle(rendererGUI.togglePointShadow, "PointShadow");
        lightPass.setToggle(rendererGUI.toggleDirShadow, "DirShadow");

        lightPass.render(renderParameters,
                         gPosition,
                         gNormal,
                         gAlbedoSpec,
                         skyboxCube,
                         pointShadowPass.farPlane);
        auto lightPassTex = lightPass.getTextures();
        /************************************Bloom*******************************************/
        unsigned int bloomPassTex = 0;
        if (rendererGUI.toggleBloom)
        {
            bloomPass.render(lightPassTex);
            bloomPassTex = bloomPass.getTextures();
        }
        /****************************PostProcess*********************************************/

        postProcessPass.setToggle(rendererGUI.toggleSSAO, "SSAO");
        postProcessPass.setToggle(rendererGUI.toggleGammaCorrection, "GammaCorrection");
        postProcessPass.setToggle(rendererGUI.toggleHDR, "HDR");
        postProcessPass.setToggle(rendererGUI.toggleVignetting, "Vignetting");
        postProcessPass.setToggle(rendererGUI.toggleBloom, "Bloom");

        postProcessPass.render(lightPassTex, ssaoBlurTex, bloomPassTex);
        auto postProcessPassTex = postProcessPass.getTextures();

        /****************************Screen渲染*********************************************/
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // screenPass.render(postProcessPassTex); // 渲染到底层窗口

        auto renderWindowSize = rendererGUI.getRenderWindowSize();
        resize(static_cast<int>(renderWindowSize.x), static_cast<int>(renderWindowSize.y));
        rendererGUI.renderToDockingWindow(postProcessPassTex);

        unfoldPass.render(skyboxCube);
        auto unfoldedTex = unfoldPass.getUnfoldedCubemap();
        rendererGUI.renderPassInspector(unfoldedTex);
    }
};
