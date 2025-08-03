#pragma once
#include "Renderer.hpp"
class DebugDepthRenderer : public Renderer
{
    Shader depthShader = Shader("Shaders/shadow_depth.vs", "Shaders/shadow_depth.fs");
    Shader quadShader = Shader("Shaders/debug_quad.vs", "Shaders/debug_quad.fs");
    int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    int SCR_WIDTH = 1600, SCR_HEIGHT = 900;
    unsigned int depthMapFBO;
    unsigned int depthMap;
    unsigned int quadVAO = 0;
    unsigned int quadVBO;

public:
    void reloadCurrentShaders()
    {
        depthShader = std::move(Shader("Shaders/shadow_depth.vs", "Shaders/shadow_depth.fs"));
        quadShader = std::move(Shader("Shaders/debug_quad.vs", "Shaders/debug_quad.fs"));
        contextSetup();
    }
    void contextSetup()
    {
        glEnable(GL_DEPTH_TEST);
        glGenFramebuffers(1, &depthMapFBO);
        // create depth texture
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // attach depth texture as FBO's depth buffer
        // Shadow Pass render settings
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
        auto &[lights, cam, scene, model, window] = renderParameters;
        // temporary light source variable
        LightSource &light = lights[0]; // Assuming the first light is the one we want to use for shadow
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 配置光源空间的投影 视图 矩阵
        float near_plane = 0.1f, far_plane = 70.f;
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(light.position,
                                          glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        DrawScene(scene, model, depthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        quadShader.use();
        quadShader.setFloat("near_plane", near_plane);
        quadShader.setFloat("far_plane", far_plane);

        quadShader.setTextureAuto(depthMap, GL_TEXTURE_2D, 0, "depthMap");

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }
};
