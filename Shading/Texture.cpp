#include "Texture.hpp"

///@brief 生成2DTexture对象,设置属性
///@param width 纹理的宽度，以像素为单位。
///@param height 纹理的高度，以像素为单位。
///@param internalFormat 纹理内部存储的颜色格式（例如 GL_RGBA, GL_RGBA16F）。
///@param format 纹理源数据（即 data 指向的数据）的颜色格式（例如 GL_RGBA, GL_BGR）。
///@param type 纹理源数据的每个颜色分量的数据类型（例如 GL_UNSIGNED_BYTE, GL_FLOAT）。
///@param data 指向纹理像素数据的指针。如果为 NULL，则只分配内存。
void Texture::Generate(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type, void *data)
{
    glGenTextures(1, &ID);

    Width = width;
    Height = height;
    InternalFormat = internalFormat;
    Format = format;
    Type = type;

    assert(Target == GL_TEXTURE_2D);
    glBindTexture(Target, ID);
    {
        glTexImage2D(Target, 0, internalFormat, width, height, 0, format, type, data);
        glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, FilterMin);
        glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, FilterMax);
        glTexParameteri(Target, GL_TEXTURE_WRAP_S, WrapS);
        glTexParameteri(Target, GL_TEXTURE_WRAP_T, WrapT);
        if (Mipmapping)
            glGenerateMipmap(Target);
    }
    glBindTexture(Target, 0);
}

void Texture::SetData(void *data)
{
    glBindTexture(Target, ID);
    glTexImage2D(Target, 0, InternalFormat, Width, Height, 0, Format, Type, data);
}

// void Texture::SetWrapMode(GLenum wrapMode)
// {
//     if (Target == GL_TEXTURE_2D)
//     {
//         WrapS = wrapMode;
//         WrapT = wrapMode;
//         glTexParameteri(Target, GL_TEXTURE_WRAP_S, wrapMode);
//         glTexParameteri(Target, GL_TEXTURE_WRAP_T, wrapMode);
//     }
// }

void Texture::SetFilterMin(GLenum filter)
{
    glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, filter);
}

void Texture::SetFilterMax(GLenum filter)
{
    glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, filter);
}

void Texture::Resize(unsigned int Width, unsigned int Height)
{
    // 设置大小
    // 删除旧Texture
    // 重新分配资源
}
