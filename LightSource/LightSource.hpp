#pragma once
#include "../Shader.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Only PointLight is supported
class LightSource
{
public:
    glm::vec3 intensity;
    glm::vec3 position;

    unsigned int depthCubemap = 0;

public:
    LightSource(const glm::vec3 &_intensity, const glm::vec3 &_position);
    void setToShader(Shader &shaders);
};

class ParallelLight
{
private:
    glm::mat4 lightProjection;
    glm::mat4 lightView;

public:
    glm::vec3 intensity;
    glm::vec3 position;
    float ortho_scale;
    unsigned int depthMap = 0;
    float nearPlane;
    float farPlane;

    int depthMapResolution;

    glm::mat4 lightSpaceMatrix;

    ParallelLight(const glm::vec3 &_intensity = glm::vec3(0.1f), const glm::vec3 &_position = glm::vec3(50.f, 20.f, 60.f), int _depthMapResolution = 2048);

    void setToShader(Shader &shaders);
    void updatePosition(glm::vec3 &_position);
    void generateDepthMapResource();
};

using Lights = std::vector<LightSource>;