#include "CubemapUnfoldRenderer.hpp"
#include <vector>
#include <tuple>
#include <glad/glad.h>
#include "../Shading/Shader.hpp"
#include "../Utils/Random.hpp"
#include "../Utils/TextureLoader.hpp"
#include "../../Utils/Utils.hpp"

CubemapUnfoldRenderer::CubemapUnfoldRenderer(int _width, int _height)
    : unfoldPass(CubemapUnfoldPass(_width, _height, "Shaders/GBuffer/cubemap_unfold_debug.vs", "Shaders/GBuffer/cubemap_unfold_debug.fs", 256)),
      screenPass(ScreenPass(_width, _height, "Shaders/screenQuad.vs", "Shaders/GBuffer/texture.fs")),
      width(_width), height(_height)
{
}

void CubemapUnfoldRenderer::reloadCurrentShaders()
{
    unfoldPass.reloadCurrentShaders();
    contextSetup();
}
void CubemapUnfoldRenderer::contextSetup()
{
}

void CubemapUnfoldRenderer::resize(int _width, int _height)
{
    width = _width;
    height = _height;

    contextSetup();
}
void CubemapUnfoldRenderer::render(RenderParameters &renderParameters)
{
    auto &[allLights, cam, scene, model, window] = renderParameters;
    auto &[pointLights, dirLights] = allLights;

    static std::vector<std::string> faces{
        "Resource/skybox/right.jpg",
        "Resource/skybox/left.jpg",
        "Resource/skybox/top.jpg",
        "Resource/skybox/bottom.jpg",
        "Resource/skybox/front.jpg",
        "Resource/skybox/back.jpg"};

    static auto skyboxCubemap = LoadCubemap(faces);

    unfoldPass.unfoldCubemap(skyboxCubemap);
    auto unfoldedCubemap = unfoldPass.getUnfoldedCubemap();
    screenPass.render(unfoldedCubemap);
}
