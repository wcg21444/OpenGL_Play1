#include "SkyTexPass.hpp"
#include "../../Shading/Cubemap.hpp"
#include "../../ShaderGUI.hpp"
SkyTexPass::SkyTexPass(std::string _vs_path, std::string _fs_path, int _cubemapSize)
    : Pass(0, 0, _vs_path, _fs_path),
      cubemapSize(_cubemapSize)
{
    cubemapParam = std::make_shared<CubemapParameters>(0.1f, 1e4f, glm::vec3(0.f));
    initializeGLResources();
    contextSetup();
}

SkyTexPass::~SkyTexPass()
{
    cleanUpGLResources();
}

inline void SkyTexPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);
    skyCubemapTex.generate(cubemapSize, cubemapSize, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_LINEAR, GL_LINEAR, false);
}

void SkyTexPass::cleanUpGLResources()
{
    glDeleteFramebuffers(1, &FBO);
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
    cam.setToShader(shaders);
    /****************************************天空设置*****************************************************/
    SkySetting::SetShaderUniforms(shaders);
    /****************************************方向光源输入**************************************************/
    shaders.setTextureAuto(transmittanceLUT, GL_TEXTURE_2D, 0, "transmittanceLUT");
    allLights.dirLights[0].setToShader(shaders);

    cubemapParam->update(cam.getPosition());
    for (unsigned int i = 0; i < 6; ++i)
    {
        shaders.setMat4("view", cubemapParam->viewMatrices[i]);
        shaders.setUniform3fv("eyePos", cubemapParam->viewPosition);
        shaders.setMat4("projection", cubemapParam->projectionMartix);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               TextureCube::FaceTargets[i], skyCubemapTex.ID, 0);
        Renderer::DrawSphere();
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

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

    SkySetting::SetShaderUniforms(shaders);

    Renderer::DrawQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SkyEnvmapPass::render(unsigned int &skyTexture, std::shared_ptr<CubemapParameters> cubemapParam)
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glViewport(0, 0, cubemapSize, cubemapSize);
    shaders.use();
    if (!shaders.used)
        throw(std::exception("Shader failed to setup."));

    for (unsigned int i = 0; i < 6; ++i)
    {
        shaders.setMat4("projection", cubemapParam->projectionMartix);
        shaders.setMat4("view", cubemapParam->viewMatrices[i]);
        shaders.setTextureAuto(skyTexture, GL_TEXTURE_CUBE_MAP, 0, "skyTexture"); // 输入原cubemap
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               TextureCube::FaceTargets[i], skyEnvmapTex.ID, 0); // 绑定输出目标cubemap
        Renderer::DrawSphere();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}