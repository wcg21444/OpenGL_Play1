#include "LightSource.hpp"
#include "../Shading/Texture.hpp"
#include "Cubemap.hpp"
#include <glm/gtc/matrix_transform.hpp>

DirectionLight::DirectionLight(const glm::vec3 &_intensity, const glm::vec3 &_position, int _texResolution)
    : LightSource(_intensity, _position), orthoScale(100.f), nearPlane(0.1f), farPlane(10000.f), texResolution(_texResolution)
{
    VSMTexture = std::make_shared<Texture>();
    SATTexture = std::make_shared<Texture>();
    depthTexture = std::make_shared<Texture>();
    lightProjection = glm::ortho(-1.0f * orthoScale,
                                 1.0f * orthoScale,
                                 -1.0f * orthoScale,
                                 1.0f * orthoScale,
                                 nearPlane, farPlane);
    lightView = glm::lookAt(position,
                            glm::vec3(0.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;
}

void DirectionLight::setToShader(Shader &shaders)
{
    combIntensity = ColorIntensity::Combine(colorIntensity);

    shaders.setTextureAuto(depthTexture->ID, GL_TEXTURE_2D, 0, "dirDepthMap");
    if (useVSM)
    {
        shaders.setTextureAuto(VSMTexture->ID, GL_TEXTURE_2D, 0, "dirVSMTexture");
        shaders.setUniform("useVSM", useVSM);
    }
    shaders.setUniform3fv("dirLightPos", position);
    shaders.setUniform3fv("dirLightIntensity", combIntensity);
    shaders.setMat4("dirLightSpaceMatrix", lightSpaceMatrix);
}
void DirectionLight::setToShaderLightArray(Shader &shaders, size_t index)
{
    combIntensity = ColorIntensity::Combine(colorIntensity);

    shaders.setTextureAuto(depthTexture->ID, GL_TEXTURE_2D, 0, std::format("dirLightArray[{}].depthMap", index));
    if (useVSM)
    {
        shaders.setTextureAuto(VSMTexture->ID, GL_TEXTURE_2D, 0, std::format("dirLightArray[{}].VSMTexture", index));
        shaders.setTextureAuto(SATTexture->ID, GL_TEXTURE_2D, 0, std::format("dirLightArray[{}].SATTexture", index));
    }
    else
    {
        shaders.setTextureAuto(0, GL_TEXTURE_2D, 0, std::format("dirLightArray[{}].VSMTexture", index));
        shaders.setTextureAuto(0, GL_TEXTURE_2D, 0, std::format("dirLightArray[{}].SATTexture", index));
    }
    shaders.setUniform(std::format("dirLightArray[{}].useVSM", index), useVSM);
    shaders.setUniform3fv(std::format("dirLightArray[{}].pos", index), position);
    shaders.setUniform3fv(std::format("dirLightArray[{}].intensity", index), combIntensity);
    shaders.setMat4(std::format("dirLightArray[{}].spaceMatrix", index), lightSpaceMatrix);
    shaders.setUniform(std::format("dirLightArray[{}].farPlane", index), farPlane);
    shaders.setUniform(std::format("dirLightArray[{}].orthoScale", index), orthoScale);
}
void DirectionLight::setPosition(glm::vec3 &_position)
{
    position = _position;
    lightProjection = glm::ortho(-1.0f * orthoScale,
                                 1.0f * orthoScale,
                                 -1.0f * orthoScale,
                                 1.0f * orthoScale,
                                 nearPlane, farPlane);
    lightView = glm::lookAt(position,
                            glm::vec3(0.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;
}

glm::vec3 DirectionLight::getPosition() const
{
    return position;
}

void DirectionLight::generateShadowTexResource()
{
    if (depthTexture->ID == 0)
    {
        depthTexture->SetFilterMax(GL_NEAREST);
        depthTexture->SetFilterMin(GL_NEAREST);
        depthTexture->SetWrapMode(GL_CLAMP_TO_EDGE);
        depthTexture->Generate(texResolution, texResolution, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, NULL, false);
    }
    if (useVSM && VSMTexture->ID == 0)
    {
        VSMTexture->SetFilterMax(GL_LINEAR);
        VSMTexture->SetFilterMin(GL_LINEAR);
        VSMTexture->SetWrapMode(GL_CLAMP_TO_EDGE);
        VSMTexture->Generate(texResolution, texResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL);

        SATTexture->SetFilterMax(GL_LINEAR);
        SATTexture->SetFilterMin(GL_LINEAR);
        SATTexture->SetWrapMode(GL_CLAMP_TO_BORDER);
        const float borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        SATTexture->Generate(texResolution, texResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL);
    }
}
