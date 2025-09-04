
#pragma once
#include "Pass.hpp"
/*
Feature:
输入:Tex对象,Tex分辨率,dirLight
输出:阴影贴图

不负责阴影贴图创建管理.
*/
class PointShadowPass : public Pass
{
private:
    void initializeGLResources() override;
    void cleanUpGLResources() override;

    void attachDepthMap(const unsigned int _depthCubemap);

public:
    PointShadowPass(std::string _vs_path, std::string _fs_path, std::string _gs_path);
    ~PointShadowPass()
    {
        cleanUpGLResources();
    }
    void contextSetup() override;

    void resize(int _width, int _height) override;

    void renderToTexture(
        const PointLight &light,
        std::vector<std::unique_ptr<Object>> &scene,
        glm::mat4 &model,
        int width,
        int height);
};

class PointShadowVSMPass : public Pass
{
private:
    void initializeGLResources();
    void cleanUpGLResources() override;

public:
    PointShadowVSMPass(std::string _vs_path, std::string _fs_path);
    ~PointShadowVSMPass();
    void contextSetup() override;

    void resize(int _width, int _height) override;

    void renderToVSMTexture(
        const PointLight &light);
};
