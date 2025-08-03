
#include "Renderer.hpp"

void ShowGLMMatrixAsTable(const glm::mat4 &matrix, const char *name)
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

// 绘制场景
void Renderer::DrawScene(std::vector<std::unique_ptr<Object>> &scene, glm::mat4 &model, Shader &shaders)
{

    for (auto &&object : scene)
    {
        try
        {
            glm::mat4 obj_model = model * object->modelMatrix;
            object->draw(obj_model, shaders);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error rendering object '" << object->name << "': " << e.what() << std::endl;
        }
    }
}

// 生成Quad并注册到OpenGL. [out]quadVAO,quadVBO
void Renderer::GenerateQuad(unsigned int &quadVAO, unsigned int &quadVBO)
{
    static float quadVertices[] = {
        // positions       // texCoords
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f};
    // setup plane VAO
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
}

// 绘制公共Quad .单一职责:不负责视口管理.
void Renderer::DrawQuad()
{
    static float quadVertices[] = {
        // positions       // texCoords
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f};

    // Setup 渲染过程是串行的,所有pass渲染共用一个quad资源没问题
    static unsigned int quadVAO;
    static unsigned int quadVBO;
    static bool initialized = false;
    if (!initialized)
    {
        Renderer::GenerateQuad(quadVAO, quadVBO);
        initialized = true;
    }
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
    glBindVertexArray(0);
}
