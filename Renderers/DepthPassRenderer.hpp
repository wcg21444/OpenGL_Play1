#pragma once
#include "Renderer.hpp"
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
        auto &[lights, cam, scene, model, window] = renderParameters;

        // temporary light source variable
        LightSource &light = lights[0]; // Assuming the first light is the one we want to use for shadow

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

        DrawScene(scene, model, shaders);
    }
};
