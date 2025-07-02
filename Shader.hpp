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
    unsigned int progrm_ID;
    bool used = false;

private:
    std::string loadShaderFile(const char *shader_path)
    {
        // if success , return true
        std::fstream shader_file(shader_path);
        if (!shader_file.is_open()) // 文件可能因为各种原因(不存在,权限,打开方式等) 打不开
        {
            throw std::runtime_error("Failed to open shader file: " + std::string(shader_path));
        }
        std::stringstream shader_buffer;
        shader_buffer << shader_file.rdbuf();         // 将文件内容读入stringstream
        if (shader_file.fail() && !shader_file.eof()) // 文件可能因为各种原因读取不了
        {
            throw std::runtime_error("Failed to read shader file: " + std::string(shader_path));
        }

        shader_file.close();
        return shader_buffer.str();
    }

public:
    Shader() {}
    Shader(const char *vs_path, const char *fs_path, const char *gs_path = nullptr)
    {
        // Config Vertex Shader
        std::string shader_buf = loadShaderFile(vs_path);

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
        }

        // Config Fragment Shader
        shader_buf = loadShaderFile(fs_path);
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
        }

        unsigned int geometryShader;
        if (gs_path)
        { // Config Geometry Shader
            shader_buf = loadShaderFile(gs_path);
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
    void use()
    {
        glUseProgram(progrm_ID);
        used = true;
    }
    void setUniform4fv(const std::string &name, GLsizei count, float *value)
    {
        if (used)
        {
            GLuint location = glGetUniformLocation(progrm_ID, name.c_str());
            glUniform4fv(location, count, value);
        }
        else
        {
            throw std::runtime_error("Set Uniform while Shader is not used.");
        }
    }
    void setUniform4fv(const std::string &name, glm::vec4 vec4)
    {
        if (used)
        {
            GLuint location = glGetUniformLocation(progrm_ID, name.c_str());
            glUniform4fv(location, 1, glm::value_ptr(vec4));
        }
        else
        {
            throw std::runtime_error("Set Uniform while Shader is not used.");
        }
    }
    void setUniform3fv(const std::string &name, glm::vec3 vec3)
    {
        if (used)
        {
            GLuint location = glGetUniformLocation(progrm_ID, name.c_str());
            glUniform3fv(location, 1, glm::value_ptr(vec3));
        }
        else
        {
            throw std::runtime_error("Set Uniform while Shader is not used.");
        }
    }
    void setMat4(const std::string &name, glm::mat4 mat)
    {
        if (used)
        {
            GLuint location = glGetUniformLocation(progrm_ID, name.c_str());
            glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
        }
        else
        {
            throw std::runtime_error("Set Matrix4 while Shader is not used.");
        }
    }
    void setFloat(const std::string &name, float f)
    {
        if (used)
        {
            GLuint location = glGetUniformLocation(progrm_ID, name.c_str());
            glUniform1f(location, f);
        }
        else
        {
            throw std::runtime_error("Set Float while Shader is not used.");
        }
    }
    void setInt(const std::string &name, int i)
    {
        if (used)
        {
            GLuint location = glGetUniformLocation(progrm_ID, name.c_str());
            glUniform1i(location, i);
        }
        else
        {
            throw std::runtime_error("Set Int while Shader is not used.");
        }
    }
};