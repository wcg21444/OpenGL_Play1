#include <array>
#include <iostream>

#include "Camera.hpp"
#include "GUI.hpp"
#include "InputHandler.hpp"
#include "LightSource/LightSource.hpp"
#include "ModelLoader.hpp"
#include "Objects/Cube.hpp"
#include "Objects/Grid.hpp"
#include "Objects/Object.hpp"
#include "Objects/Plane.hpp"
#include "Objects/Sphere.hpp"
#include "Objects/FrustumWireframe.hpp"
#include "Renderers/RendererManager.hpp"
#include "Shader.hpp"
#include "Utils/DebugOutput.hpp"

#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "imguizmo/ImGuizmo.h"

const int InitWidth = 1600;
const int InitHeight = 900;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    const char *glsl_version = "#version 460";
    // create window object
    GLFWwindow *window = glfwCreateWindow(InitWidth, InitHeight, "OpenGLPlay", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    InputHandler::BindWindow(window);

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

    // 窗口Flags设置
    // io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // 关闭multi viewport imguizmo 偏移bug会消失  ,原因是rect设置没有参照scene窗口  它会导致主窗口imguizmo偏移

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // 字体设置
    float fontSize = 16.f;
    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/CONSOLA.TTF", fontSize);
    io.Fonts->Build();

    std::cout << "ImGui Version: " << IMGUI_VERSION << std::endl;

    // 场景布置
    glm::mat4 model = glm::mat4(1.0f);
    Camera cam(InitWidth, InitHeight, 14.f, 0.05f);

    Scene scene;
    glm::mat4 plane_model = glm::translate(model, glm::vec3(0.f, 1.f, 0.f));
    glm::mat4 box_model = glm::translate(model, glm::vec3(3.f, 2.f, -4.f));
    glm::mat4 sphere_model = glm::translate(model, glm::vec3(6.f, 2.f, 2.f));
    glm::mat4 backPack_model = glm::translate(model, glm::vec3(0.f, 3.f, 4.f));
    glm::mat4 bass_model = glm::translate(model, glm::vec3(0.f, 4.f, 4.f));

    auto currentID = scene.addObject(std::make_unique<Cube>(glm::vec3(1.f, 1.f, 1.f)));
    scene.getObject(currentID).setModelTransform(box_model);

    currentID = scene.addObject(std::make_unique<Sphere>(1.f));
    scene.getObject(currentID).setModelTransform(sphere_model);

    currentID = scene.addObject(std::make_unique<Plane>(20.f, 20.f));
    scene.getObject(currentID).setModelTransform(plane_model);

    Lights allLights;
    auto &[pointLights, dirLights] = allLights;
    pointLights.emplace_back(glm::vec3(0.1f, 0.1f, 0.1f),
                             glm::vec3(8.f, 10.f, 4.f), 1024, 250.f);

    pointLights.emplace_back(glm::vec3(10.f, 30.f, 20.f),
                             glm::vec3(2.f, 10.f, 14.f), 1024, 250.f);
    pointLights.emplace_back(glm::vec3(30.f, 20.f, 40.f),
                             glm::vec3(16.f, 4.f, 8.f), 1024, 250.f);

    dirLights.emplace_back(
        DirectionLight(glm::vec3(1.0f), glm::vec3(50.f, 20.f, 10.f), 1024));

    // temporary light source variable
    PointLight &light = pointLights[0]; // Assuming the first light is the one we
                                        // want to use for shadow

    // 应用初始化
    auto ptrRenderParameters = std::make_shared<RenderParameters>(
        allLights,
        cam,
        scene,
        model,
        window);
    auto ptrRenderManager = std::make_shared<RenderManager>();

    InputHandler::BindRenderApplication(ptrRenderParameters, ptrRenderManager);

    GUI::BindRenderApplication(ptrRenderParameters, ptrRenderManager);

    //  main render loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        InputHandler::ProcessInput(window, cam);
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
        ImGuizmo::SetOrthographic(false);

        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockspace_flags);

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

        GUI::ShowSidebarToolbar(scene, *ptrRenderManager, allLights, model);

        if (GUI::modelLoadView)
        {
            GUI::ModelLoadView();
        }
        ModelLoader::run(scene);

        ptrRenderManager->render(ptrRenderParameters);

        scene.update();

        for(LightSource& light : allLights.dirLights){
            light.update();
        }
        for(LightSource& light : allLights.pointLights){
            light.update();
        }

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}