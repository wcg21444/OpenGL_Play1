
#pragma once
#include "Pass.hpp"

/*
Feature:
����:Tex����,Tex�ֱ���,dirLight
���:��Ӱ��ͼ

��������Ӱ��ͼ��������.
*/
class PointShadowPass : public Pass
{
private:
    glm::mat4 shadowProj;
    float aspect;
    float near;

private:
    void initializeGLResources();
    void attachDepthMap(const unsigned int _depthCubemap);

public:
    float far;

public:
    PointShadowPass(std::string _vs_path, std::string _fs_path, std::string _gs_path);

    void contextSetup() override;
    void render(PointLight &light, std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model, glm::mat4 &lightSpaceMatrix);

    void renderToTexture(
        const PointLight &light,
        std::vector<std::unique_ptr<Object>> &scene,
        glm::mat4 &model,
        int width,
        int height);
};
