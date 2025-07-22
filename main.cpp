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
#include "DebugOutput.hpp"
#include "ModelLoader.hpp"
#include "GUI.hpp"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

const int width = 1600;
const int height = 900;

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
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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
    // ModelLoader::loadFile("Resource/backpack.obj");

    LightSource light(glm::vec3(20.f), glm::vec3(0.f, 5.f, 4.f));

    RenderParameters renderParameters{light, cam, scene, model, window};

    RenderManager renderManager;

    //  main render loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        InputHandler::processInput(window, cam);
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::BeginMainMenuBar();
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Import"))
                {
                    GUI::modelLoadView = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        GUI::ShowSidebarToolbar(scene, renderManager, light, model);

        if (GUI::modelLoadView)
        {
            GUI::ModelLoadView();
        }
        ModelLoader::run(scene);
        // 渲染顺序
        renderManager.render(renderParameters);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}