#include "Texture.hpp"

Texture::Texture()
{
    ID = 0;
    Target = GL_TEXTURE_2D;
    InternalFormat = GL_RGBA;
    Format = GL_RGBA;
    Type = GL_FLOAT;
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
void Texture::generate(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type, void *data, bool mipMapping)
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

    // 关闭Mipmap 不应该采用GL_LINEAR_MIPMAP_LINEAR . 这里应该assert
    assert((Mipmapping || (FilterMin != GL_LINEAR_MIPMAP_LINEAR)));
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

void Texture::generateComputeStorage(unsigned int width, unsigned int height, GLenum internalFormat)
{
    if (ID != 0)
    {
        glDeleteTextures(1, &ID);
    }
    glGenTextures(1, &ID);

    Width = width;
    Height = height;
    InternalFormat = internalFormat;

    assert(Target == GL_TEXTURE_2D);
    glBindTexture(Target, ID);
    {
        glTexStorage2D(Target, 1, internalFormat, Width, Height);
        glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(Target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(Target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    glBindTexture(Target, 0);
}

/// @brief 设置纹理数据 在Generate()之后调用 通常是逐帧调用
/// @param data 纹理数据指针 注意与纹理格式一致
void Texture::setData(void *data)
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
void Texture::setWrapMode(GLenum wrapMode)
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
void Texture::setFilterMin(GLenum filter)
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
void Texture::setFilterMax(GLenum filter)
{
    FilterMax = filter;
    glBindTexture(Target, ID);
    glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, FilterMax);
    glBindTexture(Target, 0);
}

void Texture::resize(int ResizeWidth, int ResizeHeight)
{
    ResizeWidth = (ResizeWidth <= 0) ? 1 : ResizeWidth;
    ResizeHeight = (ResizeHeight <= 0) ? 1 : ResizeHeight;

    Width = static_cast<unsigned int>(ResizeWidth);
    Height = static_cast<unsigned int>(ResizeHeight);

    glBindTexture(Target, ID);
    {
        glTexImage2D(Target, 0, InternalFormat, Width, Height, 0, Format, Type, NULL);
        if (Mipmapping)
            glGenerateMipmap(Target);
    }
    glBindTexture(Target, 0);
}

void Texture::resizeComputeStorage(int ResizeWidth, int ResizeHeight)
{

    ResizeWidth = (ResizeWidth <= 0) ? 1 : ResizeWidth;
    ResizeHeight = (ResizeHeight <= 0) ? 1 : ResizeHeight;

    Width = static_cast<unsigned int>(ResizeWidth);
    Height = static_cast<unsigned int>(ResizeHeight);

    generateComputeStorage(Width, Height, InternalFormat);
}

Texture::~Texture()
{
    glDeleteTextures(1, &ID);
}

/*************************************************************************************************************** */

TextureCube::TextureCube()
{
    ID = 0;
    Width = 0;
    Height = 0;
    WrapS = GL_REPEAT;
    WrapT = GL_REPEAT;
    WrapR = GL_REPEAT;
}

///@brief 生成CubeTexture对象,设置属性
///@param width 正方形纹理的宽度，以像素为单位。
///@param height 正方形纹理的高度，以像素为单位。
///@param internalFormat 纹理内部存储的颜色格式（例如 GL_RGBA, GL_RGBA16F）。
///@param format 纹理源数据（即 data 指向的数据）的颜色格式（例如 GL_RGBA, GL_BGR）。
///@param type 纹理源数据的每个颜色分量的数据类型（例如 GL_UNSIGNED_BYTE, GL_FLOAT）。
///@param filterMax 放大过滤模式
///@param filterMin 缩小过滤模式
void TextureCube::generate(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type, GLenum filterMax, GLenum filterMin, bool mipmap)
{
    Width = width;
    Height = height;
    Format = format;
    InternalFormat = internalFormat;
    Type = type;
    FilterMin = filterMin;
    FilterMax = filterMax;

    Mipmapping = mipmap;

    if (ID != 0)
    {
        glDeleteTextures(1, &ID);
    }
    glGenTextures(1, &ID);

    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, filterMax);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, filterMin); // 三线性插值
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, WrapS);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, WrapT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, WrapR);

    for (auto &faceTarget : TextureCube::FaceTargets)
    {
        glTexImage2D(faceTarget, 0, InternalFormat, width, height, 0, format, type, NULL);
    }
    if (mipmap)
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void TextureCube::setFaceData(FaceEnum faceTarget, void *data)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    glTexImage2D(faceTarget,
                 0, InternalFormat, Width, Height, 0, Format, Type, data);
}

void TextureCube::setFilterMin(GLenum filter)
{
    FilterMin = filter;
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, FilterMin);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
void TextureCube::setFilterMax(GLenum filter)
{
    FilterMax = filter;
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, FilterMax);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void TextureCube::resize(int ResizeWidth, int ResizeHeight)
{
    ResizeWidth = (ResizeWidth <= 0) ? 1 : ResizeWidth;
    ResizeHeight = (ResizeHeight <= 0) ? 1 : ResizeHeight;

    Width = static_cast<unsigned int>(ResizeWidth);
    Height = static_cast<unsigned int>(ResizeHeight);

    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    for (auto &faceTarget : TextureCube::FaceTargets)
    {
        glTexImage2D(faceTarget, 0, InternalFormat, Width, Height, 0, Format, Type, NULL);
    }
    if (Mipmapping)
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
void TextureCube::setWrapMode(GLenum wrapMode)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
    {
        WrapS = wrapMode;
        WrapT = wrapMode;
        WrapR = wrapMode;
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, WrapS);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, WrapT);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, WrapR);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

TextureCube::~TextureCube()
{
    glDeleteTextures(1, &ID);
}