#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
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