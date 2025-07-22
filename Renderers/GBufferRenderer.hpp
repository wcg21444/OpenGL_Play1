#pragma once
#include "RendererManager.hpp"
#include "Renderer.hpp"
class GBufferRenderer : public Renderer
{
private:
    Shader gShaders = Shader("Shaders/GBuffer/gbuffer.vs", "Shaders/GBuffer/gbuffer.fs");
    Shader quadShader = Shader("Shaders/GBuffer/texture.vs", "Shaders/GBuffer/texture.fs");
    int width = 1600;
    int height = 900;

    unsigned int quadVAO = 0;
    unsigned int quadVBO;

    unsigned int gBuffer;
    unsigned int gPosition, gNormal, gAlbedoSpec;
    unsigned int depthMap;

public:
    void reloadCurrentShaders()
    {
        gShaders = Shader("Shaders/GBuffer/gbuffer.vs", "Shaders/GBuffer/gbuffer.fs");
        quadShader = Shader("Shaders/GBuffer/texture.vs", "Shaders/GBuffer/texture.fs");
        contextSetup();
    }
    void contextSetup()
    {
        glEnable(GL_DEPTH_TEST); // 深度缓冲

        glGenFramebuffers(1, &gBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        // create depth texture
        // - position color buffer
        glGenTextures(1, &gPosition);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

        // - normal color buffer
        glGenTextures(1, &gNormal);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

        // - color + specular color buffer
        glGenTextures(1, &gAlbedoSpec);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

        // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
        unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, attachments);

        glGenRenderbuffers(1, &depthMap);
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

#define DEBUG_GBUFFER
        // #define RENDER_GBUFFER

#ifdef DEBUG_GBUFFER
        renderDebug(renderParameters);
#endif
#ifdef RENDER_GBUFFER
        renderLight(renderParameters)
#endif
    }

private:
    void renderGBuffer(RenderParameters &renderParameters)
    {
        auto &[lights, cam, scene, model, window] = renderParameters;
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        {
            gShaders.use();
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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
        quadShader.setTexture(gPosition, GL_TEXTURE_2D, 0, "tex_sampler");
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
        renderGBuffer(renderParameters);
        // 绑定 GBuffer Texture 到Quad

        // 绘制Quad
    }
};
