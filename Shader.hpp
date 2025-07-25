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

#include "DebugOutput.hpp"

static GLint s_maxTextureUnits = -1;

class Shader
{
public:
    std::string vs_path;
    std::string fs_path;
    std::string gs_path;
    unsigned int progrm_ID;
    bool used = false;
    std::unordered_map<std::string, int> textureLocationMap;
    int location_ID;

private:
    std::string loadShaderFile(const char *shader_path)
    {
        std::fstream shader_file(shader_path, std::ios::in);
        if (!shader_file.is_open()) // 文件可能因为各种原因(不存在,权限,打开方式等) 打不开
        {
            std::cerr << "(errno " << errno << "): " << strerror(errno) << std::endl;
            throw std::runtime_error("Failed to open shader file: " + std::string(shader_path));
        }
        std::stringstream shader_buffer;
        shader_buffer << shader_file.rdbuf();         // 将文件内容读入stringstream
        if (shader_file.fail() && !shader_file.eof()) // 文件可能因为各种原因读取不了
        {
            std::cerr << "(errno " << errno << "): " << strerror(errno) << std::endl;
            throw std::runtime_error("Failed to read shader file: " + std::string(shader_path));
        }
        return shader_buffer.str();
    }
    GLint getUniformLocationSafe(const std::string &name)
    {
        if (!used)
        {
            throw std::runtime_error("Attempted to set uniform '" + name + "' while shader is not active (glUseProgram was not called).");
        }

        GLint location = glGetUniformLocation(progrm_ID, name.c_str());

        return location;
    }

    //[in] shader_source , shader_type
    // [out] shader_id
    void compileShader(const char *shader_source, GLenum shader_type, unsigned int &shader_id)
    {
        shader_id = glCreateShader(shader_type);
        glShaderSource(shader_id, 1, &shader_source, NULL);
        glCompileShader(shader_id);

        int success;
        char infoLog[512];
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader_id, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
            throw std::runtime_error("Failed to compile shader");
        }
    }

public:
    Shader() : location_ID(0) {}
    Shader(const char *vs_path, const char *fs_path, const char *gs_path = nullptr) : Shader()
    {
        this->vs_path = vs_path;
        this->fs_path = fs_path;
        this->gs_path = gs_path ? gs_path : "";
        bool hasGS = gs_path;
        if (hasGS)
        {
            hasGS = gs_path[0] != '\0';
        }
        std::string shader_buf;
        // Config Vertex Shader
        try
        {
            shader_buf = loadShaderFile(vs_path);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Shader Load Error: " << e.what() << std::endl;
            throw;
        }

        const char *vertexShaderSource = shader_buf.c_str();
        unsigned int vertexShader;
        compileShader(vertexShaderSource, GL_VERTEX_SHADER, vertexShader);

        // Config Fragment Shader
        try
        {
            shader_buf = loadShaderFile(fs_path);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Shader Load Error: " << e.what() << std::endl;
            throw;
        }
        const char *fragmentShaderSource = shader_buf.c_str();
        unsigned int fragmentShader;
        compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER, fragmentShader);

        unsigned int geometryShader;
        if (hasGS)
        { // Config Geometry Shader
            try
            {
                shader_buf = loadShaderFile(gs_path);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Shader Load Error: " << e.what() << std::endl;
                throw;
            }
            const char *geometryShadersource = shader_buf.c_str();
            compileShader(geometryShadersource, GL_GEOMETRY_SHADER, geometryShader);
        }
        // Cofig Shader Program
        int success;
        char infoLog[512];
        progrm_ID = glCreateProgram();

        glAttachShader(progrm_ID, vertexShader);
        glAttachShader(progrm_ID, fragmentShader);
        if (hasGS)
            glAttachShader(progrm_ID, geometryShader);
        glLinkProgram(progrm_ID);

        glGetProgramiv(progrm_ID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(progrm_ID, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n"
                      << infoLog << std::endl;
        }
        // Delete our used Shaders
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        if (hasGS)
            glDeleteShader(geometryShader);
    }
    ~Shader()
    {
        if (progrm_ID)
        {
            DebugOutput::AddLog("Shader Program ID:{} Was Deleted~\n", progrm_ID);
            glDeleteProgram(progrm_ID);
        }
    }
    Shader &operator=(Shader &&other) noexcept
    {
        if (this != &other)
        {
            if (progrm_ID)
            {
                DebugOutput::AddLog("Shader Program ID:{} Was Deleted\n", progrm_ID);
                glDeleteProgram(progrm_ID);
            }
            this->progrm_ID = other.progrm_ID;
            this->used = other.used;
            other.progrm_ID = 0; // 释放源对象资源
        }
        return *this;
    }

    bool hasUniform(const std::string &name)
    {
        GLint location = glGetUniformLocation(progrm_ID, name.c_str());
        return location != -1; // 返回是否找到该 uniform
    }

    void use()
    {
        glUseProgram(progrm_ID);
        used = true;
    }
    void setUniform4fv(const std::string &name, GLsizei count, const float *value)
    { // Changed float* to const float*
        GLint location = getUniformLocationSafe(name);
        glUniform4fv(location, count, value);
    }

    void setUniform4fv(const std::string &name, const glm::vec4 &vec4)
    { // Changed to const reference
        GLint location = getUniformLocationSafe(name);
        glUniform4fv(location, 1, glm::value_ptr(vec4));
    }

    void setUniform3fv(const std::string &name, const glm::vec3 &vec3)
    { // Changed to const reference
        GLint location = getUniformLocationSafe(name);
        glUniform3fv(location, 1, glm::value_ptr(vec3));
    }

    void setMat4(const std::string &name, const glm::mat4 &mat)
    { // Changed to const reference
        GLint location = getUniformLocationSafe(name);
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }

    void setFloat(const std::string &name, float f)
    {
        GLint location = getUniformLocationSafe(name);
        glUniform1f(location, f);
    }

    void setInt(const std::string &name, int i)
    {
        GLint location = getUniformLocationSafe(name);
        glUniform1i(location, i);
    }

    // TODO Shaders 接管 TextureLocation分配
    // 分配一个局部唯一的数.联想到数据库的ID. 考虑用一个自增的数作为location.
    void setTextureAuto(GLuint textureID, GLenum textureTarget, int shaderTextureLocation, const std::string &samplerUniformName)
    {
        if (textureLocationMap.find(samplerUniformName) == textureLocationMap.end())
        {
            textureLocationMap.insert({samplerUniformName, location_ID});
            location_ID++;
        }
        int location = textureLocationMap.at(samplerUniformName);
        GLenum activeTextureUnit = getTextureUnitEnum(location);
        // 激活纹理单元
        glActiveTexture(activeTextureUnit);

        // 绑定纹理
        glBindTexture(textureTarget, textureID);

        // 获取 uniform 位置并设置
        GLint samplerLoc = getUniformLocationSafe(samplerUniformName);
        if (samplerLoc == -1)
        {
            std::cerr << "Warning: Uniform '" << samplerUniformName << "' not found in shader program " << progrm_ID << std::endl;
        }
        else
        {
            glUniform1i(samplerLoc, location);
        }
    }

    inline static GLenum getTextureUnitEnum(int textureLocation)
    {
        if (s_maxTextureUnits == -1)
        {
            // 如果没有初始化，则尝试初始化或抛出错误
            throw std::runtime_error("OpenGL texture limits not initialized. Call initializeTextureLimits() first.");
        }

        if (textureLocation < 0 || textureLocation >= s_maxTextureUnits)
        {
            std::string errorMsg = "Texture location " + std::to_string(textureLocation) +
                                   " out of bounds. Max texture units: " + std::to_string(s_maxTextureUnits) + ".";
            throw std::out_of_range(errorMsg);
        }
        return GL_TEXTURE0 + textureLocation;
    }
    inline static void initializeTextureLimits()
    {
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &s_maxTextureUnits);
        if (s_maxTextureUnits == -1)
        {
            // 处理错误或设置默认值
            std::cerr << "Warning: Could not query GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS. Setting to default 32." << std::endl;
            s_maxTextureUnits = 32; // 安全回退
        }
    }
};