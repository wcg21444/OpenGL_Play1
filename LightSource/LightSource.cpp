#include "LightSource.hpp"

/********************************PointLight******************************************************************** */
PointLight::PointLight(const glm::vec3 &_intensity, const glm::vec3 &_position, int _texResolution)
    : intensity(_intensity), position(_position), texResolution(_texResolution)
{
}

void PointLight::setToShader(Shader &shaders)
{
    shaders.setUniform3fv("lightIntensity", intensity);
    shaders.setUniform3fv("lightPos", position);
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
/********************************DirectionLight******************************************************************** */
DirectionLight::DirectionLight(const glm::vec3 &_intensity, const glm::vec3 &_position, int _texResolution)
    : intensity(_intensity), position(_position), ortho_scale(50.f), nearPlane(0.1f), farPlane(10000.f), texResolution(_texResolution)
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
    shaders.setUniform3fv("lightPos", position);
}

void DirectionLight::updatePosition(glm::vec3 &_position)
{
    position = _position;
    lightView = glm::lookAt(position,
                            glm::vec3(0.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;
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