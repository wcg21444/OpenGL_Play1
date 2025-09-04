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
#include "Passes/SkyTexPass.hpp"
#include "Passes/DownSamplePass.hpp"

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

    const int MAX_POINT_LIGHTS = 10;

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
    SkyTexPass skyTexPass;
    TransmittanceLUTPass transmittanceLUTPass;
    DirShadowVSMPass dirShadowVSMPass;
    PointShadowVSMPass pointShadowVSMPass;

    GBufferRendererGUI rendererGUI;

public:
    GBufferRenderer()
        : gBufferPass(GBufferPass(width, height, "Shaders/GBuffer/gbuffer.vs", "Shaders/GBuffer/gbuffer.fs")),
          lightPass(LightPass(width, height, "Shaders/screenQuad.vs", "Shaders/light.fs")),
          screenPass(ScreenPass(width, height, "Shaders/screenQuad.vs", "Shaders/GBuffer/texture.fs")),
          ssaoPass(SSAOPass(width, height, "Shaders/screenQuad.vs", "Shaders/SSAOPass/ssao.fs")),
          ssaoBlurPass(SSAOBlurPass(width, height, "Shaders/screenQuad.vs", "Shaders/SSAOPass/blur.fs")),
          dirShadowPass(DirShadowPass("Shaders/ShadowDepthTexture/dirShadow.vs", "Shaders/ShadowDepthTexture/dirShadow.fs")),
          pointShadowPass(PointShadowPass("Shaders/ShadowDepthTexture/shadow_depth.vs", "Shaders/ShadowDepthTexture/shadow_depth.fs", "Shaders/ShadowDepthTexture/shadow_depth.gs")),
          postProcessPass(PostProcessPass(width, height, "Shaders/screenQuad.vs", "Shaders/PostProcess/postProcess.fs")),
          bloomPass(BloomPass(width, height, "Shaders/screenQuad.vs", "Shaders/PostProcess/bloom.fs")),
          unfoldPass(CubemapUnfoldPass(width, height, "Shaders/GBuffer/cubemap_unfold_debug.vs", "Shaders/GBuffer/cubemap_unfold_debug.fs", 256)),
          skyTexPass(SkyTexPass("Shaders/cubemapSphere.vs", "Shaders/SkyTexPass/skyTex.fs", 256)),
          transmittanceLUTPass(TransmittanceLUTPass(256, 64, "Shaders/screenQuad.vs", "Shaders/SkyTexPass/transmittanceLUT.fs")),
          dirShadowVSMPass(DirShadowVSMPass("Shaders/screenQuad.vs", "Shaders/ShadowMapping/VSMPreprocessDir.fs")),
          pointShadowVSMPass(PointShadowVSMPass("Shaders/cubemapSphere.vs", "Shaders/ShadowMapping/VSMPreprocessPoint.fs"))
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
        skyTexPass.reloadCurrentShaders();
        transmittanceLUTPass.reloadCurrentShaders();
        dirShadowVSMPass.reloadCurrentShaders();
        pointShadowVSMPass.reloadCurrentShaders();
        contextSetup();
    }

    void contextSetup() override
    {
        static bool initialized = false;
        glEnable(GL_DEPTH_TEST);                // 深度缓冲
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // 无缝Cubemap

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
        // 点光源阴影贴图
        for (auto &light : pointLights)
        {
            light.useVSM = true;
            if (!rendererGUI.toggleVSM)
            {
                light.useVSM = false;
            }
            light.generateShadowTexResource();
            if (rendererGUI.togglePointShadow)
            {
                pointShadowPass.renderToTexture(
                    light,
                    scene,
                    model,
                    light.texResolution,
                    light.texResolution);
                if (light.useVSM)
                {
                    pointShadowVSMPass.renderToVSMTexture(light);
                }
            }
        }
        // 平行光源阴影贴图
        for (auto &light : dirLights)
        {
            light.useVSM = true;
            if (!rendererGUI.toggleVSM)
            {
                light.useVSM = false;
            }
            light.generateShadowTexResource();
            if (rendererGUI.toggleDirShadow)
            {

                dirShadowPass.renderToTexture(
                    light,
                    scene,
                    model,
                    light.texResolution,
                    light.texResolution);
                if (light.useVSM)
                {
                    dirShadowVSMPass.renderToVSMTexture(light, light.texResolution, light.texResolution);
                }
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
        /****************************天空渲染*********************************************/
        transmittanceLUTPass.render();
        auto transmittanceLUTTex = transmittanceLUTPass.getTextures();
        skyTexPass.render(renderParameters, transmittanceLUTTex);
        auto skyPassCubemap = skyTexPass.getCubemap();

        /****************************光照渲染*********************************************/
        lightPass.setToggle(rendererGUI.toggleSkybox, "Skybox");
        lightPass.setToggle(rendererGUI.togglePointShadow, "PointShadow");
        lightPass.setToggle(rendererGUI.toggleDirShadow, "DirShadow");
        lightPass.render(renderParameters,
                         gPosition,
                         gNormal,
                         gAlbedoSpec,
                         skyPassCubemap,
                         transmittanceLUTTex);
        auto lightPassTex = lightPass.getTextures();

        /************************************Bloom*******************************************/
        auto [bloomPassTex0,
              bloomPassTex1,
              bloomPassTex2,
              bloomPassTex3,
              bloomPassTex4] = bloomPass.getTextures();
        if (rendererGUI.toggleBloom)
        {

            bloomPass.render(lightPassTex);
        }
        else
        {
            bloomPassTex0 = 0;
            bloomPassTex1 = 0;
            bloomPassTex2 = 0;
            bloomPassTex3 = 0;
            bloomPassTex4 = 0;
        }
        /****************************PostProcess*********************************************/

        postProcessPass.setToggle(rendererGUI.toggleSSAO, "SSAO");
        postProcessPass.setToggle(rendererGUI.toggleGammaCorrection, "GammaCorrection");
        postProcessPass.setToggle(rendererGUI.toggleHDR, "HDR");
        postProcessPass.setToggle(rendererGUI.toggleVignetting, "Vignetting");
        postProcessPass.setToggle(rendererGUI.toggleBloom, "Bloom");

        postProcessPass.render(
            lightPassTex,
            ssaoBlurTex,
            {bloomPassTex0,
             bloomPassTex1,
             bloomPassTex2,
             bloomPassTex3,
             bloomPassTex4});
        auto postProcessPassTex = postProcessPass.getTextures();
        /****************************Screen渲染*********************************************/
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // screenPass.render(postProcessPassTex); // 渲染到底层窗口

        unfoldPass.render(pointLights[0].VSMCubemap->ID);
        auto unfoldedTex = unfoldPass.getUnfoldedCubemap();

        rendererGUI.renderPassInspector(unfoldedTex);
        // rendererGUI.renderPassInspector(std::vector<GLuint>{bloomPassTex0, bloomPassTex1, bloomPassTex2, bloomPassTex3, bloomPassTex4});

        // rendererGUI.renderPassInspector({gPosition, ssaoBlurTex, lightPassTex});
        ImGui::Begin("RendererGUI");
        {
            ImGui::DragFloat("OrthoScale", &allLights.dirLights[0].orthoScale, 1e1, 1e3);
            ImGui::DragFloat("FarPlane", &allLights.dirLights[0].farPlane, 1e2, 1e7);
            ImGui::End();
        }

        auto renderWindowSize = rendererGUI.getRenderWindowSize();
        resize(static_cast<int>(renderWindowSize.x), static_cast<int>(renderWindowSize.y));
        rendererGUI.renderToDockingWindow(postProcessPassTex);
    }
};
