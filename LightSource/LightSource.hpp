#pragma once
#include "../Shading/Shader.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// TODO 阴影开关功能
// class LightSource
// {
// public:
//     enum LightType
//     {
//         point;
//         direction;
//         spot;
//     }
//     glm::vec3 intensity;
//     glm::vec3 position;
//     bool enableShadow;
//     virtual void generateShadowTexResource();
// };
namespace LightSource
{

    // 光源亮度颜色混合 简陋方法  这样的分离依然做不到颜色 亮度线性无关. 强度置零 转换丢失颜色.
    // TODO: 优化逻辑
    struct ColorIntensity
    {
        glm::vec3 color;
        float intensity;
    };
    inline ColorIntensity SeparateIntensity(glm::vec3 combined_intensity)
    {
        ColorIntensity result;

        float max_intensity = glm::max(combined_intensity.x, glm::max(combined_intensity.y, combined_intensity.z));

        if (max_intensity > 0.0f)
        {
            result.color = combined_intensity / max_intensity;
            result.intensity = max_intensity;
        }
        else
        {
            result.color = glm::vec3(0.0f);
            result.intensity = 0.1f; // 钳制数据
        }
        return result;
    }
    inline glm::vec3 CombineIntensity(const ColorIntensity &combined_data)
    {
        return combined_data.color * combined_data.intensity;
    }
}

// Only PointLight is supported
class PointLight
{
public:
    glm::vec3 combIntensity;
    glm::vec3 position;

    unsigned int depthCubemap = 0;
    int texResolution;

public:
    PointLight(const glm::vec3 &_intensity, const glm::vec3 &_position, int _texResolution = 1024);
    void setToShader(Shader &shaders);
    void generateShadowTexResource();
};

class DirectionLight
{
private:
    glm::mat4 lightProjection;
    glm::mat4 lightView;

public:
    glm::vec3 combIntensity;
    glm::vec3 position;

    unsigned int depthMap = 0;
    int texResolution;

    float nearPlane;
    float farPlane;
    float ortho_scale;

    glm::mat4 lightSpaceMatrix;

    DirectionLight(const glm::vec3 &_intensity = glm::vec3(0.1f), const glm::vec3 &_position = glm::vec3(50.f, 20.f, 60.f), int _texResolution = 2048);

    void setToShader(Shader &shaders);
    void updatePosition(glm::vec3 &_position);
    void generateShadowTexResource();
};
struct Lights
{
    std::vector<PointLight> pointLights;
    std::vector<DirectionLight> dirLights;
};