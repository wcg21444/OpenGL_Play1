#include <iostream>
#include <array>

#include "Shader.hpp"
#include "Camera.hpp"
#include "InputHandler.hpp"
#include "Objects/Grid.hpp"
#include "Objects/Cube.hpp"
#include "Objects/Sphere.hpp"
#include "Objects/Plane.hpp"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

const int width = 1600;
const int height = 900;

void renderShadow()
{
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE); // avoid rendering color
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 配置光源空间的投影 视图 矩阵
    float near_plane = 1.0f, far_plane = 7.5f;
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f),
                                      glm::vec3(0.0f, 0.0f, 0.0f),
                                      glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    Shader shadowShader("Shadow.vs", "Shadow.fs");
    shadowShader.use();
    shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    // RenderScene(shadowShader);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void lightHandle(Shader &normal_shaders)
{
    static glm::vec3 lightIntensity(100.f);
    static glm::vec3 lightPosition(10.f);
    ImGui::DragFloat("LightIntensitiy.X", &lightIntensity.x, 1.f);
    ImGui::DragFloat("LightIntensitiy.Y", &lightIntensity.y, 1.f);
    ImGui::DragFloat("LightIntensitiy.Z", &lightIntensity.z, 1.f);
    normal_shaders.setUniform3fv("light_intensity", lightIntensity);

    ImGui::DragFloat("LightPosition.X", &lightPosition.x, 0.1f);
    ImGui::DragFloat("LightPosition.Y", &lightPosition.y, 0.1f);
    ImGui::DragFloat("LightPosition.Z", &lightPosition.z, 0.1f);
    normal_shaders.setUniform3fv("light_pos", lightPosition);
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

    // 场景布置
    Camera cam(width, height, 14.f, 0.05f);

    glm::mat4 model = glm::mat4(1.0f);

    Grid grid;
    Cube cube(glm::vec3(1.f, 1.f, 1.f));
    Sphere sphere(1.f);
    Plane plane(10.f, 1, 1);

    // set viewport
    glViewport(0, 0, width, height);
    Shader shaders("VertShader.vs", "FragmShader.fs");
    shaders.use();
    //  main render loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("UI");

        // 草草抽离出了IMGUI逻辑

        lightHandle(shaders);

        model = MatrixInputWithImGui("Model Matrix", model);

        ImGui::End();

        InputHandler::processInput(window, cam);

        // camera/view transformation
        cam.setViewMatrix(shaders);
        cam.setPerspectiveMatrix(shaders, width, height);

        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 深度缓冲和Color缓冲一样需要交换链,清理他

        // doing render thing

        {
            grid.draw(model, shaders);

            plane.draw(model, shaders);

            cube.draw(model, shaders);

            glm::mat4 sphere_model = glm::translate(model, glm::vec3(2.f, 0.f, 0.f));
            sphere.draw(sphere_model,
                        shaders);

            sphere_model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
            sphere_model = glm::translate(sphere_model, glm::vec3(0.f, 0.f, 2.f));
            sphere.draw(sphere_model,
                        shaders);
        }
        glEnable(GL_DEPTH_TEST); // 深度缓冲
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