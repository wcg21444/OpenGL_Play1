#pragma once

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "includes/stb_image.h"

#include "Shader.hpp"
#include "Camera.hpp"
#include "InputHandler.hpp"
#include "LightSource.hpp"

#include "Objects/Object.hpp"
#include "Objects/Grid.hpp"
#include "Objects/Cube.hpp"
#include "Objects/Sphere.hpp"
#include "Objects/Plane.hpp"
class RenderManager;

void ShowGLMMatrixAsTable(const glm::mat4 &matrix, const char *name = "Matrix")
{
    if (ImGui::BeginTable(name, 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        for (int row = 0; row < 4; ++row)
        {
            ImGui::TableNextRow();
            for (int col = 0; col < 4; ++col)
            {
                ImGui::TableSetColumnIndex(col);
                ImGui::Text("%.3f", matrix[col][row]);
            }
        }
        ImGui::EndTable();
    }
}

void renderScene(std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model, Shader &shaders)
{
    glm::mat4 sphere_model = glm::translate(model, glm::vec3(6.f, 0.f, 0.f));
    glm::mat4 plane_model = glm::translate(model, glm::vec3(0.f, -1.f, 0.f));
    glm::mat4 backPack_model = glm::translate(model, glm::vec3(0.f, 2.f, 4.f));
    glm::mat4 bass_model = glm::translate(model, glm::vec3(0.f, 4.f, 4.f));
    bass_model = glm::scale(bass_model, glm::vec3(4.f, 4.f, 4.f));
    for (auto &&object : scene)
    {
        try
        {
            if (object->name == "Sphere")
                object->draw(sphere_model, shaders);
            else if (object->name == "Plane")
                object->draw(plane_model, shaders);
            else if (object->name == "Backpack")
            {
                object->draw(backPack_model, shaders);
            }
            else if (object->name == "Bass")
            {
                object->draw(bass_model, shaders);
            }
            else if (object->name == "Grid")
                continue;
            else
                object->draw(model, shaders);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error rendering object '" << object->name << "': " << e.what() << std::endl;
        }
    }
}

struct RenderParameters
{
    LightSource &light;
    Camera &cam;
    std::vector<std::unique_ptr<Object>> &scene;
    glm::mat4 &model;
    GLFWwindow *window;
};

class Renderer
{
public:
    virtual void contextSetup() = 0;
    virtual void render(RenderParameters &renderParameters) = 0;
    virtual void reloadCurrentShaders() = 0;
    virtual ~Renderer() {}
};

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
        auto &[light, cam, scene, model, window] = renderParameters;
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

        renderScene(scene, model, depthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        quadShader.use();
        quadShader.setInt("depthMap", 0);
        quadShader.setFloat("near_plane", near_plane);
        quadShader.setFloat("far_plane", far_plane);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
    }
};

class ShadowRenderer : public Renderer
{
    class ParrllelShadowPass
    {
        Shader depthShader = Shader("Shaders/shadow_depth.vs", "Shaders/shadow_depth.fs");
        int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
        int SCR_WIDTH = 1600, SCR_HEIGHT = 900;
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

            renderScene(scene, model, depthShader);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    };

    class PointShadowPass
    {
        Shader depthShader = Shader("Shaders/PointShadow/shadow_depth.vs", "Shaders/PointShadow/shadow_depth.fs", "Shaders/PointShadow/shadow_depth.gs");

        int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
        int SCR_WIDTH = 1600, SCR_HEIGHT = 900;
        unsigned int depthMapFBO;

        float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
        float near = 1.0f;

        glm::mat4 shadowProj;
        std::vector<glm::mat4> shadowTransforms;

    public:
        unsigned int depthCubemap;
        float far = 250.0f;

    public:
        PointShadowPass()
        {
            glEnable(GL_DEPTH_TEST);
            glGenFramebuffers(1, &depthMapFBO);
            // create depth Cubemap
            glGenTextures(1, &depthCubemap);
            glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
            for (unsigned int i = 0; i < 6; ++i)
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                             SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            // attach depth cubemap as FBO's depth buffer
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // config light space transformation
            shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);
        }
        void render(LightSource &light, std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model)
        {
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

            glClearColor(0.f, 0.f, 0.f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);

            depthShader.use();
            if (!depthShader.used)
                throw(std::exception("Shader failed to setup."));
            for (unsigned int i = 0; i < 6; ++i)
                depthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
            depthShader.setFloat("far_plane", far);
            depthShader.setUniform3fv("lightPos", light.position);

            renderScene(scene, model, depthShader);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    };

public:
    enum class ShadowMode
    {
        parallel_shadow,
        point_shadow
    };

private:
    Shader pls_shaders = Shader("Shaders/VertShader.vs", "Shaders/FragmShader.fs");
    Shader ps_shaders = Shader("Shaders/PointShadow/point_shadow.vs", "Shaders/PointShadow/point_shadow.fs");
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
        auto &[light, cam, scene, model, window] = renderParameters;
        static PointShadowPass pointShadowPass;
        pointShadowPass.render(light, scene, model);

        glEnable(GL_DEPTH_TEST); // 深度缓冲
        glViewport(0, 0, width, height);
        ps_shaders.use();
        ps_shaders.setInt("depthMap", 0);
        ps_shaders.setFloat("far_plane", pointShadowPass.far);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, pointShadowPass.depthCubemap);
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 深度缓冲和Color缓冲一样需要交换链,清理他

        light.setToShader(ps_shaders);

        // camera/view transformation
        cam.setViewMatrix(ps_shaders);
        cam.setPerspectiveMatrix(ps_shaders, width, height);

        renderScene(scene, model, ps_shaders);
    }
    void renderParallelShadow(RenderParameters &renderParameters)
    {
        auto &[light, cam, scene, model, window] = renderParameters;
        static float near_plane = 1.f, far_plane = 700.f;
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(light.position,
                                          glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        ShowGLMMatrixAsTable(lightSpaceMatrix);
        static ParrllelShadowPass parrllelShadowPass;
        parrllelShadowPass.render(light, scene, model, lightSpaceMatrix);

        glEnable(GL_DEPTH_TEST); // 深度缓冲
        glViewport(0, 0, width, height);
        pls_shaders.use();
        pls_shaders.setInt("shdaowDepthMap", 0);
        pls_shaders.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, parrllelShadowPass.depthMap);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 深度缓冲和Color缓冲一样需要交换链,清理他

        light.setToShader(pls_shaders);

        // camera/view transformation
        cam.setViewMatrix(pls_shaders);
        cam.setPerspectiveMatrix(pls_shaders, width, height);

        renderScene(scene, model, pls_shaders);
    }
};

class SimpleTextureRenderer : public Renderer
{
    Shader shaders = Shader("Shaders/texture.vs", "Shaders/texture.fs");
    int width = 1600;
    int height = 900;
    unsigned int VBO, VAO, EBO;
    unsigned int texture1, texture2;

public:
    void reloadCurrentShaders()
    {
        shaders = std::move(Shader("Shaders/texture.vs", "Shaders/texture.fs"));
        contextSetup();
    }
    void contextSetup()
    {
        static float vertices[] = {
            // positions          // colors           // texture coords
            0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,   // top right
            0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
            -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left
        };
        static unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
        };
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
        // color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // texture coord attribute
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        // texture 1
        // ---------
        glGenTextures(1, &texture1);
        glBindTexture(GL_TEXTURE_2D, texture1);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // load image, create texture and generate mipmaps

        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
        unsigned char *data = stbi_load("textures/container.jpg", &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);

        // texture 2
        // ---------
        glGenTextures(1, &texture2);
        glBindTexture(GL_TEXTURE_2D, texture2);
        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // load image, create texture and generate mipmaps
        data = stbi_load("textures/awesomeface.png", &width, &height, &nrChannels, 0);
        if (data)
        {
            // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);

        shaders.use();
        shaders.setInt("texture1", 0);
        shaders.setInt("texture2", 1);
    }
    void render(RenderParameters &renderParameters)
    {
        auto &[light, cam, scene, model, window] = renderParameters;
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        // render container
        shaders.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // glm::mat4 sphere_model = glm::translate(model, glm::vec3(2.f, 0.f, 0.f));
        // for (auto &&object : scene)
        // {
        //     if (object->name == "Sphere")
        //         object->draw(sphere_model, shaders);
        // }
    }
};

class DepthPassRenderer : public Renderer
{
    Shader shaders = Shader("Shaders/DepthPass/depth_pass.vs", "Shaders/DepthPass/depth_pass.fs");
    int width = 1600;
    int height = 900;

public:
    void reloadCurrentShaders()
    {
        shaders = std::move(Shader("Shaders/DepthPass/depth_pass.vs", "Shaders/DepthPass/depth_pass.fs"));
        contextSetup();
    }
    void contextSetup()
    {
        glEnable(GL_DEPTH_TEST); // 深度缓冲
        glViewport(0, 0, width, height);
    }
    void render(RenderParameters &renderParameters)
    {
        auto &[light, cam, scene, model, window] = renderParameters;

        glEnable(GL_DEPTH_TEST); // 深度缓冲
        glViewport(0, 0, width, height);
        shaders.use();
        shaders.setFloat("far_plane", cam.far);
        shaders.setUniform3fv("camPos", cam.getPosition());

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 深度缓冲和Color缓冲一样需要交换链,清理他

        light.setToShader(shaders);

        // camera/view transformation
        cam.setViewMatrix(shaders);
        cam.setPerspectiveMatrix(shaders, width, height);

        renderScene(scene, model, shaders);
    }
};

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
        auto &[light, cam, scene, model, window] = renderParameters;
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        {
            glEnable(GL_DEPTH_TEST); // 深度缓冲
            gShaders.use();
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            cam.setViewMatrix(gShaders);
            cam.setPerspectiveMatrix(gShaders, width, height);
            renderScene(scene, model, gShaders);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void renderDebug(RenderParameters &renderParameters)
    {

        renderGBuffer(renderParameters);
        // 绑定 GBuffer Texture 到Quad
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        quadShader.use();
        quadShader.setInt("tex_sampler", 0); // gPosition
        // quadShader.setInt("tex_sampler", 1); // gNormal
        // quadShader.setInt("tex_sampler", 2); // gAlbedoSpec

        // 绘制Quad
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 深度缓冲
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
        switchMode(point_shadow); // default
    }
    void switchMode(Mode _mode)
    {
        clearContext();
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