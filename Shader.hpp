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

#include "DebugOutput.hpp"

class Shader
{

public:
    std::string vs_path;
    std::string fs_path;
    std::string gs_path;
    unsigned int progrm_ID;
    bool used = false;

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

public:
    Shader() {}
    Shader(const char *vs_path, const char *fs_path, const char *gs_path = nullptr)
    {
        this->vs_path = vs_path;
        this->fs_path = fs_path;
        this->gs_path = gs_path ? gs_path : "";
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
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
            throw std::runtime_error("Failed to setup shader : " + std::string(vs_path));
        }

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
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
            throw std::runtime_error("Failed to setup shader : " + std::string(fs_path));
        }

        unsigned int geometryShader;
        if (gs_path)
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

            geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometryShader, 1, &geometryShadersource, NULL);
            glCompileShader(geometryShader);

            glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
                std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n"
                          << infoLog << std::endl;
                throw std::runtime_error("Failed to setup shader : " + std::string(gs_path));
            }
        }
        // Cofig Shader Program
        progrm_ID = glCreateProgram();

        glAttachShader(progrm_ID, vertexShader);
        glAttachShader(progrm_ID, fragmentShader);
        if (gs_path)
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
        if (gs_path)
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
};