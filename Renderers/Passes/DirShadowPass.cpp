#include "DirShadowPass.hpp"

#include "../../GUI.hpp"
#include "../../Objects/FrustumWireframe.hpp"
#include "../DebugObjectRenderer.hpp"
DirShadowPass::DirShadowPass(std::string _vs_path, std::string _fs_path)
    : Pass(0, 0, _vs_path, _fs_path)
{
    initializeGLResources();
    contextSetup();
}
inline void DirShadowPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);
}
void DirShadowPass::cleanUpGLResources()
{
    glDeleteFramebuffers(1, &FBO);
}
inline void DirShadowPass::contextSetup()
{
}
void DirShadowPass::resize(int _width, int _height)
{
    // void 此处应该是缩放阴影贴图大小
}
/// @brief 将输入的深度图attach到FBO
/// @param _depthMap 通道输出纹理对象
void DirShadowPass::attachDepthMap(const unsigned int _depthMap)
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/// @brief 输入存在的Tex对象,绑定Tex对象到FBO,结果输出到Tex.
void DirShadowPass::renderToTexture(
    const DirectionLight &light,
    Scene &scene,
    glm::mat4 &model,
    int width,
    int height)
{
    attachDepthMap(light.depthTexture->ID);

    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaders.use();
    shaders.setMat4("lightSpaceMatrix", light.lightSpaceMatrix);

    glViewport(0, 0, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    Renderer::DrawScene(scene, model, shaders);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (GUI::DebugToggleDrawFrustum())
    {
        DebugObjectRenderer::AddDrawCall([&](Shader &debugObjectShaders)
                                         { DebugObjectRenderer::DrawFrustum(
                                               OrthoFrustum(light.getlightView(), light.getlightProjection()),
                                               shaders); });
    }
}

void DirShadowPass::render(DirShadowUnit &shadowUnit, Scene &scene, glm::mat4 &model)
{
    attachDepthMap(shadowUnit.depthTexture->ID);

    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shaders.use();
    shaders.setMat4("lightSpaceMatrix", shadowUnit.frustum.getProjViewMatrix());

    glViewport(0, 0, shadowUnit.resolution, shadowUnit.resolution);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    Renderer::DrawScene(scene, model, shaders);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (GUI::DebugToggleDrawFrustum())
    {
        DebugObjectRenderer::AddDrawCall([&](Shader &debugObjectShaders)
                                         { DebugObjectRenderer::DrawFrustum(shadowUnit.frustum, debugObjectShaders); });
    }
}

DirShadowVSMPass::DirShadowVSMPass(std::string _vs_path, std::string _fs_path)
    : Pass(0, 0, _vs_path, _fs_path)
{
    initializeGLResources();
    contextSetup();
}

inline void DirShadowVSMPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBO);
}
void DirShadowVSMPass::cleanUpGLResources()
{
    glDeleteFramebuffers(1, &FBO);
}
inline void DirShadowVSMPass::contextSetup()
{
}
void DirShadowVSMPass::resize(int _width, int _height)
{
}
/// @brief 输入存在的Tex对象,绑定Tex对象到FBO,结果输出到Tex.
void DirShadowVSMPass::renderToVSMTexture(const DirectionLight &light, int width, int height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, light.VSMTexture->ID, 0);

    glViewport(0, 0, width, height);

    shaders.use();
    shaders.setTextureAuto(light.depthTexture->ID, GL_TEXTURE_2D, 0, "depthMap");
    shaders.setUniform("kernelSize", GUI::DebugVSMKernelSize());

    Renderer::DrawQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DirShadowVSMPass::renderToVSMTexture(DirShadowUnit &shadowUnit)
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowUnit.VSMTexture->ID, 0);

    glViewport(0, 0, shadowUnit.resolution, shadowUnit.resolution);

    shaders.use();
    shaders.setTextureAuto(shadowUnit.depthTexture->ID, GL_TEXTURE_2D, 0, "depthMap");
    shaders.setUniform("kernelSize", GUI::DebugVSMKernelSize());

    Renderer::DrawQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

DirShadowSATPass::DirShadowSATPass(std::string _vs_path, std::string _fs_path)
    : Pass(0, 0, _vs_path, _fs_path)
{
    initializeGLResources();
    contextSetup();
}

void DirShadowSATPass::initializeGLResources()
{
    glGenFramebuffers(1, &FBOCol);
    glGenFramebuffers(1, &FBORow);

    SATRowTexture.setFilterMax(GL_LINEAR);
    SATRowTexture.setFilterMin(GL_LINEAR);
    SATRowTexture.setWrapMode(GL_CLAMP_TO_EDGE);
    SATRowTexture.generate(0, 0, GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL, false);

    SATCompInputTex.generateComputeStorage(0, 0, GL_RGBA32F);
    SATCompOutputTex.generateComputeStorage(0, 0, GL_RGBA32F);
}

void DirShadowSATPass::cleanUpGLResources()
{
    glDeleteFramebuffers(1, &FBOCol);
    glDeleteFramebuffers(1, &FBORow);
}

void DirShadowSATPass::reloadCurrentShaders()
{
    Pass::reloadCurrentShaders();
    SATComputeShader = ComputeShader("Shaders/ShadowMapping/SATCompute.comp");
    momentComputeShader = ComputeShader("Shaders/ShadowMapping/MomentsCompute.comp");
}

void DirShadowSATPass::contextSetup()
{
}

void DirShadowSATPass::resize(int _width, int _height)
{
}

void DirShadowSATPass::renderToSATTexture(const DirectionLight &light, int width, int height)
{
    if (GUI::DebugToggleUseComputeShaderAccelerate())
    {
        computeSAT(light.depthTexture->ID, width, height);
        glCopyImageSubData(
            SATCompOutputTex.ID, GL_TEXTURE_2D, 0, 0, 0, 0,  // 源纹理信息 (ID, 目标, mipmap级, x, y, z)
            light.SATTexture->ID, GL_TEXTURE_2D, 0, 0, 0, 0, // 目标纹理信息 (ID, 目标, mipmap级, x, y, z)
            width, height, 1                                 // 复制区域的尺寸 (宽, 高, 深)
        );
    }
    else
    {
        SATRowTexture.resize(width, height);

        glBindFramebuffer(GL_FRAMEBUFFER, FBORow);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SATRowTexture.ID, 0); // 输出目标绑定
        glViewport(0, 0, width, height);

        shaders.use();
        shaders.setUniform("RowSummary", 1);
        shaders.setTextureAuto(light.depthTexture->ID, GL_TEXTURE_2D, 0, "InputTexture");
        Renderer::DrawQuad();

        glBindFramebuffer(GL_FRAMEBUFFER, FBOCol);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, light.SATTexture->ID, 0); // 输出目标绑定

        glViewport(0, 0, width, height);

        shaders.use();
        shaders.setUniform("RowSummary", 0);
        shaders.setTextureAuto(SATRowTexture.ID, GL_TEXTURE_2D, 0, "InputTexture");

        Renderer::DrawQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void DirShadowSATPass::renderToSATTexture(DirShadowUnit &shadowUnit)
{
    if (GUI::DebugToggleUseComputeShaderAccelerate())
    {
        computeSAT(shadowUnit.depthTexture->ID, shadowUnit.resolution, shadowUnit.resolution);
        glCopyImageSubData(
            SATCompOutputTex.ID, GL_TEXTURE_2D, 0, 0, 0, 0,       // 源纹理信息 (ID, 目标, mipmap级, x, y, z)
            shadowUnit.SATTexture->ID, GL_TEXTURE_2D, 0, 0, 0, 0, // 目标纹理信息 (ID, 目标, mipmap级, x, y, z)
            shadowUnit.resolution, shadowUnit.resolution, 1       // 复制区域的尺寸 (宽, 高, 深)
        );
    }
    else
    {
        SATRowTexture.resize(shadowUnit.resolution, shadowUnit.resolution);

        glBindFramebuffer(GL_FRAMEBUFFER, FBORow);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SATRowTexture.ID, 0); // 输出目标绑定
        glViewport(0, 0, shadowUnit.resolution, shadowUnit.resolution);

        shaders.use();
        shaders.setUniform("RowSummary", 1);
        shaders.setTextureAuto(shadowUnit.depthTexture->ID, GL_TEXTURE_2D, 0, "InputTexture");
        Renderer::DrawQuad();

        glBindFramebuffer(GL_FRAMEBUFFER, FBOCol);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadowUnit.SATTexture->ID, 0); // 输出目标绑定

        glViewport(0, 0, shadowUnit.resolution, shadowUnit.resolution);

        shaders.use();
        shaders.setUniform("RowSummary", 0);
        shaders.setTextureAuto(SATRowTexture.ID, GL_TEXTURE_2D, 0, "InputTexture");

        Renderer::DrawQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void DirShadowSATPass::computeSAT(const unsigned int depthTextureID, int width, int height)
{
    const int g = ComputeShader::GetGroupSize(width);

    const float borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};

    momentsTex.setWrapMode(GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_2D, momentsTex.ID);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    momentsTex.generateComputeStorage(width, height, GL_RGBA32F);

    SATCompInputTex.setWrapMode(GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_2D, SATCompInputTex.ID);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    SATCompInputTex.generateComputeStorage(width, height, GL_RGBA32F);

    SATCompOutputTex.setWrapMode(GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_2D, SATCompOutputTex.ID);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    SATCompOutputTex.generateComputeStorage(width, height, GL_RGBA32F);
    // 用depth计算Moments
    momentComputeShader.use();
    glBindImageTexture(0, momentsTex.ID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTextureID);
    momentComputeShader.setUniform("depthMap", 0); // 手动设置纹理
    if (GUI::useBias == true)
    {
        momentComputeShader.setInt("useBias", 1);
    }
    else
    {
        momentComputeShader.setInt("useBias", 0);
    }

    glDispatchCompute(g, g, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    SATComputeShader.use();
    SATComputeShader.setUniform("Size", width);

    glBindImageTexture(0, momentsTex.ID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, SATCompInputTex.ID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(width, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glBindImageTexture(0, SATCompInputTex.ID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, SATCompOutputTex.ID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(width, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    GUI::RenderTextureInspector({momentsTex.ID, SATCompInputTex.ID, SATCompOutputTex.ID});
}
