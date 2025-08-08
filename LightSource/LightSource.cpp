#include "LightSource.hpp"

LightSource::LightSource(const glm::vec3 &_intensity, const glm::vec3 &_position)
    : intensity(_intensity), position(_position)
{
}

void LightSource::setToShader(Shader &shaders)
{
    shaders.setUniform3fv("lightIntensity", intensity);
    shaders.setUniform3fv("lightPos", position);
}

ParallelLight::ParallelLight(const glm::vec3 &_intensity, const glm::vec3 &_position, int _depthMapResolution)
    : intensity(_intensity), position(_position), ortho_scale(50.f), nearPlane(0.1f), farPlane(10000.f), depthMapResolution(_depthMapResolution)
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

void ParallelLight::setToShader(Shader &shaders)
{
    shaders.setUniform3fv("lightPos", position);
}

void ParallelLight::updatePosition(glm::vec3 &_position)
{
    position = _position;
    lightView = glm::lookAt(position,
                            glm::vec3(0.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;
}

// 在渲染器中调用,等待GL上下文
void ParallelLight::generateDepthMapResource()
{
    if (depthMap == 0)
    {
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, depthMapResolution, depthMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}