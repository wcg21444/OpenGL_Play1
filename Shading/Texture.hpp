#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <iostream>

/// @brief 管理GL Texture资源
class Texture
{
public:
    unsigned int ID;       // 纹理对象的 ID，由 OpenGL 分配
    GLenum Target;         // 纹理类型，如 GL_TEXTURE_2D、GL_TEXTURE_3D 等
    GLenum InternalFormat; // 纹理在 GPU 中存储的内部格式（例如：GL_RGBA8）
    GLenum Format;         // 纹理在 CPU 中存储的格式（例如：GL_RGBA）
    GLenum Type;           // 纹素数据的数据类型（例如：GL_UNSIGNED_BYTE）
    GLenum FilterMin;      // 纹理缩小时使用的过滤方式
    GLenum FilterMax;      // 纹理放大时使用的过滤方式
    GLenum WrapS;          // S 轴（水平）的纹理环绕方式
    GLenum WrapT;          // T 轴（垂直）的纹理环绕方式
    GLenum WrapR;          // R 轴（深度）的纹理环绕方式，主要用于 3D 纹理
    bool Mipmapping;       // 是否启用多级渐远纹理（Mipmapping）

    unsigned int Width;  // 纹理的宽度（以像素为单位）
    unsigned int Height; // 纹理的高度（以像素为单位）

    Texture();
    void Generate(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type, void *data, bool mipMapping = true);
    void SetData(void *data);

    void SetFilterMin(GLenum filter);

    void SetFilterMax(GLenum filter);

    void Resize(int ResizeWidth, int ResizeHeight);

    void SetWrapMode(GLenum wrapMode);

    ~Texture();
};

// Texture是对Texture GL对象的封装
// GL对象 包括 对象ID 内部格式 格式 数据类型 这些初始化属性; 以及状态属性,Filter,Wrap是对使用方开放的.
//  方法 生成Texture : 初始化资源
//       设置状态
//       设置数据
//       调整大小
/// @brief 管理GL CubeTexture资源
class TextureCube
{
public:
    enum FaceEnum
    {
        Right = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
        Left = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
        Top = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
        Bottom = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
        Front = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
        Back = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };
    //{ Right, Left, Top, Bottom, Front, Back }
    inline static const std::vector<FaceEnum> FaceTargets = {Right, Left, Top, Bottom, Front, Back};

    inline static std::vector<glm::mat4> GenearteViewMatrices(const glm::vec3 &position)
    {
        std::vector<glm::mat4> viewMatrices;
        viewMatrices.push_back(
            glm::lookAt(position, position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
        viewMatrices.push_back(
            glm::lookAt(position, position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
        viewMatrices.push_back(
            glm::lookAt(position, position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
        viewMatrices.push_back(
            glm::lookAt(position, position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
        viewMatrices.push_back(
            glm::lookAt(position, position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
        viewMatrices.push_back(
            glm::lookAt(position, position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
        return viewMatrices;
    }

private:
    GLenum InternalFormat; // 纹理在 GPU 中存储的内部格式（例如：GL_RGBA8）
    GLenum Format;         // 纹理在 CPU 中存储的格式（例如：GL_RGBA）
    GLenum Type;           // 纹素数据的数据类型（例如：GL_UNSIGNED_BYTE）
    GLenum FilterMin;      // 纹理缩小时使用的过滤方式
    GLenum FilterMax;      // 纹理放大时使用的过滤方式
    GLenum WrapS;          // S 轴（水平）的纹理环绕方式
    GLenum WrapT;          // T 轴（垂直）的纹理环绕方式
    GLenum WrapR;          // R 轴（深度）的纹理环绕方式，主要用于 3D 纹理
    bool Mipmapping;       // 是否启用多级渐远纹理（Mipmapping）
public:
    unsigned int ID; // 纹理对象的 ID，由 OpenGL 分配

    unsigned int Width;  // 纹理的宽度（以像素为单位）
    unsigned int Height; // 纹理的高度（以像素为单位）

    TextureCube();

    void Generate(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type, GLenum filterMax, GLenum filterMin, bool mipmap);

    void SetFaceData(FaceEnum faceTarget, void *data);

    void SetFilterMin(GLenum filter);

    void SetFilterMax(GLenum filter);

    void Resize(int ResizeWidth, int ResizeHeight);

    void SetWrapMode(GLenum wrapMode);

    ~TextureCube();
};