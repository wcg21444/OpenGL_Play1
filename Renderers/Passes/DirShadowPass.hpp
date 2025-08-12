
#pragma once
#include "Pass.hpp"
/*
Feature:
����:Tex����,Tex�ֱ���,dirLight
���:��Ӱ��ͼ

��������Ӱ��ͼ��������.
*/
class DirShadowPass : public Pass
{

    void initializeGLResources();
    void attachDepthMap(const unsigned int _depthMap);

public:
    DirShadowPass(std::string _vs_path, std::string _fs_path);

    void contextSetup() override;
    void render(PointLight &light, std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model, glm::mat4 &lightSpaceMatrix);

    void renderToTexture(
        const DirectionLight &light,
        std::vector<std::unique_ptr<Object>> &scene,
        glm::mat4 &model,
        int width,
        int height);
};
