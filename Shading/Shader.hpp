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
#include <unordered_set>
#include <iostream>
#include <functional>

#include "../Utils/DebugOutput.hpp"
#include "GLResource.hpp"

class ShaderBase : public GLResource
{
public:
    unsigned int &programID = GLResource::ID;
    bool used = false;
    std::unordered_map<std::string, int> uniformLocationMap;
    std::unordered_set<std::string> warningMsgSet;
    bool ignoreNotFoundWarning = false;

public:
    static GLenum GetTextureUnitEnum(int textureLocation);
    static GLint GetTextureUnitsLimits();
    static std::string LoadShaderFile(const char *shader_path);
    static void CompileShader(const char *shader_source, GLenum shader_type, unsigned int &shader_id, const char *path);

public:
    ShaderBase() {}
    // 禁用拷贝构造和拷贝赋值
    ShaderBase(const ShaderBase &) = delete;
    ShaderBase &operator=(const ShaderBase &) = delete;

    virtual ~ShaderBase()
    {
        if (programID)
        {
            DebugOutput::AddLog("Shader Program ID:{} Was Deleted~\n", programID);
            glDeleteProgram(programID);
            programID = 0;
            used = false;
        }
    }
    virtual GLint getUniformLocationSafe(const std::string &name) = 0;                                                                              // 接口方法
    GLint getUniformLocationSafe(const std::string &name, const std::function<void(const std::string &uniformName, GLuint programID)> &onNotFound); // 通用方法

    bool hasUniform(const std::string &name)
    {
        return glGetUniformLocation(programID, name.c_str()) != -1;
    }

    void use()
    {
        glUseProgram(programID);
        used = true;
    }

    void toggleIgnoreNotFoundWarning()
    {
        ignoreNotFoundWarning = !ignoreNotFoundWarning;
    }

    void setUniform4fv(const std::string &name, GLsizei count, const float *value);
    void setUniform4fv(const std::string &name, const glm::vec4 &vec4);
    void setUniform3fv(const std::string &name, const glm::vec3 &vec3);
    void setMat4(const std::string &name, const glm::mat4 &mat);
    void setFloat(const std::string &name, float f);
    void setInt(const std::string &name, int i);
    void setUniform(const std::string &name, const glm::vec4 &vec4);
    void setUniform(const std::string &name, const glm::vec3 &vec3);
    void setUniform(const std::string &name, const glm::vec2 &vec2);
    void setUniform(const std::string &name, const glm::mat4 &mat);
    void setUniform(const std::string &name, float f);
    void setUniform(const std::string &name, int i);
};

class Shader : public ShaderBase
{
public:
    // 增改成员变量记得在&operator=(&&)更新
    std::string vs_path;
    std::string fs_path;
    std::string gs_path;
    std::unordered_map<std::string, int> textureLocationMap;
    int texLocationID;

private:
    GLint getUniformLocationSafe(const std::string &name) override;

private:
    unsigned int linkShader(unsigned int vertexShader, unsigned int fragmentShader, unsigned int geometryShader, bool hasGS);

public:
    Shader();
    Shader(const char *vs_path, const char *fs_path, const char *gs_path = nullptr);
    Shader(Shader &&other) noexcept;
    Shader &operator=(Shader &&other) noexcept;
    void setTextureAuto(GLuint textureID, GLenum textureTarget, int shaderTextureLocation, const std::string &samplerUniformName);
};

class ComputeShader : public ShaderBase
{
private:
    GLint getUniformLocationSafe(const std::string &name) override;

public:
    static constexpr int ThreadGroupSize = 32;
    [[nodiscard]] static int GetGroupSize(int size)
    {
        return (size + ThreadGroupSize - 1) / ThreadGroupSize;
    }

public:
    std::string cs_path;

    ComputeShader(std::string cs_path);

    ComputeShader &operator=(ComputeShader &&other) noexcept;
};