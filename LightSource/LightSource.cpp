#include "LightSource.hpp"

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
DirectionLight::DirectionLight(const glm::vec3 &_intensity, const glm::vec3 &_position, int _texResolution)
    : LightSource(_intensity, _position), ortho_scale(100.f), nearPlane(0.1f), farPlane(10000.f), texResolution(_texResolution)
{
    lightProjection = glm::ortho(-1.0f * ortho_scale,
                                 1.0f * ortho_scale,
                                 -1.0f * ortho_scale,
                                 1.0f * ortho_scale,
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
    shaders.setUniform3fv("dirLightPos", position);
    shaders.setUniform3fv("dirLightIntensity", combIntensity);
    shaders.setMat4("dirLightSpaceMatrix", lightSpaceMatrix);
}
void DirectionLight::setToShaderLightArray(Shader &shaders, size_t index)
{
    combIntensity = CombineIntensity(colorIntensity);

    shaders.setTextureAuto(depthMap, GL_TEXTURE_2D, 0, std::format("dirDepthMap[{}]", index));
    shaders.setUniform3fv(std::format("dirLightPos[{}]", index), position);
    shaders.setUniform3fv(std::format("dirLightIntensity[{}]", index), combIntensity);
    shaders.setMat4(std::format("dirLightSpaceMatrix[{}]", index), lightSpaceMatrix);
}
void DirectionLight::setPosition(glm::vec3 &_position)
{
    position = _position;
    lightView = glm::lookAt(position,
                            glm::vec3(0.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;
}

glm::vec3 DirectionLight::getPosition() const
{
    return position;
}

// 在渲染器中调用,等待GL上下文
void DirectionLight::generateShadowTexResource()
{
    if (depthMap == 0)
    {
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, texResolution, texResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}