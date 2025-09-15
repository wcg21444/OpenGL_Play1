#include "Shader.hpp"
#include "ShaderIncludes.hpp"
#include "../utils/Utils.hpp"

#define STATICIMPL

STATICIMPL [[nodiscard]] std::string ShaderBase::LoadShaderFile(const char *shader_path)
{
    return GLSLPT::ShaderInclude::load(shader_path).src;
}

STATICIMPL void ShaderBase::CompileShader(const char *shader_source, GLenum shader_type, unsigned int &shader_id, const char *path)
{
    shader_id = glCreateShader(shader_type);
    glShaderSource(shader_id, 1, &shader_source, NULL);
    glCompileShader(shader_id);

    int success;
    char infoLog[512];
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        DebugOutput::ExportShaderSource("shaderLogs/" + Utils::GetFilenameNoExtension(path) + "_shaderDump.glsl", shader_source); // Dump ����ʧ�ܵ���ɫ���ļ�
        glGetShaderInfoLog(shader_id, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED (" << shader_type << ")\n"
                  << infoLog << std::endl;
        glDeleteShader(shader_id); // �ͷ�GL��Դ
        throw std::runtime_error("Failed to compile shader.");
    }
}

STATICIMPL GLenum ShaderBase::GetTextureUnitEnum(int textureLocation)
{
    if (GetTextureUnitsLimits() == -1)
    {
        throw std::runtime_error("OpenGL texture limits not initialized.");
    }
    if (textureLocation < 0 || textureLocation >= GetTextureUnitsLimits())
    {
        std::string errorMsg = "Texture location " + std::to_string(textureLocation) +
                               " out of bounds. Max texture units: " + std::to_string(GetTextureUnitsLimits()) + ".";
        throw std::out_of_range(errorMsg);
    }
    return GL_TEXTURE0 + textureLocation;
}

STATICIMPL GLint ShaderBase::GetTextureUnitsLimits()
{
    static GLint s_maxTextureUnits = -1;
    if (s_maxTextureUnits == -1)
    {
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &s_maxTextureUnits);
        if (s_maxTextureUnits == -1)
        {
            std::cerr << "Warning: Could not query GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS. Setting to default 32." << std::endl;
            s_maxTextureUnits = 32;
        }
    }
    return s_maxTextureUnits;
}

// ���෽��,���ഫ��Uniform not found �ص�
[[nodiscard]] GLint ShaderBase::getUniformLocationSafe(const std::string &name, const std::function<void(const std::string &uniformName, GLuint programID)> &onNotFound)
{
    if (!used)
    {
        throw std::runtime_error("Attempted to set uniform '" + name + "' while shader is not active (glUseProgram was not called).");
    }
    if (uniformLocationMap.find(name) != uniformLocationMap.end()) // Location����
    {
        return uniformLocationMap.at(name);
    }
    GLint location = glGetUniformLocation(programID, name.c_str());
    if (location == -1 && !ignoreNotFoundWarning)
    {
        if (!warningMsgSet.contains(std::format("{}{}", name, programID)))
        {
            warningMsgSet.insert(std::format("{}{}", name, programID));
            onNotFound(name, programID);
        }
    }
    uniformLocationMap.insert({name, location});
    return location;
}

void ShaderBase::setUniform4fv(const std::string &name, GLsizei count, const float *value)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform4fv(location, count, value);
    }
}

void ShaderBase::setUniform4fv(const std::string &name, const glm::vec4 &vec4)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform4fv(location, 1, glm::value_ptr(vec4));
    }
}

void ShaderBase::setUniform3fv(const std::string &name, const glm::vec3 &vec3)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform3fv(location, 1, glm::value_ptr(vec3));
    }
}

void ShaderBase::setMat4(const std::string &name, const glm::mat4 &mat)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }
}

void ShaderBase::setFloat(const std::string &name, float f)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform1f(location, f);
    }
}

void ShaderBase::setInt(const std::string &name, int i)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform1i(location, i);
    }
}

void ShaderBase::setUniform(const std::string &name, const glm::vec4 &vec4)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform4fv(location, 1, glm::value_ptr(vec4));
    }
}

// Ϊ vec3 ��д������
void ShaderBase::setUniform(const std::string &name, const glm::vec3 &vec3)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform3fv(location, 1, glm::value_ptr(vec3));
    }
}

// Ϊ vec2 ��д������ (GLM vec2)
void ShaderBase::setUniform(const std::string &name, const glm::vec2 &vec2)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform2fv(location, 1, glm::value_ptr(vec2));
    }
}

// Ϊ mat4 ��д������
void ShaderBase::setUniform(const std::string &name, const glm::mat4 &mat)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }
}

// Ϊ float ��д������
void ShaderBase::setUniform(const std::string &name, float f)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform1f(location, f);
    }
}

// Ϊ int ��д������
void ShaderBase::setUniform(const std::string &name, int i)
{
    GLint location = getUniformLocationSafe(name);
    if (location != -1)
    {
        glUniform1i(location, i);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////
// Shader ��ʵ��

Shader::Shader() : texLocationID(0)
{
}

Shader::Shader(const char *vs_path, const char *fs_path, const char *gs_path) : Shader()
{
    this->vs_path = vs_path;
    this->fs_path = fs_path;
    this->gs_path = gs_path ? gs_path : "";
    bool hasGS = gs_path && gs_path[0] != '\0';

    std::string shader_buf;
    unsigned int vertexShader = 0;
    unsigned int fragmentShader = 0;
    unsigned int geometryShader = 0;

    // Config Vertex Shader
    try
    {
        shader_buf = LoadShaderFile(vs_path);
        const char *vertexShaderSource = shader_buf.c_str();
        CompileShader(vertexShaderSource, GL_VERTEX_SHADER, vertexShader, vs_path);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Shader Load Error (Vertex): " << e.what() << std::endl;
    }

    // Config Fragment Shader
    try
    {
        shader_buf = LoadShaderFile(fs_path);
        const char *fragmentShaderSource = shader_buf.c_str();
        CompileShader(fragmentShaderSource, GL_FRAGMENT_SHADER, fragmentShader, fs_path);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Shader Load Error (Fragment): " << e.what() << std::endl;
    }

    // Config Geometry Shader
    if (hasGS)
    {
        try
        {
            shader_buf = LoadShaderFile(gs_path);
            const char *geometryShadersource = shader_buf.c_str();
            CompileShader(geometryShadersource, GL_GEOMETRY_SHADER, geometryShader, gs_path);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Shader Load Error (Geometry): " << e.what() << std::endl;
        }
    }

    programID = linkShader(vertexShader, fragmentShader, geometryShader, hasGS);
}

// �ƶ���ֵ�����
Shader &Shader::operator=(Shader &&other) noexcept
{
    if (this != &other)
    {
        if (programID)
        {
            DebugOutput::AddLog("Shader Program ID:{} Was Deleted\n", programID);
            glDeleteProgram(programID);
        }
        this->programID = other.programID;
        this->used = other.used;
        this->vs_path = std::move(other.vs_path);
        this->fs_path = std::move(other.fs_path);
        this->gs_path = std::move(other.gs_path);
        this->textureLocationMap = std::move(other.textureLocationMap);
        this->uniformLocationMap = std::move(other.uniformLocationMap);
        this->warningMsgSet = std::move(other.warningMsgSet);
        this->texLocationID = other.texLocationID;
        other.programID = 0; // �ͷ�Դ������Դ
        other.used = false;
        other.texLocationID = 0;
    }
    return *this;
}

/**
 * @brief �Զ�������������ɫ����
 * * �˺���ּ�ڼ�����󶨹��̡����ڲ�ά��һ��ӳ��� (textureLocationMap)��
 * ����ɫ���еĲ����� (sampler) ������һ��Ψһ������Ԫ ID (texLocationID) ����������
 * ���һ���������ǵ�һ�ΰ󶨣����ᱻ����һ���µ�����Ԫ��
 * ��󣬺����ἤ���Ӧ������Ԫ�����������󶨵��õ�Ԫ��
 * ��ͨ�� glUniform1i ��������Ԫ ID ���ݸ���ɫ���е� uniform ������
 * * @param textureID Ҫ�󶨵��������� OpenGL ID��
 * @param textureTarget ����Ŀ�����ͣ����� GL_TEXTURE_2D��GL_TEXTURE_CUBE_MAP �ȡ�
 * @param shaderTextureLocation ռλ��
 * @param samplerUniformName ��ɫ���� sampler uniform ���������ƣ����� "u_AlbedoMap"��
 */
void Shader::setTextureAuto(GLuint textureID, GLenum textureTarget, int shaderTextureLocation, const std::string &samplerUniformName)
{
    // ��һ��Ϊ�ò����� uniform ������
    if (textureLocationMap.find(samplerUniformName) == textureLocationMap.end())
    {
        // ����������������Ψһ������Ԫ ID ��������
        textureLocationMap.insert({samplerUniformName, texLocationID});
        texLocationID++;
    }

    // ��ӳ����л�ȡ�ò�������Ӧ������Ԫ ID
    int location = textureLocationMap.at(samplerUniformName);

    // ������Ԫ ID ת��Ϊ GL_TEXTURE0��GL_TEXTURE1 ��ö��ֵ
    GLenum activeTextureUnit = GetTextureUnitEnum(location);

    glActiveTexture(activeTextureUnit);

    glBindTexture(textureTarget, textureID);

    GLint samplerLoc = getUniformLocationSafe(samplerUniformName);

    if (samplerLoc != -1)
    {
        // ������Ԫ ID (location) ���ݸ���ɫ���е� uniform ����
        // ������ɫ����֪��Ӧ�ô��ĸ�����Ԫ��ȡ����
        glUniform1i(samplerLoc, location);
    }
}

unsigned int Shader::linkShader(unsigned int vertexShader, unsigned int fragmentShader, unsigned int geometryShader, bool hasGS)
{
    // Cofig Shader Program
    unsigned int progrm_ID = glCreateProgram();
    glAttachShader(progrm_ID, vertexShader);
    glAttachShader(progrm_ID, fragmentShader);
    if (hasGS)
    {
        glAttachShader(progrm_ID, geometryShader);
    }
    glLinkProgram(progrm_ID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (hasGS)
    {
        glDeleteShader(geometryShader);
    }

    int success;
    char infoLog[512];
    glGetProgramiv(progrm_ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(progrm_ID, 512, NULL, infoLog);
        glDeleteProgram(progrm_ID);

        std::cerr << "VS : " << std::string(vs_path) << "FS : " << std::string(fs_path) << std::endl
                  << "ERROR::SHADER::PROGRAM::LINK_FAILED\n"
                  << infoLog << std::endl;

        throw std::runtime_error("Shader program link failed.");
    }
    return progrm_ID;
}

GLint Shader::getUniformLocationSafe(const std::string &name)
{
    return ShaderBase::getUniformLocationSafe(name,
                                              [&vs_path = this->vs_path,
                                               &fs_path = this->fs_path,
                                               &gs_path = this->gs_path](const std::string &_name, GLuint _program_ID) -> void
                                              {
                                                  DebugOutput::AddLog(std::format("<warning>Warning:</warning> Uniform <highlight>{}</highlight> not found in shader program: {} ", _name, _program_ID));
                                                  DebugOutput::AddLog(std::format("   Vertex Shader Path: {} ", vs_path));
                                                  DebugOutput::AddLog(std::format("   Fragment Shader Path: {} ", fs_path));
                                                  if (!(gs_path == ""))
                                                  {
                                                      DebugOutput::AddLog(std::format("   Geometry Shader Path: {} ", gs_path));
                                                  }
                                              });
}

/////////////////////////////////////////////////////////////////////////////////////////
// ComputeShader ��ʵ��
ComputeShader::ComputeShader(std::string cs_path)
{

    this->cs_path = cs_path;

    std::string shader_buf;
    unsigned int computeShader;

    // Compile Compute Shader
    try
    {
        shader_buf = LoadShaderFile(cs_path.c_str());
        const char *computeShaderSource = shader_buf.c_str();
        CompileShader(computeShaderSource, GL_COMPUTE_SHADER, computeShader, cs_path.c_str());
    }
    catch (const std::exception &e)
    {
        std::cerr << "Shader Load Error (Compute): " << e.what() << std::endl;
    }

    // Config Shader Program
    programID = glCreateProgram();
    glAttachShader(programID, computeShader);
    glLinkProgram(programID);

    int success;
    char infoLog[512];
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        std::cerr << "CS : " << std::string(cs_path) << std::endl;
        std::cerr << "ERROR::SHADER::PROGRAM::LINK_FAILED\n"
                  << infoLog << std::endl;
        throw std::runtime_error("Shader program link failed.");
    }
    glDeleteShader(computeShader);
}

ComputeShader &ComputeShader::operator=(ComputeShader &&other) noexcept
{
    if (this != &other)
    {
        if (programID)
        {
            std::cout << std::format("Shader Program ID:{} Was Deleted\n", programID);
            glDeleteProgram(programID);
        }
        // Move the resources from the other object to this one
        this->cs_path = std::move(other.cs_path);
        this->programID = other.programID;
        this->used = other.used;
        this->uniformLocationMap = std::move(other.uniformLocationMap);
        this->warningMsgSet = std::move(other.warningMsgSet);

        // Invalidate the other object
        other.programID = 0;
        other.used = false;
    }
    return *this;
}

GLint ComputeShader::getUniformLocationSafe(const std::string &name)
{
    return ShaderBase::getUniformLocationSafe(name,
                                              [&cs_path = this->cs_path](const std::string &_name, GLuint _program_ID) -> void
                                              {
                                                  DebugOutput::AddLog(std::format("<warning>Warning:</warning> Uniform <highlight>{}</highlight> not found in shader program: {} ", _name, _program_ID));
                                                  DebugOutput::AddLog(std::format("   Compute Shader Path: {} ", cs_path));
                                              });
}
