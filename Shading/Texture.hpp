#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
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

// Texture�Ƕ�Texture GL����ķ�װ
// GL���� ���� ����ID �ڲ���ʽ ��ʽ �������� ��Щ��ʼ������; �Լ�״̬����,Filter,Wrap�Ƕ�ʹ�÷����ŵ�.
//  ���� ����Texture : ��ʼ����Դ
//       ����״̬
//       ��������
//       ������С
/// @brief ����GL CubeTexture��Դ
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
    GLenum InternalFormat; // ������ GPU �д洢���ڲ���ʽ�����磺GL_RGBA8��
    GLenum Format;         // ������ CPU �д洢�ĸ�ʽ�����磺GL_RGBA��
    GLenum Type;           // �������ݵ��������ͣ����磺GL_UNSIGNED_BYTE��
    GLenum FilterMin;      // ������Сʱʹ�õĹ��˷�ʽ
    GLenum FilterMax;      // ����Ŵ�ʱʹ�õĹ��˷�ʽ
    GLenum WrapS;          // S �ᣨˮƽ���������Ʒ�ʽ
    GLenum WrapT;          // T �ᣨ��ֱ���������Ʒ�ʽ
    GLenum WrapR;          // R �ᣨ��ȣ��������Ʒ�ʽ����Ҫ���� 3D ����
    bool Mipmapping;       // �Ƿ����ö༶��Զ����Mipmapping��
public:
    unsigned int ID; // �������� ID���� OpenGL ����

    unsigned int Width;  // ����Ŀ�ȣ�������Ϊ��λ��
    unsigned int Height; // ����ĸ߶ȣ�������Ϊ��λ��

    TextureCube();

    void Generate(unsigned int width, unsigned int height, GLenum internalFormat, GLenum format, GLenum type, GLenum filterMax, GLenum filterMin, bool mipmap);

    void SetFaceData(FaceEnum faceTarget, void *data);

    void SetFilterMin(GLenum filter);

    void SetFilterMax(GLenum filter);

    void Resize(int ResizeWidth, int ResizeHeight);

    void SetWrapMode(GLenum wrapMode);

    ~TextureCube();
};