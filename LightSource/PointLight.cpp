#include "LightSource.hpp"
#include "../Shading/Texture.hpp"
#include "Cubemap.hpp"

PointLight::PointLight(const glm::vec3 &_intensity, const glm::vec3 &_position, int _texResolution, float _farPlane)
    : LightSource(_intensity, _position), texResolution(_texResolution)
{
    depthCubemap = std::make_shared<TextureCube>();
    VSMCubemap = std::make_shared<TextureCube>();
    cubemapParam = std::make_shared<CubemapParameters>(0.1f, _farPlane, _position);
}

void PointLight::setToShaderLightArray(Shader &shaders, size_t index)
{
    combIntensity = ColorIntensity::Combine(colorIntensity);

    shaders.setUniform3fv(std::format("pointLightArray[{}].pos", index), position);
    shaders.setUniform3fv(std::format("pointLightArray[{}].intensity", index), combIntensity);
    shaders.setUniform(std::format("pointLightArray[{}].farPlane", index), cubemapParam->farPlane);
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
        depthCubemap->setWrapMode(GL_CLAMP_TO_EDGE);
        depthCubemap->generate(texResolution, texResolution, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, GL_NEAREST, GL_NEAREST, false);
    }
    if (VSMCubemap->ID == 0 && useVSM)
    {
        VSMCubemap->setWrapMode(GL_CLAMP_TO_EDGE);
        VSMCubemap->generate(texResolution, texResolution, GL_RGBA32F, GL_RGBA, GL_FLOAT, GL_NEAREST, GL_NEAREST, false);
    }
}

void PointLight::setPosition(glm::vec3 &_position)
{
    position = _position;
    update();
}

glm::vec3 PointLight::getPosition() const
{
    return position;
}
void PointLight::update()
{
    cubemapParam->update(position);

}
float PointLight::getFarPlane() const
{
    return cubemapParam->farPlane;
}
