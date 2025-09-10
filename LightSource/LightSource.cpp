#include "LightSource.hpp"
#include "../Shading/Texture.hpp"
#include "Cubemap.hpp"
#include "../../GUI.hpp"

void LightSource::InitialzeShaderLightArray(Shader &shaders)
{
    for (size_t i = 0; i < MAX_POINT_LIGHTS; ++i)
    {
        shaders.setUniform3fv(std::format("pointLightArray[{}].pos", i), glm::vec3(0.0f));
        shaders.setUniform3fv(std::format("pointLightArray[{}].intensity", i), glm::vec3(0.0f));
        shaders.setUniform(std::format("pointLightArray[{}].farPlane", i), 0.f);
        shaders.setUniform(std::format("pointLightArray[{}].useVSM", i), 0);

        shaders.setTextureAuto(0, GL_TEXTURE_CUBE_MAP, 0, std::format("pointLightArray[{}].depthCubemap", i));
        shaders.setTextureAuto(0, GL_TEXTURE_CUBE_MAP, 0, std::format("pointLightArray[{}].VSMCubemap", i));
    }
}

LightSource::LightSource(const glm::vec3 &_intensity, const glm::vec3 &_position)
    : combIntensity(_intensity),
      position(_position),
      colorIntensity(ColorIntensity::Separate(_intensity))
{
}
