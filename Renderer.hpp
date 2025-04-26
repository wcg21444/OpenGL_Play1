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

class Renderer
{
public:
    virtual void contextSetup() = 0;
    virtual void render(LightSource &light, Camera &cam, std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model, GLFWwindow *window) = 0;
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
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // set up VAO
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
    void render(LightSource &light, Camera &cam, std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model, GLFWwindow *window)
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 配置光源空间的投影 视图 矩阵
        float near_plane = 1.0f, far_plane = 7.5f;
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(light.position,
                                          glm::vec3(0.0f, 0.0f, 0.0f),
                                          glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        quadShader.use();
        quadShader.setInt("depthMap", 0);

        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        glm::mat4 sphere_model = glm::translate(model, glm::vec3(2.f, 0.f, 0.f));
        for (auto &&object : scene)
        {
            if (object->name == "Grid")
                continue;
            if (object->name == "Sphere")
                object->draw(sphere_model, depthShader);
            else
                object->draw(model, depthShader);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        quadShader.use();
        quadShader.setFloat("near_plane", near_plane);
        quadShader.setFloat("far_plane", far_plane);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
};
class NormalRenderer : public Renderer
{
    Shader shaders = Shader("Shaders/VertShader.vs", "Shaders/FragmShader.fs");
    int width = 1600;
    int height = 900;

public:
    void contextSetup()
    {
        glEnable(GL_DEPTH_TEST); // 深度缓冲
        glViewport(0, 0, width, height);
        shaders.use();
    }
    void render(LightSource &light, Camera &cam, std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model, GLFWwindow *window)
    {
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 深度缓冲和Color缓冲一样需要交换链,清理他

        light.set(shaders);

        // camera/view transformation
        cam.setViewMatrix(shaders);
        cam.setPerspectiveMatrix(shaders, width, height);

        glm::mat4 sphere_model = glm::translate(model, glm::vec3(2.f, 0.f, 0.f));
        for (auto &&object : scene)
        {
            if (object->name == "Sphere")
                object->draw(sphere_model, shaders);
            else
                object->draw(model, shaders);
        }
        // 问题:这样做就没法一个Object 的数据 由model matrix 不同而 复制物体
        //  相同的物体重复占用显存
        //  grid.draw(model, shaders);

        // plane.draw(model, shaders);

        // cube.draw(model, shaders);

        // glm::mat4 sphere_model = glm::translate(model, glm::vec3(2.f, 0.f, 0.f));
        // sphere.draw(sphere_model,
        //             shaders);

        // sphere_model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        // sphere_model = glm::translate(sphere_model, glm::vec3(0.f, 0.f, 2.f));
        // sphere.draw(sphere_model, shaders);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
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
        // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
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
    void render(LightSource &light, Camera &cam, std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model, GLFWwindow *window)
    {
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
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
};

class RenderManager
{
    // As you can see , we dont have a unifrom regulation of funcs , we are easily forgetting implement some process , maybe abstract it into a class is a good choice?
private:
    DebugDepthRenderer debugDepthRenderer;
    NormalRenderer normalRenderer;
    SimpleTextureRenderer simpleTextureRenderer;

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
        normal,
        debug_depth,
        simple_texture
    };

private:
    Mode render_mode;

public:
    RenderManager()
    {
        switchMode(normal); // default
    }
    void switchMode(Mode _mode)
    {
        clearContext();
        render_mode = _mode;
        switch (_mode)
        {
        case normal:
            normalRenderer.contextSetup();
            break;
        case debug_depth:
            debugDepthRenderer.contextSetup();
            break;
        case simple_texture:
            simpleTextureRenderer.contextSetup();
            break;
        default:
            throw(std::exception("No Selected Render Mode."));
            break;
        }
    }
    void render(LightSource &light, Camera &cam, std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model, GLFWwindow *window)
    {
        // using parameter list is so tedious
        // Need a lighter way
        switch (render_mode)
        {
        case normal:
            normalRenderer.render(light, cam, scene, model, window);
            break;
        case debug_depth:
            debugDepthRenderer.render(light, cam, scene, model, window);
            break;
        case simple_texture:
            simpleTextureRenderer.render(light, cam, scene, model, window);
        default:
            break;
        }
    }
};