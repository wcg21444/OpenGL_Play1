
#pragma once
#include "Pass.hpp"
#include "../LightSource/Shadow.hpp"

#include "../Shading/Texture.hpp"
/*
Feature:
输入:Tex对象,Tex分辨率,dirLight
输出:阴影贴图

不负责阴影贴图创建管理.
*/
class DirShadowPass : public Pass
{

    void initializeGLResources();
    void cleanUpGLResources() override;
    void attachDepthMap(const unsigned int _depthMap);

public:
    DirShadowPass(std::string _vs_path, std::string _fs_path);
    ~DirShadowPass() { cleanUpGLResources(); }

    void contextSetup() override;

    void resize(int _width, int _height) override;

    void renderToTexture(
        const DirectionLight &light,
        Scene &scene,
        glm::mat4 &model,
        int width,
        int height);

    void render(DirShadowUnit &shadowUnit, Scene &scene, glm::mat4 &model);
    void render(CascadedShadowComponent &CSMComponent, Scene &scene, glm::mat4 &model);
};

class DirShadowVSMPass : public Pass
{
    void initializeGLResources() override;
    void cleanUpGLResources() override;

public:
    DirShadowVSMPass(std::string _vs_path, std::string _fs_path);
    ~DirShadowVSMPass() { cleanUpGLResources(); }

    void contextSetup() override;

    void resize(int _width, int _height) override;

    void renderToVSMTexture(const DirectionLight &light, int width, int height);
    void renderToVSMTexture(DirShadowUnit &shadowUnit);
    void renderToVSMTexture(CascadedShadowComponent &CSMComponent);
};

class DirShadowSATPass : public Pass
{
public:
    Texture2D SATRowTexture;
    Texture2D momentsTex;
    Texture2D SATCompInputTex;
    Texture2D SATCompOutputTex;
    ComputeShader SATComputeShader = ComputeShader("Shaders/ShadowMapping/SATCompute.comp");
    ComputeShader momentComputeShader = ComputeShader("Shaders/ShadowMapping/MomentsCompute.comp");

private:
    unsigned int FBOCol;
    unsigned int FBORow;

    void initializeGLResources() override;
    void cleanUpGLResources() override;

public:
    DirShadowSATPass(std::string _vs_path, std::string _fs_path);
    ~DirShadowSATPass() { cleanUpGLResources(); }

    void reloadCurrentShaders() override;

    void contextSetup() override;

    void resize(int _width, int _height) override;

    void renderToSATTexture(const DirectionLight &light, int width, int height);
    void renderToSATTexture(DirShadowUnit &shadowUnit);

    void computeSAT(const unsigned int depthTextureID, int width, int height);
};