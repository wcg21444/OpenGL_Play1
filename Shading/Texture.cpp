#include "Texture.hpp"

///@brief ����2DTexture����,��������
///@param width ����Ŀ�ȣ�������Ϊ��λ��
///@param height ����ĸ߶ȣ�������Ϊ��λ��
///@param internalFormat �����ڲ��洢����ɫ��ʽ������ GL_RGBA, GL_RGBA16F����
///@param format ����Դ���ݣ��� data ָ������ݣ�����ɫ��ʽ������ GL_RGBA, GL_BGR����
///@param type ����Դ���ݵ�ÿ����ɫ�������������ͣ����� GL_UNSIGNED_BYTE, GL_FLOAT����
///@param data ָ�������������ݵ�ָ�롣���Ϊ NULL����ֻ�����ڴ档
/// ��initializeGLResources() ����
void Texture::Generate(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type, void *data)
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

void Texture::SetData(void *data)
{
    glBindTexture(Target, ID);
    glTexImage2D(Target, 0, InternalFormat, Width, Height, 0, Format, Type, data);
    glBindTexture(Target, 0);
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

void Texture::Resize(int ResizeWidth, int ResizeHeight)
{
    // ���ô�С
    // ��������Tex
    // ResizeTexture ��Ҫɾ��ԭ��Texture GL����
    assert(ResizeWidth >= 0);
    assert(ResizeHeight >= 0);

    Width = static_cast<unsigned int>(ResizeWidth);
    Height = static_cast<unsigned int>(ResizeHeight);

    Generate(Width, Height, InternalFormat, Format, Type, NULL);
}

Texture::Texture()
{
}

Texture::~Texture()
{
    glDeleteTextures(1, &ID);
}