#include "LightSource.hpp"
#include "../Shading/Texture.hpp"

LightSource::LightSource(const glm::vec3 &_intensity, const glm::vec3 &_position)
    : combIntensity(_intensity),
      position(_position),
      colorIntensity(SeparateIntensity(_intensity))
{
}

/********************************PointLight******************************************************************** */
PointLight::PointLight(const glm::vec3 &_intensity, const glm::vec3 &_position, int _texResolution = 1024, float _farPlane = 250.f)
    : LightSource(_intensity, _position), texResolution(_texResolution), farPlane(_farPlane)
{
    depthCubemap = std::make_shared<TextureCube>();
    VSMCubemap = std::make_shared<TextureCube>();

    aspect = 1.f;
    nearPlane = 0.1f;
    shadowProj = glm::perspective(glm::radians(90.0f), aspect, nearPlane, farPlane);
}

void PointLight::setToShader(Shader &shaders)
{
    combIntensity = CombineIntensity(colorIntensity);
    shaders.setUniform3fv("pointLightPos", position);
    shaders.setUniform3fv("pointLightIntensity", combIntensity);
    shaders.setUniform("pointLightFarPlane", farPlane);
    shaders.setUniform("pointLightUseVSM", useVSM);

    if (depthCubemap->ID != 0)
    {
        shaders.setTextureAuto(depthCubemap->ID, GL_TEXTURE_CUBE_MAP, 0, "shadowCubeMaps");
    }

    if (VSMCubemap->ID != 0 && useVSM)
    {
        shaders.setTextureAuto(VSMCubemap->ID, GL_TEXTURE_CUBE_MAP, 0, "VSMCubemap");
    }
}

void PointLight::setToShaderLightArray(Shader &shaders, size_t index)
{
    combIntensity = CombineIntensity(colorIntensity);

    shaders.setUniform3fv(std::format("pointLightArray[{}].pos", index), position);
    shaders.setUniform3fv(std::format("pointLightArray[{}].intensity", index), combIntensity);
    shaders.setUniform(std::format("pointLightArray[{}].farPlane", index), farPlane);
    shaders.setUniform(std::format("pointLightArray[{}].useVSM", index), useVSM);

    shaders.setTextureAuto(depthCubemap->ID, GL_TEXTURE_CUBE_MAP, 0, std::format("pointLightArray[{}].depthCubemap", index));
    if (useVSM)
    {
        shaders.setTextureAuto(VSMCubemap->ID, GL_TEXTURE_CUBE_MAP, 0, std::format("pointLightArray[{}].VSMCubemap", index));
    }
}

void PointLight::generateShadowTexResource()
{
    if (depthCubemap->ID == 0)
    {
        depthCubemap->SetWrapMode(GL_CLAMP_TO_EDGE);
        depthCubemap->Generate(texResolution, texResolution, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, GL_NEAREST, GL_NEAREST, false);
    }
    if (VSMCubemap->ID == 0 && useVSM)
    {
        VSMCubemap->SetWrapMode(GL_CLAMP_TO_EDGE);
        VSMCubemap->Generate(texResolution, texResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST, false);
    }
}

void PointLight::setPosition(glm::vec3 &_position)
{
    position = _position;
}

glm::vec3 PointLight::getPosition() const
{
    return position;
}
/********************************DirectionLight******************************************************************** */
// TODO: 根据Camera位置优化ProjectionMatrix
DirectionLight::DirectionLight(const glm::vec3 &_intensity, const glm::vec3 &_position, int _texResolution)
    : LightSource(_intensity, _position), orthoScale(100.f), nearPlane(0.1f), farPlane(10000.f), texResolution(_texResolution)
{
    VSMTexture = std::make_shared<Texture>();
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
    combIntensity = CombineIntensity(colorIntensity);

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
    combIntensity = CombineIntensity(colorIntensity);

    shaders.setTextureAuto(depthTexture->ID, GL_TEXTURE_2D, 0, std::format("dirLightArray[{}].depthMap", index));
    if (useVSM)
    {
        shaders.setTextureAuto(VSMTexture->ID, GL_TEXTURE_2D, 0, std::format("dirLightArray[{}].VSMTexture", index));
    }
    else
    {
        shaders.setTextureAuto(0, GL_TEXTURE_2D, 0, std::format("dirLightArray[{}].VSMTexture", index));
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

// 在渲染器中逐帧调用,等待GL上下文
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
    }
}