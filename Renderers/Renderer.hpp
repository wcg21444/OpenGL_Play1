#pragma once

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "includes/stb_image.h"

#include "Shader.hpp"
#include "Camera.hpp"
#include "InputHandler.hpp"
#include "LightSource.hpp"

#include "Objects/Object.hpp"
#include "Objects/Grid.hpp"
#include "Objects/Cube.hpp"
#include "Objects/Sphere.hpp"
#include "Objects/Plane.hpp"

void ShowGLMMatrixAsTable(const glm::mat4 &matrix, const char *name = "Matrix")
{
    if (ImGui::BeginTable(name, 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
    {
        for (int row = 0; row < 4; ++row)
        {
            ImGui::TableNextRow();
            for (int col = 0; col < 4; ++col)
            {
                ImGui::TableSetColumnIndex(col);
                ImGui::Text("%.3f", matrix[col][row]);
            }
        }
        ImGui::EndTable();
    }
}

void renderScene(std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model, Shader &shaders)
{
    glm::mat4 sphere_model = glm::translate(model, glm::vec3(6.f, 0.f, 0.f));
    glm::mat4 plane_model = glm::translate(model, glm::vec3(0.f, -1.f, 0.f));
    glm::mat4 backPack_model = glm::translate(model, glm::vec3(0.f, 2.f, 4.f));
    glm::mat4 bass_model = glm::translate(model, glm::vec3(0.f, 4.f, 4.f));
    bass_model = glm::scale(bass_model, glm::vec3(4.f, 4.f, 4.f));
    for (auto &&object : scene)
    {
        try
        {
            if (object->name == "Sphere")
                object->draw(sphere_model, shaders);
            else if (object->name == "Plane")
                object->draw(plane_model, shaders);
            else if (object->name == "Backpack")
            {
                object->draw(backPack_model, shaders);
            }
            else if (object->name == "Bass")
            {
                object->draw(bass_model, shaders);
            }
            else if (object->name == "Grid")
                continue;
            else
                object->draw(model, shaders);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error rendering object '" << object->name << "': " << e.what() << std::endl;
        }
    }
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

struct RenderParameters
{
    Lights &lights;
    Camera &cam;
    std::vector<std::unique_ptr<Object>> &scene;
    glm::mat4 &model;
    GLFWwindow *window;
};

class Renderer
{
public:
    virtual void contextSetup() = 0;
    virtual void render(RenderParameters &renderParameters) = 0;
    virtual void reloadCurrentShaders() = 0;
    virtual ~Renderer() {}
};
