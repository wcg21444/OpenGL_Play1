#include "LightSource.hpp"
#include "../Shading/Texture.hpp"
#include "Cubemap.hpp"
#include <glm/gtc/matrix_transform.hpp>

DirectionLight::DirectionLight(const glm::vec3 &_intensity, const glm::vec3 &_position, int _texResolution)
    : LightSource(_intensity, _position), orthoScale(100.f), nearPlane(0.1f), farPlane(10000.f), texResolution(_texResolution)
{
    VSMTexture = std::make_shared<Texture2D>();
    SATTexture = std::make_shared<Texture2D>();
    depthTexture = std::make_shared<Texture2D>();

    lightProjection = glm::ortho(-1.0f * orthoScale,
                                 1.0f * orthoScale,
                                 -1.0f * orthoScale,
                                 1.0f * orthoScale,
                                 nearPlane, farPlane);
    lightView = glm::lookAt(position,
                            glm::vec3(0.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, 1.0f, 0.0f));
    shadowUnit = DirShadowUnit(
        _texResolution,
        OrthoFrustum(lightView, lightProjection));

    lightSpaceMatrix = lightProjection * lightView;
}

void DirectionLight::setSunlightToShader(Shader &shaders)
{
    //TODO 这一部分逻辑冗余.  单光源和多光源设置
    // 为skyTex保留
    shaders.setUniform3fv("dirLightPos", position);
    shaders.setUniform3fv("dirLightIntensity", combIntensity);
}
void DirectionLight::setToShaderLightArray(Shader &shaders, size_t index)
{
    combIntensity = ColorIntensity::Combine(colorIntensity);

    // shaders.setTextureAuto(depthTexture->ID, GL_TEXTURE_2D, 0, std::format("dirLightArray[{}].depthMap", index));
    shaders.setTextureAuto(shadowUnit.depthTexture->ID, GL_TEXTURE_2D, 0, std::format("dirLightArray[{}].depthMap", index));
    if (useVSM)
    {
        // shaders.setTextureAuto(VSMTexture->ID, GL_TEXTURE_2D, 0, std::format("dirLightArray[{}].VSMTexture", index));
        // shaders.setTextureAuto(SATTexture->ID, GL_TEXTURE_2D, 0, std::format("dirLightArray[{}].SATTexture", index));
        shaders.setTextureAuto(shadowUnit.VSMTexture->ID, GL_TEXTURE_2D, 0, std::format("dirLightArray[{}].VSMTexture", index));
        shaders.setTextureAuto(shadowUnit.SATTexture->ID, GL_TEXTURE_2D, 0, std::format("dirLightArray[{}].SATTexture", index));
    }
    else
    {
        shaders.setTextureAuto(0, GL_TEXTURE_2D, 0, std::format("dirLightArray[{}].VSMTexture", index));
        shaders.setTextureAuto(0, GL_TEXTURE_2D, 0, std::format("dirLightArray[{}].SATTexture", index));
    }
    shaders.setUniform(std::format("dirLightArray[{}].useVSM", index), useVSM);
    shaders.setUniform3fv(std::format("dirLightArray[{}].pos", index), shadowUnit.frustum.getPosition());
    shaders.setUniform3fv(std::format("dirLightArray[{}].intensity", index), combIntensity);
    shaders.setMat4(std::format("dirLightArray[{}].spaceMatrix", index), shadowUnit.frustum.getProjViewMatrix());
    shaders.setUniform(std::format("dirLightArray[{}].farPlane", index), shadowUnit.frustum.getFarPlane());
    shaders.setUniform(std::format("dirLightArray[{}].orthoScale", index), orthoScale);
}
void DirectionLight::setPosition(glm::vec3 &_position)
{
    position = _position;
    update();
}

glm::vec3 DirectionLight::getPosition() const
{
    return position;
}

void DirectionLight::update()
{
    lightProjection = glm::ortho(-1.0f * orthoScale,
                                 1.0f * orthoScale,
                                 -1.0f * orthoScale,
                                 1.0f * orthoScale,
                                 nearPlane, farPlane);
    lightView = glm::lookAt(position,
                            glm::vec3(0.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;
    shadowUnit.update(lightView, lightProjection);
}

void DirectionLight::generateShadowTexResource()
{
    if (depthTexture->ID == 0)
    {
        depthTexture->setFilterMax(GL_NEAREST);
        depthTexture->setFilterMin(GL_NEAREST);
        depthTexture->setWrapMode(GL_CLAMP_TO_EDGE);
        depthTexture->generate(texResolution, texResolution, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, NULL, false);
    }
    shadowUnit.generateDepthTexture(GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT);
    if (useVSM && VSMTexture->ID == 0)
    {
        VSMTexture->setFilterMax(GL_LINEAR);
        VSMTexture->setFilterMin(GL_LINEAR);
        VSMTexture->setWrapMode(GL_CLAMP_TO_EDGE);
        VSMTexture->generate(texResolution, texResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL);

        SATTexture->setFilterMax(GL_LINEAR);
        SATTexture->setFilterMin(GL_LINEAR);
        SATTexture->setWrapMode(GL_CLAMP_TO_BORDER);
        const float borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        SATTexture->generate(texResolution, texResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL);
    }
    if (useVSM)
    {
        shadowUnit.generateVSMTexture(GL_RGBA32F, GL_RGBA);
        shadowUnit.generateSATTexture(GL_RGBA32F, GL_RGBA);
    }
}
