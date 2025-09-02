#include "LightSource.hpp"
#include "../Shading/Texture.hpp"

LightSource::LightSource(const glm::vec3 &_intensity, const glm::vec3 &_position)
    : combIntensity(_intensity),
      position(_position),
      colorIntensity(SeparateIntensity(_intensity))
{
}

/********************************PointLight******************************************************************** */
PointLight::PointLight(const glm::vec3 &_intensity, const glm::vec3 &_position, int _texResolution)
    : LightSource(_intensity, _position), texResolution(_texResolution)
{
}

void PointLight::setToShader(Shader &shaders)
{
    combIntensity = CombineIntensity(colorIntensity);
    shaders.setUniform3fv("pointLightPos", position);
    shaders.setUniform3fv("pointLightIntensity", combIntensity);

    if (depthCubemap != 0)
    {
        shaders.setTextureAuto(depthCubemap, GL_TEXTURE_CUBE_MAP, 0, "shadowCubeMaps");
    }
}

void PointLight::setToShaderLightArray(Shader &shaders, size_t index)
{
    combIntensity = CombineIntensity(colorIntensity);
    shaders.setUniform3fv("pointLightPos[" + std::to_string(index) + "]", position);
    shaders.setUniform3fv("pointLightIntensity[" + std::to_string(index) + "]", combIntensity);

    // 阴影贴图绑定
    if (depthCubemap != 0)
    {
        shaders.setTextureAuto(depthCubemap, GL_TEXTURE_CUBE_MAP, 0, "shadowCubeMaps[" + std::to_string(index) + "]");
    }
}

void PointLight::generateShadowTexResource()
{
    if (depthCubemap == 0)
    {
        glGenTextures(1, &depthCubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        for (unsigned int i = 0; i < 6; ++i)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                         texResolution, texResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
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

    shaders.setTextureAuto(depthMap, GL_TEXTURE_2D, 0, "dirDepthMap");
    if (useVSM)
    {
        shaders.setTextureAuto(VSMTexture->ID, GL_TEXTURE_2D, 0, "VSMTexture");
        shaders.setUniform("useVSM", useVSM);
    }
    shaders.setUniform3fv("dirLightPos", position);
    shaders.setUniform3fv("dirLightIntensity", combIntensity);
    shaders.setMat4("dirLightSpaceMatrix", lightSpaceMatrix);
}
void DirectionLight::setToShaderLightArray(Shader &shaders, size_t index)
{
    combIntensity = CombineIntensity(colorIntensity);

    shaders.setTextureAuto(depthMap, GL_TEXTURE_2D, 0, std::format("dirLightArray[{}].depthMap", index));
    if (useVSM)
    {
        shaders.setTextureAuto(VSMTexture->ID, GL_TEXTURE_2D, 0, std::format("dirLightArray[{}].VSMTexture", index));
        shaders.setUniform(std::format("dirLightArray[{}].useVSM", index), useVSM);
    }
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
    if (depthMap == 0)
    {
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, texResolution, texResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    if (useVSM && VSMTexture->ID == 0)
    {
        VSMTexture->SetFilterMax(GL_LINEAR);
        VSMTexture->SetFilterMin(GL_LINEAR);
        VSMTexture->SetWrapMode(GL_CLAMP_TO_EDGE);
        VSMTexture->Generate(texResolution, texResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, NULL);
    }
}