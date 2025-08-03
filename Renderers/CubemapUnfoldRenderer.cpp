#include "CubemapUnfoldRenderer.hpp"
#include <vector>
#include <tuple>
#include <glad/glad.h>
#include "../Shader.hpp"
#include "../utils/Random.hpp"
#include "Pass.hpp"
#include "../ShaderGUI.hpp"
#include "../utils/TextureLoader.hpp"

CubemapUnfoldRenderer::CubemapUnfoldRenderer()
    : unfoldShaders("Shaders/GBuffer/cubemap_unfold_debug.vs", "Shaders/GBuffer/cubemap_unfold_debug.fs"),
      quadShader("Shaders/GBuffer/texture.vs", "Shaders/GBuffer/texture.fs"),
      width(1600), height(900), quadVAO(0), unfoldFBO(0), unfoldedCubemap(0),
      CUBEMAP_FACE_SIZE(1024) {}

void CubemapUnfoldRenderer::reloadCurrentShaders()
{
    unfoldShaders = Shader("Shaders/GBuffer/cubemap_unfold_debug.vs", "Shaders/GBuffer/cubemap_unfold_debug.fs");
    quadShader = Shader("Shaders/GBuffer/texture.vs", "Shaders/GBuffer/texture.fs");
    pointShadowPass.reloadCurrentShader();
    contextSetup();
}
void CubemapUnfoldRenderer::contextSetup()
{
    static bool initialized = false;
    if (!initialized)
    {
        glGenFramebuffers(1, &unfoldFBO);
        glGenTextures(1, &unfoldedCubemap);
        Renderer::GenerateQuad(quadVAO, quadVBO);
        initialized = true;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, unfoldFBO);
    glBindTexture(GL_TEXTURE_2D, unfoldedCubemap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, CUBEMAP_FACE_SIZE * 4, CUBEMAP_FACE_SIZE * 3, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, unfoldedCubemap, 0);
}
void CubemapUnfoldRenderer::unfoldCubemap(unsigned int cubemap)
{
    glBindFramebuffer(GL_FRAMEBUFFER, unfoldFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    unfoldShaders.use();
    unfoldShaders.setTextureAuto(cubemap, GL_TEXTURE_CUBE_MAP, 5, "u_cubemap");
    glBindVertexArray(quadVAO);
    std::vector<std::tuple<int, int, int>> faceLayout = {
        {2 * CUBEMAP_FACE_SIZE, 1 * CUBEMAP_FACE_SIZE, 0},
        {0 * CUBEMAP_FACE_SIZE, 1 * CUBEMAP_FACE_SIZE, 1},
        {1 * CUBEMAP_FACE_SIZE, 2 * CUBEMAP_FACE_SIZE, 2},
        {1 * CUBEMAP_FACE_SIZE, 0 * CUBEMAP_FACE_SIZE, 3},
        {1 * CUBEMAP_FACE_SIZE, 1 * CUBEMAP_FACE_SIZE, 4},
        {3 * CUBEMAP_FACE_SIZE, 1 * CUBEMAP_FACE_SIZE, 5}};
    for (const auto &faceInfo : faceLayout)
    {
        int xOffset = std::get<0>(faceInfo);
        int yOffset = std::get<1>(faceInfo);
        int faceIndex = std::get<2>(faceInfo);
        glViewport(xOffset, yOffset, CUBEMAP_FACE_SIZE, CUBEMAP_FACE_SIZE);
        unfoldShaders.setInt("u_faceIndex", faceIndex);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void CubemapUnfoldRenderer::render(RenderParameters &renderParameters)
{
    auto &[lights, cam, scene, model, window] = renderParameters;
    static bool initialized = false;
    static std::vector<std::string> faces{
        "Resource/skybox/right.jpg",
        "Resource/skybox/left.jpg",
        "Resource/skybox/top.jpg",
        "Resource/skybox/bottom.jpg",
        "Resource/skybox/front.jpg",
        "Resource/skybox/back.jpg"};
    static auto skyboxCubemap = LoadCubemap(faces);
    if (!initialized)
        initialized = true;
    pointShadowPass.render(lights[0], scene, model);
    unfoldCubemap(skyboxCubemap);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    quadShader.use();
    quadShader.setTextureAuto(unfoldedCubemap, GL_TEXTURE_2D, 10, "tex_sampler");
    Renderer::DrawQuad();
}
