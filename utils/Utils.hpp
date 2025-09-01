#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <filesystem>

namespace Utils
{
    inline void CheckGLErrors()
    {
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            std::cout << "OpenGL error: " << err << std::endl;
        }
    }
    inline std::string GetFilenameNoExtension(const std::string &path_str)
    {
        std::filesystem::path p(path_str);
        return p.stem().string();
    }
    inline bool CreateParentDirectories(const std::string &filepath)
    {
        std::filesystem::path p(filepath);
        std::filesystem::path parent_dir = p.parent_path();

        if (!parent_dir.empty() && !std::filesystem::exists(parent_dir))
        {
            try
            {
                std::filesystem::create_directories(parent_dir);
                // std::cerr << "Created directory: " << parent_dir << std::endl;
                return true;
            }
            catch (const std::filesystem::filesystem_error &e)
            {
                std::cerr << "Failed to create directory " << parent_dir << ": " << e.what() << std::endl;
                return false;
            }
        }
        return true; // 目录已存在或路径无父目录
    }
}
