#pragma once
#include "Pass.hpp"
class PostProcessShaderUI;

class PostProcessPass : public Pass
{
private:
    Texture postProcessPassTex;
    std::unique_ptr<PostProcessShaderUI> shaderUI;

private:
    void initializeGLResources() override;
    void cleanUpGLResources() override;

public:
    PostProcessPass(int _vp_width, int _vp_height, std::string _vs_path,
                    std::string _fs_path);
    ~PostProcessPass(); // 使用智能指针实现pImpl , 必须声明析构函数

    void contextSetup() override;
    void resize(int _width, int _height) override;
    unsigned int getTextures();
    void render(unsigned int screenTex, unsigned int ssaoTex, const std::vector<unsigned int> &bloomTexArray);
};