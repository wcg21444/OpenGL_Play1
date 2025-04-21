#include <iostream>

#include "Shader.hpp"
#include "Camera.hpp"
#include "InputHandler.hpp"
#include "Objects/Grid.hpp"
#include "Objects/Cube.hpp"
#include "Objects/Sphere.hpp"

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

    // 初始为程序控制模式
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // initalize GLAD , which manages all of the func ptr of OpenGL
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

    // set viewport
    glViewport(0, 0, width, height);

    Camera cam(width, height, 14.f, 0.05f);

    Shader shaders("VertShader.vs", "FragmShader.fs");
    shaders.use();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(0.f), glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

    shaders.setMat4("model", model);

    Grid grid;
    Cube cube(glm::vec3(1.f, 1.f, 1.f));
    Sphere sphere(1.f);

    glm::vec3 lightIntensity(100.f);
    glm::vec3 lightPosition(10.f);

    float matrix[4][4] = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}};
    //  main render loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("UI");

        ImGui::DragFloat("LightIntensitiy.X", &lightIntensity.x, 1.f);
        ImGui::DragFloat("LightIntensitiy.Y", &lightIntensity.y, 1.f);
        ImGui::DragFloat("LightIntensitiy.Z", &lightIntensity.z, 1.f);
        shaders.setUniform3fv("light_intensity", lightIntensity);

        ImGui::DragFloat("LightPosition.X", &lightPosition.x, 0.1f);
        ImGui::DragFloat("LightPosition.Y", &lightPosition.y, 0.1f);
        ImGui::DragFloat("LightPosition.Z", &lightPosition.z, 0.1f);
        shaders.setUniform3fv("light_pos", lightPosition);

        ImGui::Text("Global Model Matrix Input");
        ImGui::Separator();

        // 逐行输入
        for (int row = 0; row < 4; ++row)
        {
            ImGui::PushID(row);
            if (ImGui::DragFloat4("", matrix[row], 0.01f))
            {
                // 矩阵数据已修改，可以在这里触发更新
            }
            ImGui::PopID();
        }
        if (ImGui::Button("Reset to Identity"))
        {
            memset(matrix, 0, sizeof(matrix));
            for (int i = 0; i < 4; ++i)
                matrix[i][i] = 1.0f;
        }

        ImGui::End();

        InputHandler::processInput(window, cam);

        // camera/view transformation
        cam.setViewMatrix(shaders);
        cam.setPerspectiveMatrix(shaders, width, height);

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 深度缓冲和Color缓冲一样需要交换链,清理他

        model = glm::make_mat4(&matrix[0][0]);
        model = glm::transpose(model);

        // doing render thing
        grid.draw(model, shaders);

        cube.draw(model, shaders);

        glm::mat4 sphere_model = glm::translate(model, glm::vec3(2.f, 0.f, 0.f));
        sphere.draw(sphere_model,
                    shaders);

        sphere_model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        sphere_model = glm::translate(sphere_model, glm::vec3(0.f, 0.f, 2.f));
        sphere.draw(sphere_model,
                    shaders);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glEnable(GL_DEPTH_TEST); // 深度缓冲

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}