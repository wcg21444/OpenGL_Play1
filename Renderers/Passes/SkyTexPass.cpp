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
    glDeleteTextures(1, &skyCubemapTex);
}

inline void SkyTexPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);

    glGenTextures(1, &skyCubemapTex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyCubemapTex);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F,
                     cubemapSize, cubemapSize, 0, GL_RGBA, GL_FLOAT, NULL);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // 三线性插值

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    // glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
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

    static std::vector<glm::mat4> camTransforms;
    glViewport(0, 0, cubemapSize, cubemapSize);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    shaders.use();
    if (!shaders.used)
        throw(std::exception("Shader failed to setup."));
    for (unsigned int i = 0; i < 6; ++i)
    {
        cam.setCubemapViewMatrix(shaders, i);
        auto cubemapProjection = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);
        shaders.setMat4("projection", cubemapProjection);
        shaders.setTextureAuto(transmittanceLUT, GL_TEXTURE_2D, 0, "transmittanceLUT");
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, skyCubemapTex, 0);
        Renderer::DrawSphere();
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }
    /****************************************方向光源输入**************************************************/
    allLights.dirLights[0].setToShader(shaders);

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
    // cam.setViewMatrix(shaders);
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

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int SkyTexPass::getCubemap()
{
    return skyCubemapTex;
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