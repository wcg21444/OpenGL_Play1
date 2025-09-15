#pragma once

#include "../Shading/Shader.hpp"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../Math/Frustum.hpp"
#include "../Shading/Texture.hpp"

struct DirShadowUnit
{
    int resolution;
    OrthoFrustum frustum;
    // Texture ptr == nullpt 意味着不使用 VSM
    std::shared_ptr<Texture> depthTexture = nullptr;
    std::shared_ptr<Texture> VSMTexture = nullptr;
    std::shared_ptr<Texture> SATTexture = nullptr;

    DirShadowUnit() {}
    DirShadowUnit(int _resolution, const OrthoFrustum &_frustum)
        : resolution(_resolution), frustum(_frustum) {}
    void generateDepthTexture(GLenum internalFormat = GL_DEPTH_COMPONENT32F, GLenum format = GL_DEPTH_COMPONENT)
    {
        if (depthTexture == nullptr)
        {
            depthTexture = std::make_shared<Texture>();
            depthTexture->setFilterMin(GL_NEAREST);
            depthTexture->setFilterMax(GL_NEAREST);
            depthTexture->setWrapMode(GL_CLAMP_TO_EDGE);
            depthTexture->generate(resolution, resolution,
                                   GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr, false);
        }
    }
    void generateVSMTexture(GLenum internalFormat = GL_RGBA32F, GLenum format = GL_RGBA)
    {
        if (VSMTexture == nullptr)
        {
            VSMTexture = std::make_shared<Texture>();
            VSMTexture->setFilterMax(GL_LINEAR);
            VSMTexture->setFilterMin(GL_LINEAR);
            VSMTexture->setWrapMode(GL_CLAMP_TO_EDGE);
            VSMTexture->generate(resolution, resolution, internalFormat, format, GL_FLOAT, NULL);
        }
    }
    void generateSATTexture(GLenum internalFormat = GL_RGBA32F, GLenum format = GL_RGBA)
    {
        if (SATTexture == nullptr)
        {
            SATTexture = std::make_shared<Texture>();
            SATTexture->setFilterMax(GL_LINEAR);
            SATTexture->setFilterMin(GL_LINEAR);
            SATTexture->setWrapMode(GL_CLAMP_TO_BORDER);
            const float borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
            SATTexture->generate(resolution, resolution, internalFormat, format, GL_FLOAT, NULL);
        }
    }
};