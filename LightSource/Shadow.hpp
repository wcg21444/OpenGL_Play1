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
    std::shared_ptr<Texture2D> depthTexture = nullptr;
    std::shared_ptr<Texture2D> VSMTexture = nullptr;
    std::shared_ptr<Texture2D> SATTexture = nullptr;

    DirShadowUnit() {}
    DirShadowUnit(int _resolution, const OrthoFrustum &_frustum)
        : resolution(_resolution), frustum(_frustum) {}
    void generateDepthTexture(GLenum internalFormat = GL_DEPTH_COMPONENT32F, GLenum format = GL_DEPTH_COMPONENT)
    {
        if (depthTexture == nullptr)
        {
            depthTexture = std::make_shared<Texture2D>();
            depthTexture->setFilterMin(GL_NEAREST);
            depthTexture->setFilterMax(GL_NEAREST);
            depthTexture->setWrapMode(GL_CLAMP_TO_EDGE);
            depthTexture->generate(resolution, resolution,
                                   internalFormat, format, GL_FLOAT, NULL, false);
        }
    }
    void generateVSMTexture(GLenum internalFormat = GL_RGBA32F, GLenum format = GL_RGBA)
    {
        if (VSMTexture == nullptr)
        {
            VSMTexture = std::make_shared<Texture2D>();
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
            SATTexture = std::make_shared<Texture2D>();
            SATTexture->setFilterMax(GL_LINEAR);
            SATTexture->setFilterMin(GL_LINEAR);
            SATTexture->setWrapMode(GL_CLAMP_TO_BORDER);
            const float borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
            SATTexture->generate(resolution, resolution, internalFormat, format, GL_FLOAT, NULL);
        }
    }
    void update(glm::mat4 &view, glm::mat4 &projeciton)
    {
        frustum = OrthoFrustum(view, projeciton);
    }
};

// struct CSMShadowMultiUnit
// {
//     std::vector<DirShadowUnit> CSMUnits;
//     int depth;

//     CSMShadowMultiUnit() {}

// };

struct CSMShadowUnit
{
    std::shared_ptr<Texture2DArray> CSMTextureArray = nullptr;
    std::shared_ptr<Texture2DArray> VSMTextureArray = nullptr;
    std::shared_ptr<Texture2DArray> SATTextureArray = nullptr;
    std::vector<OrthoFrustum> CSMfrustums;
    int resolution;
    int depth;

    CSMShadowUnit() {}
    CSMShadowUnit(int _resolution, int _depth, const std::vector<OrthoFrustum> &_frustums)
        : resolution(_resolution), depth(_depth), CSMfrustums(_frustums) {}
    CSMShadowUnit(int _resolution, int _depth, std::vector<OrthoFrustum> &&_frustums)
        : resolution(_resolution), depth(_depth), CSMfrustums(std::move(_frustums)) {}
    void generateDepthTexture(GLenum internalFormat = GL_DEPTH_COMPONENT32F, GLenum format = GL_DEPTH_COMPONENT)
    {
        if (CSMTextureArray == nullptr)
        {
            CSMTextureArray = std::make_shared<Texture2DArray>();
            CSMTextureArray->setFilterMin(GL_NEAREST);
            CSMTextureArray->setFilterMax(GL_NEAREST);
            CSMTextureArray->setWrapMode(GL_CLAMP_TO_BORDER);
            const float borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
            glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
            CSMTextureArray->generate(resolution, resolution, depth,
                                      internalFormat, format, GL_FLOAT, NULL, false);
        }
    }
    void generateVSMTexture(GLenum internalFormat = GL_RGBA32F, GLenum format = GL_RGBA)
    {
        if (VSMTextureArray == nullptr)
        {
            VSMTextureArray = std::make_shared<Texture2DArray>();
            VSMTextureArray->setFilterMax(GL_LINEAR);
            VSMTextureArray->setFilterMin(GL_LINEAR);
            VSMTextureArray->setWrapMode(GL_CLAMP_TO_EDGE);
            VSMTextureArray->generate(resolution, resolution, depth, internalFormat, format, GL_FLOAT, NULL, false);
        }
    }
    void generateSATTexture(GLenum internalFormat = GL_RGBA32F, GLenum format = GL_RGBA)
    {
        if (SATTextureArray == nullptr)
        {
            SATTextureArray = std::make_shared<Texture2DArray>();
            SATTextureArray->setFilterMax(GL_LINEAR);
            SATTextureArray->setFilterMin(GL_LINEAR);
            SATTextureArray->setWrapMode(GL_CLAMP_TO_BORDER);
            const float borderColor[] = {0.0f, 0.0f, 0.0f, 0.0f};
            glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
            SATTextureArray->generate(resolution, resolution, depth, internalFormat, format, GL_FLOAT, NULL, false);
        }
    }
    void update(std::vector<OrthoFrustum> &&_frustums)
    {
        CSMfrustums = std::move(_frustums);
    }
    void update(const std::vector<OrthoFrustum> &_frustums)
    {
        CSMfrustums = _frustums;
    }
};

class DirShadowPass;

// 使用多个阴影单元组合
// 尽管如此,我们不采用非正方形分辨率阴影贴图.每张阴影贴图分辨率固定,不可调节
class CascadedShadowComponent
{
public:
    friend class DirShadowPass;
    std::vector<DirShadowUnit> shadowUnits;
    glm::vec3 lightDir;
    Frustum cameraFrustum;
    int numLevels;
    int resolution;

public:
    bool useVSM;

public:
    CascadedShadowComponent(int _numLevels, int _resolution, bool _useVSM)
        : numLevels(_numLevels), resolution(_resolution), useVSM(_useVSM)
    {
        for (int i = 0; i < numLevels; ++i)
        {
            shadowUnits.emplace_back();
        }
    }
    void update(const glm::vec3 &dir, const Frustum &camFrustum)
    {
        lightDir = dir;
        cameraFrustum = camFrustum;
        updateOrthoFrustum();
    }
    void generateShadowResource()
    {
        for (auto &unit : shadowUnits)
        {
            unit.resolution = resolution;
            unit.generateDepthTexture();
            unit.generateVSMTexture();
            unit.generateSATTexture();
        }
    }
    void setToShader(Shader &shaders)
    {
        // 设置级联阴影相关uniform
        // 等待着色器编写完毕再实现
        // struct DirShadowUnit
        // {
        //     mat4 spaceMatrix;
        //     sampler2D depthMap;
        //     sampler2D VSMTexture;
        //     sampler2D SATTexture;
        //     vec3 pos;
        //     float farPlane;
        //     float nearPlane;
        //     float orthoScale;
        // };

        // const int MAX_CASCADES = 6;
        // struct CSMComponent
        // {
        //     DirShadowUnit units[MAX_CASCADES];
        //     int useVSM;
        //     int useVSSM;
        // };
        shaders.setInt("CSM.useVSM", useVSM ? 1 : 0);
        shaders.setInt("CSM.useVSSM", 0); // placeholder
        for (int i = 0; i < shadowUnits.size(); ++i)
        {
            shaders.setMat4(std::format("CSM.units[{}].spaceMatrix", i), shadowUnits[i].frustum.getProjViewMatrix());
            shaders.setTextureAuto(shadowUnits[i].depthTexture->ID, GL_TEXTURE_2D, 0, std::format("CSM.units[{}].depthMap", i));
            if (useVSM)
            {
                shaders.setTextureAuto(shadowUnits[i].VSMTexture->ID, GL_TEXTURE_2D, 0, std::format("CSM.units[{}].VSMTexture", i));
                shaders.setTextureAuto(shadowUnits[i].SATTexture->ID, GL_TEXTURE_2D, 0, std::format("CSM.units[{}].SATTexture", i));
            }
            else
            {
                shaders.setTextureAuto(0, GL_TEXTURE_2D, 0, std::format("CSM.units[{}].VSMTexture", i));
                shaders.setTextureAuto(0, GL_TEXTURE_2D, 0, std::format("CSM.units[{}].SATTexture", i));
            }
            shaders.setFloat(std::format("CSM.units[{}].nearPlane", i), shadowUnits[i].frustum.getNearPlane());
            shaders.setFloat(std::format("CSM.units[{}].farPlane", i), shadowUnits[i].frustum.getFarPlane());
            shaders.setFloat(std::format("CSM.units[{}].orthoScale", i), shadowUnits[i].frustum.getOrthoScaleArea());
            shaders.setUniform(std::format("CSM.units[{}].pos", i), shadowUnits[i].frustum.getPosition());
        }
    }

private:
    // 根据方向,层数,视锥体,进行切割,计算正交体
    OrthoFrustum calculateOrthoFrustum(int level)
    {
        // 实现切割算法
        // 0:0 ,80 1:80, 640,2: 640:5120, 3:5120,far
        // (near,far) = (80*4^(i-1),80*4^i) ;
        // near = near if i==0;
        // far = far if i==numLevels-1
        const float nearLv1 = 80.f;
        const float base = 4.0f;

        auto near = cameraFrustum.getNearPlane();
        auto far = cameraFrustum.getFarPlane();
        float levelNear = static_cast<float>((level == 0) ? near : nearLv1 * pow(base, level - 1));
        float levelFar = static_cast<float>((level == numLevels - 1) ? far : nearLv1 * pow(base, level));
        auto subFrustum = cameraFrustum.getSubFrustum(levelNear, levelFar);
        auto subCorners = subFrustum.getCorners();
        return OrthoFrustum::GenTightFtustum(subCorners, lightDir, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    // 基于摄像机视锥体和光源方向，更新每个阴影单元的正交视锥体
    void updateOrthoFrustum()
    {
        for (int i = 0; i < shadowUnits.size(); ++i)
        {
            // 计算每个阴影单元的正交视锥体
            shadowUnits[i].frustum = calculateOrthoFrustum(i);
        }
    }
};