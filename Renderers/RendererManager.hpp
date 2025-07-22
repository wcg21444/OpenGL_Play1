#pragma once
#include "Renderers/DebugDepthRenderer.hpp"
#include "Renderers/ShadowRenderer.hpp"
#include "Renderers/SimpleTextureRenderer.hpp"
#include "Renderers/DepthPassRenderer.hpp"
#include "Renderers/GBufferRenderer.hpp"
#include "Renderer.hpp"

class RenderManager
{
private:
    DebugDepthRenderer debugDepthRenderer;
    ShadowRenderer shadowRenderer;
    SimpleTextureRenderer simpleTextureRenderer;
    DepthPassRenderer depthPassRenderer;
    GBufferRenderer gbufferRenderer;
    Renderer *currentRenderer = nullptr;

private:
    void clearContext()
    {
        // 解绑所有主要对象
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindVertexArray(0);
        glUseProgram(0);

        // 清理纹理绑定
        GLint maxTextureUnits;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
        for (int i = 0; i < maxTextureUnits; ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        // 重置常用状态
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // 检查错误
    }

public:
    enum Mode
    {
        point_shadow,
        parallel_shadow,
        debug_depth,
        simple_texture,
        depth_pass,
        gbuffer
    };

public:
    void reloadShadowShaders(Shader &&mainShader, Shader &&pointShadowShader)
    {
        if (currentRenderer == &shadowRenderer)
        {
            shadowRenderer.reloadShaders(std::move(mainShader), std::move(pointShadowShader));
        }
        else
        {
            throw(std::exception("ShadowRenderer is not the current renderer."));
        }
    }
    void reloadCurrentShaders()
    {
        if (currentRenderer)
        {
            currentRenderer->reloadCurrentShaders();
        }
    }

    RenderManager()
    {
        switchMode(gbuffer); // default
    }
    void switchMode(Mode _mode)
    {
        switch (_mode)
        {
        case point_shadow:
            currentRenderer = &shadowRenderer;
            shadowRenderer.render_mode = ShadowRenderer::ShadowMode::point_shadow;
            break;
        case parallel_shadow:
            currentRenderer = &shadowRenderer;
            shadowRenderer.render_mode = ShadowRenderer::ShadowMode::parallel_shadow;
            break;
        case debug_depth:
            currentRenderer = &debugDepthRenderer;
            break;
        case simple_texture:
            currentRenderer = &simpleTextureRenderer;
            break;
        case depth_pass:
            currentRenderer = &depthPassRenderer;
            break;
        case gbuffer:
            currentRenderer = &gbufferRenderer;
            break;
        default:
            throw(std::exception("No Selected Render Mode."));
            break;
        }
        switchContext();
    }
    void switchContext()
    {
        clearContext();
        if (currentRenderer)
        {
            currentRenderer->contextSetup();
        }
        else
        {
            throw(std::exception("No Renderer Selected."));
        }
    }
    void render(RenderParameters &renderParameters)
    {
        if (currentRenderer)
        {
            currentRenderer->render(renderParameters);
        }
        else
        {
            throw(std::exception("No Renderer Selected."));
        }
    }
};