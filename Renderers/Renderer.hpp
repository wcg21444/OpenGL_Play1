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
