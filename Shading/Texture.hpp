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

/// @brief ����GL Texture��Դ
class Texture
{
public:
    unsigned int ID;       // �������� ID���� OpenGL ����
    GLenum Target;         // �������ͣ��� GL_TEXTURE_2D��GL_TEXTURE_3D ��
    GLenum InternalFormat; // ������ GPU �д洢���ڲ���ʽ�����磺GL_RGBA8��
    GLenum Format;         // ������ CPU �д洢�ĸ�ʽ�����磺GL_RGBA��
    GLenum Type;           // �������ݵ��������ͣ����磺GL_UNSIGNED_BYTE��
    GLenum FilterMin;      // ������Сʱʹ�õĹ��˷�ʽ
    GLenum FilterMax;      // ����Ŵ�ʱʹ�õĹ��˷�ʽ
    GLenum WrapS;          // S �ᣨˮƽ���������Ʒ�ʽ
    GLenum WrapT;          // T �ᣨ��ֱ���������Ʒ�ʽ
    GLenum WrapR;          // R �ᣨ��ȣ��������Ʒ�ʽ����Ҫ���� 3D ����
    bool Mipmapping;       // �Ƿ����ö༶��Զ����Mipmapping��

    unsigned int Width;  // ����Ŀ�ȣ�������Ϊ��λ��
    unsigned int Height; // ����ĸ߶ȣ�������Ϊ��λ��

    Texture();
    void Generate(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type, void *data, bool mipMapping = true);
    void SetData(void *data);

    void SetFilterMin(GLenum filter);

    void SetFilterMax(GLenum filter);

    void Resize(int ResizeWidth, int ResizeHeight);

    void SetWrapMode(GLenum wrapMode);

    ~Texture();
};