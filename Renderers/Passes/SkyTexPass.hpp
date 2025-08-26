
#pragma once
#include "Pass.hpp"

class SkyTexPass : public Pass
{
private:
    glm::mat4 camProj;
    float aspect;
    float nearPlane;
    int cubemapSize;

    unsigned int skyCubemapTex;

private:
    void initializeGLResources();

public:
    float farPlane;

public:
    SkyTexPass(std::string _vs_path, std::string _fs_path, /*  std::string _gs_path, */ int _cubemapSize);
    ~SkyTexPass();
    void contextSetup() override;

    void render(PointLight &light, std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model, glm::mat4 &lightSpaceMatrix);

    void resize(int _width, int _height) override;

    void render(
        RenderParameters &renderParameters);
    unsigned int getCubemap();
};
