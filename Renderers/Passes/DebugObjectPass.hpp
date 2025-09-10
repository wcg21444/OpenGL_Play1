#pragma once
#include "Pass.hpp"

using DebugObjectDrawCall = std::function<void(Shader &shaders, glm::mat4 modelMatrix)>;

class RenderTarget;
class Texture;
class DebugObjectPass : public Pass
{
private:
    std::shared_ptr<RenderTarget> renderTarget = nullptr;
    std::shared_ptr<Texture> debugObjectPassTex = nullptr;

    void initializeGLResources() override;
    void cleanUpGLResources() override;

public:
    DebugObjectPass(int _vp_width, int _vp_height, std::string _vs_path, std::string _fs_path);
    ~DebugObjectPass()
    {
        cleanUpGLResources();
    }

    void contextSetup() override;

    void resize(int _width, int _height) override;

    void render(std::queue<DebugObjectDrawCall> &drawQueue, Camera &cam);

    unsigned int getTexture();

};
