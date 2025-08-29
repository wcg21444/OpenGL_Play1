#include "CubemapUnfoldPass.hpp"
#include <vector>
#include <tuple>
#include <glad/glad.h>
#include "../../Shading/Shader.hpp"
#include "../../Utils/Random.hpp"
#include "Pass.hpp"
#include "../../Utils/TextureLoader.hpp"

CubemapUnfoldPass::CubemapUnfoldPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path, int _CUBEMAP_FACE_SIZE)
    : Pass(_vp_width, _vp_height, _vs_path, _fs_path), CUBEMAP_FACE_SIZE(_CUBEMAP_FACE_SIZE)
{
    initializeGLResources();
    contextSetup();
}

void CubemapUnfoldPass::initializeGLResources()
{

    glGenFramebuffers(1, &FBO);
    unfoldedTex.Generate(CUBEMAP_FACE_SIZE * 4, CUBEMAP_FACE_SIZE * 3, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);

    Renderer::GenerateQuad(quadVAO, quadVBO);
}

CubemapUnfoldPass::~CubemapUnfoldPass()
{
    glDeleteFramebuffers(1, &FBO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
}

void CubemapUnfoldPass::contextSetup()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, unfoldedTex.ID, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void CubemapUnfoldPass::unfoldCubemap(unsigned int cubemap)
{
    // 将此函数置为空函数, 天空盒正常渲染
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shaders.use();
    shaders.setTextureAuto(cubemap, GL_TEXTURE_CUBE_MAP, 5, "u_cubemap");
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
        shaders.setInt("u_faceIndex", faceIndex);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void CubemapUnfoldPass::resize(int _width, int _height)
{
    vp_width = _width;
    vp_height = _height;

    contextSetup();
}

void CubemapUnfoldPass::render(unsigned int cubemap)
{
    unfoldCubemap(cubemap);
}

unsigned int CubemapUnfoldPass::getUnfoldedCubemap() const
{
    return unfoldedTex.ID;
}