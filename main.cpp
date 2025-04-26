#include <iostream>
#include <array>

#include "Shader.hpp"
#include "Camera.hpp"
#include "InputHandler.hpp"
#include "Objects/Object.hpp"
#include "Objects/Grid.hpp"
#include "Objects/Cube.hpp"
#include "Objects/Sphere.hpp"
#include "Objects/Plane.hpp"
#include "LightSource.hpp"
#include "Renderer.hpp"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

const int width = 1600;
const int height = 900;

namespace GUI
{
    void lightHandle(LightSource &light_source)
    {
        static glm::vec3 lightIntensity = light_source.intensity;
        static glm::vec3 lightPosition = light_source.position;
        ImGui::DragFloat("LightIntensitiy.X", &lightIntensity.x, 1.f);
        ImGui::DragFloat("LightIntensitiy.Y", &lightIntensity.y, 1.f);
        ImGui::DragFloat("LightIntensitiy.Z", &lightIntensity.z, 1.f);

        ImGui::DragFloat("LightPosition.X", &lightPosition.x, 0.1f);
        ImGui::DragFloat("LightPosition.Y", &lightPosition.y, 0.1f);
        ImGui::DragFloat("LightPosition.Z", &lightPosition.z, 0.1f);
        light_source.intensity = lightIntensity;
        light_source.position = lightPosition;
    }

    glm::mat4 MatrixInputWithImGui(const char *label, const glm::mat4 &initial)
    {
        // TODO : 将矩阵转置
        //  将GLM矩阵转换为ImGui可编辑的格式
        std::array<std::array<float, 4>, 4> matrix;
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                matrix[i][j] = initial[i][j];
            }
        }

        // ImGui矩阵编辑器
        if (ImGui::TreeNodeEx(label, ImGuiTreeNodeFlags_DefaultOpen))
        {

            // 逐行编辑
            for (int row = 0; row < 4; ++row)
            {
                ImGui::PushID(row);
                if (ImGui::DragFloat4("", matrix[row].data(), 0.01f))
                {
                    ///
                }
                ImGui::PopID();
            }

            // 重置按钮
            if (ImGui::Button("Reset to Identity"))
            {
                for (auto &row : matrix)
                {
                    std::fill(row.begin(), row.end(), 0.0f);
                }
                for (int i = 0; i < 4; ++i)
                {
                    matrix[i][i] = 1.0f;
                }
            }

            ImGui::TreePop();
        }

        // 转换回GLM矩阵
        glm::mat4 result;
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                result[i][j] = matrix[i][j];
            }
        }
        return result;
    }

    void RenderSwitchCombo(RenderManager &renderManager)
    {
        static const char *modes[] = {"Normal", "Debug_Depth", "Texture"};
        static int current_mode = 0;
        static int prev_mode = current_mode;
        ImGui::Combo("Mode", &current_mode, modes, IM_ARRAYSIZE(modes));

        if (current_mode != prev_mode)
        {
            prev_mode = current_mode;
            switch (current_mode)
            {
            case 0:
                renderManager.switchMode(RenderManager::Mode::normal);
                break;
            case 1:
                renderManager.switchMode(RenderManager::Mode::debug_depth);

                break;
            case 2:
                renderManager.switchMode(RenderManager::Mode::simple_texture);
                break;
            }
        }
    }

    void displaySceneHierarchy(std::vector<std::unique_ptr<Object>> &scene, int &selectedIndex)
    {
        ImGui::Begin("Scene Hierarchy");

        // 列表显示所有对象
        for (int i = 0; i < scene.size(); ++i)
        {
            bool isSelected = (selectedIndex == i);
            if (ImGui::Selectable(scene[i]->name.c_str(), isSelected))
            {
                selectedIndex = i;
            }

            // 右键菜单
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Delete"))
                {
                    if (selectedIndex == i)
                        selectedIndex = -1;
                    scene.erase(scene.begin() + i);
                    ImGui::EndPopup();
                    break;
                }
                ImGui::EndPopup();
            }
        }

        // ImGui::End();

        // // 属性面板
        // ImGui::Begin("Inspector");
        // if (selectedIndex >= 0 && selectedIndex < scene.size())
        // {
        //     scene[selectedIndex]->displayInInspector();
        // }
        // else
        // {
        //     ImGui::Text("No object selected");
        // }
        ImGui::End();
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    const char *glsl_version = "#version 130";
    // create window object
    GLFWwindow *window = glfwCreateWindow(width, height, "OpenGLPlay", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    InputHandler::bindWindow(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // 场景布置
    glm::mat4 model = glm::mat4(1.0f);
    Camera cam(width, height, 14.f, 0.05f);

    std::vector<std::unique_ptr<Object>> scene;
    scene.push_back(std::make_unique<Grid>());
    scene.push_back(std::make_unique<Cube>(glm::vec3(1.f, 1.f, 1.f)));
    scene.push_back(std::make_unique<Sphere>(1.f));
    scene.push_back(std::make_unique<Plane>(200.f, 200.f));

    LightSource light(glm::vec3(200.f), glm::vec3(0.f, 0.f, 40.f));

    RenderManager renderManager;

    int selectedIndex = -1;
    //  main render loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        InputHandler::processInput(window, cam);
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("UI");

        // 草草抽离出了IMGUI逻辑

        GUI::lightHandle(light);

        model = GUI::MatrixInputWithImGui("Model Matrix", model);
        GUI::RenderSwitchCombo(renderManager);
        GUI::displaySceneHierarchy(scene, selectedIndex);
        ImGui::End();
        ImGui::Render();

        renderManager.render(light, cam, scene, model, window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}