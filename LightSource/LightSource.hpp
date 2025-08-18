#pragma once
#include "../Shading/Shader.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// TODO 阴影开关功能
// TODO 运行时类型切换
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
inline glm::vec3 CombineIntensity(const ColorIntensity &separatedData)
{
    return separatedData.color * separatedData.intensity;
}
enum class LightType
{
    TypePointLight,
    TypeDirLight,
    TypeSpotLight,
};
class LightSource
{
protected:
    glm::vec3 combIntensity;
    glm::vec3 position;

public:
    ColorIntensity colorIntensity;

public:
    LightSource(const glm::vec3 &_intensity, const glm::vec3 &_position);
    virtual void setToShader(Shader &shaders) = 0;
    virtual void setToShaderLightArray(Shader &shaders, size_t index) = 0;
    virtual void setPosition(glm::vec3 &_position) = 0;
    virtual glm::vec3 getPosition() const = 0;
    virtual ~LightSource() = default;
};

class PointLight : public LightSource
{
private:
public:
    unsigned int depthCubemap = 0;
    int texResolution;

public:
    PointLight(const glm::vec3 &_intensity, const glm::vec3 &_position, int _texResolution = 1024);
    void setToShader(Shader &shaders) override;
    void setToShaderLightArray(Shader &shaders, size_t index) override;
    void setPosition(glm::vec3 &_position) override;
    glm::vec3 getPosition() const override;

    void generateShadowTexResource();
};

class DirectionLight : public LightSource
{
private:
    glm::mat4 lightProjection;
    glm::mat4 lightView;

public:
    unsigned int depthMap = 0;
    int texResolution;

    float nearPlane;
    float farPlane;
    float ortho_scale;

    glm::mat4 lightSpaceMatrix;

    DirectionLight(const glm::vec3 &_intensity = glm::vec3(0.1f), const glm::vec3 &_position = glm::vec3(50.f, 20.f, 60.f), int _texResolution = 2048);

    void setToShader(Shader &shaders) override;
    void setToShaderLightArray(Shader &shaders, size_t index) override;
    void setPosition(glm::vec3 &_position) override;
    glm::vec3 getPosition() const override;

    void generateShadowTexResource();
};
struct Lights
{
    std::vector<PointLight> pointLights;
    std::vector<DirectionLight> dirLights;
};