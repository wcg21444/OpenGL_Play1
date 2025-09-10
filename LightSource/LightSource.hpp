#pragma once
#include "../Shading/Shader.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../Shading/Frustum.hpp"

// TODO 阴影开关功能
// TODO 运行时类型切换
class Texture; // fwd declaration
class TextureCube;
class CubemapParameters;

struct ColorIntensity
{
    glm::vec3 color;
    float intensity;

    inline static ColorIntensity Separate(glm::vec3 combined_intensity)
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
    inline static glm::vec3 Combine(const ColorIntensity &separatedData)
    {
        return separatedData.color * separatedData.intensity;
    }
};

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
    static const int MAX_POINT_LIGHTS = 10;
    static const int MAX_DIR_LIGHTS = 5;

    static void InitialzeShaderLightArray(Shader &shaders);

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

public:
    int texResolution;
    std::shared_ptr<CubemapParameters> cubemapParam;
    std::shared_ptr<TextureCube> depthCubemap;
    std::shared_ptr<TextureCube> VSMCubemap;
    bool useVSM = false;

public:
    PointLight(const glm::vec3 &_intensity, const glm::vec3 &_position, int _texResolution, float _farPlane);
    void setToShader(Shader &shaders) override;
    void setToShaderLightArray(Shader &shaders, size_t index) override;
    void setPosition(glm::vec3 &_position) override;
    glm::vec3 getPosition() const override;
    float getFarPlane() const;

    void generateShadowTexResource();
};

class DirectionLight : public LightSource
{
private:
    glm::mat4 lightProjection;
    glm::mat4 lightView;

public:
    int texResolution;
    std::shared_ptr<Texture> depthTexture;
    std::shared_ptr<Texture> VSMTexture;
    std::shared_ptr<Texture> SATTexture;
    bool useVSM = false;

    float nearPlane;
    float farPlane;
    float orthoScale;

    glm::mat4 lightSpaceMatrix;

    DirectionLight(const glm::vec3 &_intensity = glm::vec3(0.1f), const glm::vec3 &_position = glm::vec3(50.f, 20.f, 60.f), int _texResolution = 2048);

    void setToShader(Shader &shaders) override;
    void setToShaderLightArray(Shader &shaders, size_t index) override;
    void setPosition(glm::vec3 &_position) override;
    glm::vec3 getPosition() const override;
    glm::mat4 getlightProjection() const
    {
        return lightProjection;
    }
    glm::mat4 getlightView() const
    {
        return lightView;
    }

    void generateShadowTexResource();
};

struct Lights
{
    std::vector<PointLight> pointLights;
    std::vector<DirectionLight> dirLights;
};