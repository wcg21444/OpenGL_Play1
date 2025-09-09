#pragma once

#include <iostream>

#include "../Shading/Shader.hpp"
#include "../Camera.hpp"
#include "../InputHandler.hpp"
#include "../LightSource/LightSource.hpp"

#include "../Objects/Object.hpp"
#include "../Objects/Grid.hpp"
#include "../Objects/Cube.hpp"
#include "../Objects/Sphere.hpp"
#include "../Objects/Plane.hpp"

class RenderParameters
{
public:
    Lights &lights;
    Camera &cam;
    Scene &scene;
    glm::mat4 &model;
    GLFWwindow *window;
};

class Renderer
{
public:
    // 绘制场景
    static void DrawScene(Scene &scene, glm::mat4 &model, Shader &shaders);

    // 生成Quad并注册到OpenGL. [out]quadVAO,quadVBO
    static void GenerateQuad(unsigned int &quadVAO, unsigned int &quadVBO);

    // 绘制公共Quad .单一职责:不负责视口管理.
    static void DrawQuad();

    // 绘制立方体贴图的球体
    static void DrawSphere();

public:
    // 在切换渲染器时执行
    virtual void contextSetup() = 0;
    virtual void render(RenderParameters &renderParameters) = 0;
    virtual void reloadCurrentShaders() = 0;
    virtual void resize(int _width, int _height) = 0;
    virtual ~Renderer() {}
};
