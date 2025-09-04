#pragma once
#include "../../Shading/Shader.hpp"
#include "../../Shading/Texture.hpp"
#include "../../Utils/Utils.hpp"
#include "../Renderer.hpp"
/* 1个Pass对应一个FBO , 一个Shader
[in] Textures , Uniform Varibles , Extra Resources
[out] Pass Texture
*/
class Pass
{
protected:
    unsigned int FBO;
    Shader shaders;
    std::string vs_path;
    std::string fs_path;
    std::string gs_path;
    int vp_width;
    int vp_height;

    // 管理GL资源
    //  初始化内部OpenGL资源,如获取FBO,Texture,绑定framebuffer等
    virtual void initializeGLResources() = 0;
    // 对称方法 清理裸GL资源. 析构函数调用
    virtual void cleanUpGLResources() = 0;

public:
    Pass(int _vp_width = 0, int _vp_height = 0, std::string _vs_path = "",
         std::string _fs_path = "",
         std::string _gs_path = "")
        : vp_width(_vp_width), vp_height(_vp_height), vs_path(_vs_path), fs_path(_fs_path), gs_path(_gs_path)
    {
        shaders = Shader(vs_path.c_str(), fs_path.c_str(), gs_path.c_str());
    }
    virtual void reloadCurrentShaders()
    {
        shaders = Shader(vs_path.c_str(), fs_path.c_str(), gs_path.c_str());
        contextSetup();
    }

    // 上下文设置
    virtual void contextSetup() = 0;

    virtual void resize(int _width, int _height) = 0;
    virtual ~Pass() = default;
    void setToggle(bool status, std::string toggle)
    {
        shaders.use();
        shaders.setInt(toggle, status ? 1 : 0);
    }
};

// 将最终通道渲染到屏幕
// 输出到默认FBO
class ScreenPass : public Pass
{
private:
    void initializeGLResources() {}
    void cleanUpGLResources() override {};

public:
    ScreenPass(int _vp_width, int _vp_height, std::string _vs_path,
               std::string _fs_path)
        : Pass(_vp_width, _vp_height, _vs_path, _fs_path)
    {
        initializeGLResources();
        contextSetup();
    }
    ~ScreenPass()
    {
        cleanUpGLResources();
    }

    void contextSetup() {}

    void resize(int _width, int _height)
    {
        if (vp_width == _width &&
            vp_height == _height)
        {
            return;
        }
        vp_width = _width;
        vp_height = _height;

        contextSetup();
    }

    void render(unsigned int finalTextureID)
    {

        glViewport(0, 0, vp_width, vp_height);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // 设置着色器参数
        shaders.use();
        shaders.setTextureAuto(finalTextureID, GL_TEXTURE_2D, 0, "tex_sampler");
        Renderer::DrawQuad();
    }
};