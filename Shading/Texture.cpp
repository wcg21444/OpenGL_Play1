#include "Texture.hpp"

Texture::Texture()
{
    ID = 0;
    Target = GL_TEXTURE_2D;
    InternalFormat = GL_RGBA;
    Format = GL_RGBA;
    Type = GL_UNSIGNED_BYTE;
    FilterMin = GL_LINEAR_MIPMAP_LINEAR;
    FilterMax = GL_LINEAR;
    WrapS = GL_REPEAT;
    WrapT = GL_REPEAT;
    WrapR = GL_REPEAT;
    Mipmapping = true;
    Width = 0;
    Height = 0;
}

///@brief 生成2DTexture对象,设置属性
///@param width 纹理的宽度，以像素为单位。
///@param height 纹理的高度，以像素为单位。
///@param internalFormat 纹理内部存储的颜色格式（例如 GL_RGBA, GL_RGBA16F）。
///@param format 纹理源数据（即 data 指向的数据）的颜色格式（例如 GL_RGBA, GL_BGR）。
///@param type 纹理源数据的每个颜色分量的数据类型（例如 GL_UNSIGNED_BYTE, GL_FLOAT）。
///@param data 指向纹理像素数据的指针。如果为 NULL，则只分配内存。
/// 在initializeGLResources() 调用
void Texture::Generate(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type, void *data, bool mipMapping)
{
    if (ID != 0)
    {
        glDeleteTextures(1, &ID);
    }
    glGenTextures(1, &ID);

    Width = width;
    Height = height;
    InternalFormat = internalFormat;
    Format = format;
    Type = type;
    Mipmapping = mipMapping;

    assert(Target == GL_TEXTURE_2D);
    glBindTexture(Target, ID);
    {
        glTexImage2D(Target, 0, internalFormat, Width, Height, 0, format, type, data);
        glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, FilterMin);
        glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, FilterMax);
        glTexParameteri(Target, GL_TEXTURE_WRAP_S, WrapS);
        glTexParameteri(Target, GL_TEXTURE_WRAP_T, WrapT);
        if (Mipmapping)
            glGenerateMipmap(Target);
    }
    glBindTexture(Target, 0);
}

/// @brief 设置纹理数据 在Generate()之后调用 通常是逐帧调用
/// @param data 纹理数据指针 注意与纹理格式对齐
void Texture::SetData(void *data)
{
    glBindTexture(Target, ID);
    glTexImage2D(Target, 0, InternalFormat, Width, Height, 0, Format, Type, data);
    glBindTexture(Target, 0);
}

/**
 * @brief 设置纹理的环绕模式。
 * @param wrapMode 用于环绕的 OpenGL 枚举值。
 *
 * 此函数同时设置 S、T 和 R 轴的纹理环绕方式。
 * 环绕模式决定了当纹理坐标超出 [0, 1] 范围时，纹理如何重复或延伸。
 * 可选的环绕模式包括：
 * - GL_REPEAT: 默认值，重复纹理图像。
 * - GL_MIRRORED_REPEAT: 镜像重复纹理。
 * - GL_CLAMP_TO_EDGE: 延伸纹理边缘的颜色。
 * - GL_CLAMP_TO_BORDER: 延伸到用户自定义的边框颜色。
 */
void Texture::SetWrapMode(GLenum wrapMode)
{
    glBindTexture(Target, ID);
    if (Target == GL_TEXTURE_2D)
    {
        WrapS = wrapMode;
        WrapT = wrapMode;
        glTexParameteri(Target, GL_TEXTURE_WRAP_S, wrapMode);
        glTexParameteri(Target, GL_TEXTURE_WRAP_T, wrapMode);
    }
    glBindTexture(Target, 0);
}

/**
 * @brief 设置纹理的缩小过滤模式。
 * @param filter 用于缩小过滤的 OpenGL 枚举值。
 *
 * 当纹理被渲染得比其原始大小时，此函数决定如何处理纹理。
 * 可选的过滤模式包括：
 * - GL_NEAREST: 像素化效果，速度快。
 * - GL_LINEAR: 平滑效果，质量更高。
 * - GL_NEAREST_MIPMAP_NEAREST
 * - GL_LINEAR_MIPMAP_NEAREST
 * - GL_NEAREST_MIPMAP_LINEAR
 * - GL_LINEAR_MIPMAP_LINEAR
 */
void Texture::SetFilterMin(GLenum filter)
{
    FilterMin = filter;
    glBindTexture(Target, ID);
    glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, FilterMin);
    glBindTexture(Target, 0);
}
/**
 * @brief 设置纹理的放大过滤模式。
 * @param filter 用于放大过滤的 OpenGL 枚举值。
 *
 * 当纹理被渲染得比其原始大小时，此函数决定如何处理纹理。
 * 可选的过滤模式通常只有：
 * - GL_NEAREST: 像素化效果。
 * - GL_LINEAR: 平滑效果。
 */
void Texture::SetFilterMax(GLenum filter)
{
    FilterMax = filter;
    glBindTexture(Target, ID);
    glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, FilterMax);
    glBindTexture(Target, 0);
}

void Texture::Resize(int ResizeWidth, int ResizeHeight)
{
    // 设置大小
    // 重新生成Tex
    // ResizeTexture 需要删除原有Texture GL对象
    // assert(ResizeWidth >= 0);
    // assert(ResizeHeight >= 0);
    // clamp
    ResizeWidth = (ResizeWidth <= 0) ? 1 : ResizeWidth;
    ResizeHeight = (ResizeHeight <= 0) ? 1 : ResizeHeight;

    Width = static_cast<unsigned int>(ResizeWidth);
    Height = static_cast<unsigned int>(ResizeHeight);

    Generate(Width, Height, InternalFormat, Format, Type, NULL);
}

Texture::~Texture()
{
    glDeleteTextures(1, &ID);
}