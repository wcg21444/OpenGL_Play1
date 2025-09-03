#include "SkyTexPass.hpp"
#include "../../ShaderGUI.hpp"
SkyTexPass::SkyTexPass(std::string _vs_path, std::string _fs_path, int _cubemapSize)
    : Pass(0, 0, _vs_path, _fs_path),
      cubemapSize(_cubemapSize),
      aspect(1.0f),
      nearPlane(0.1f),
      farPlane(1e4),
      camProj(glm::perspective(glm::radians(90.0f), aspect, nearPlane, farPlane))
{
    initializeGLResources();
    contextSetup();
}

SkyTexPass::~SkyTexPass()
{
}

inline void SkyTexPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);
    skyCubemapTex.Generate(cubemapSize, cubemapSize, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
}

inline void SkyTexPass::contextSetup()
{
}

inline void SkyTexPass::resize(int _width, int _height)
{
    contextSetup();
}

// 输入光源的Tex对象,绑定Tex对象到FBO,结果输出到Tex.
void SkyTexPass::render(
    RenderParameters &renderParameters,
    unsigned int transmittanceLUT)
{

    auto &[allLights, cam, scene, model, window] = renderParameters;

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glViewport(0, 0, cubemapSize, cubemapSize);

    shaders.use();
    if (!shaders.used)
        throw(std::exception("Shader failed to setup."));

    /****************************************视口设置***************************************************/
    shaders.setUniform("width", cubemapSize);
    shaders.setUniform("height", cubemapSize);
    /****************************************摄像机设置**************************************************/
    shaders.setUniform3fv("eyePos", cam.getPosition());
    shaders.setUniform3fv("eyeFront", cam.getFront());
    shaders.setUniform3fv("eyeUp", cam.getUp());
    shaders.setFloat("farPlane", cam.farPlane);
    shaders.setFloat("nearPlane", cam.nearPlane);
    shaders.setFloat("fov", cam.fov);
    /****************************************天空设置*****************************************************/
    shaders.setFloat("skyHeight", SkyGUI::skyHeight);
    shaders.setFloat("earthRadius", SkyGUI::earthRadius);
    shaders.setFloat("skyIntensity", SkyGUI::skyIntensity);
    shaders.setInt("maxStep", SkyGUI::maxStep);
    shaders.setFloat("HRayleigh", SkyGUI::HRayleigh);
    shaders.setFloat("HMie", SkyGUI::HMie);
    shaders.setFloat("atmosphereDensity", SkyGUI::atmosphereDensity);
    shaders.setFloat("MieDensity", SkyGUI::MieDensity);
    shaders.setFloat("gMie", SkyGUI::gMie);
    shaders.setFloat("absorbMie", SkyGUI::absorbMie);
    shaders.setFloat("MieIntensity", SkyGUI::MieIntensity);
    shaders.setUniform("betaMie", SkyGUI::betaMie);
    /****************************************方向光源输入**************************************************/
    shaders.setTextureAuto(transmittanceLUT, GL_TEXTURE_2D, 0, "transmittanceLUT");

    for (unsigned int i = 0; i < 6; ++i)
    {
        cam.setCubemapViewMatrix(shaders, i);
        auto cubemapProjection = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);
        shaders.setMat4("projection", cubemapProjection);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               TextureCube::FaceTargets[i], skyCubemapTex.ID, 0);

        Renderer::DrawSphere();
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    allLights.dirLights[0].setToShader(shaders);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int SkyTexPass::getCubemap()
{
    return skyCubemapTex.ID;
}

void TransmittanceLUTPass::render()
{
    glViewport(0, 0, vp_width, vp_height);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    shaders.use();

    shaders.setInt("width", vp_width);
    shaders.setInt("height", vp_height);

    shaders.setFloat("skyHeight", SkyGUI::skyHeight);
    shaders.setFloat("earthRadius", SkyGUI::earthRadius);
    shaders.setFloat("skyIntensity", SkyGUI::skyIntensity);
    shaders.setInt("maxStep", SkyGUI::maxStep);
    shaders.setFloat("HRayleigh", SkyGUI::HRayleigh);
    shaders.setFloat("HMie", SkyGUI::HMie);
    shaders.setFloat("atmosphereDensity", SkyGUI::atmosphereDensity);
    shaders.setFloat("MieDensity", SkyGUI::MieDensity);
    shaders.setFloat("gMie", SkyGUI::gMie);
    shaders.setFloat("absorbMie", SkyGUI::absorbMie);
    shaders.setFloat("MieIntensity", SkyGUI::MieIntensity);
    shaders.setUniform("betaMie", SkyGUI::betaMie);

    Renderer::DrawQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}