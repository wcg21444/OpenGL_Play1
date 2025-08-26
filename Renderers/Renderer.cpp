
#include "Renderer.hpp"

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

void Renderer::DrawSphere()
{
    // 静态变量，用于存储球体的VAO和顶点数量，确保只生成一次
    static unsigned int sphereVAO = 0;
    static unsigned int indexCount;

    // 如果球体VAO尚未生成，则进行初始化
    if (sphereVAO == 0)
    {
        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64; // 经度分段数
        const unsigned int Y_SEGMENTS = 64; // 纬度分段数
        const float PI = 3.14159265359f;

        // 生成顶点数据
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos)); // 球体的法线和位置向量相同
            }
        }

        // 生成索引数据（用于绘制三角形）
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            for (unsigned int x = 0; x < X_SEGMENTS; ++x)
            {
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x + 1);

                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y * (X_SEGMENTS + 1) + x + 1);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x + 1);
            }
        }

        indexCount = indices.size();

        // 绑定VAO，然后绑定并填充VBO和EBO
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3) + uv.size() * sizeof(glm::vec2) + normals.size() * sizeof(glm::vec3), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), &positions[0]);
        glBufferSubData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), uv.size() * sizeof(glm::vec2), &uv[0]);
        glBufferSubData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3) + uv.size() * sizeof(glm::vec2), normals.size() * sizeof(glm::vec3), &normals[0]);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // 设置顶点属性指针
        // 位置属性
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
        // 纹理坐标属性
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)(positions.size() * sizeof(glm::vec3)));
        // 法线属性
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)(positions.size() * sizeof(glm::vec3) + uv.size() * sizeof(glm::vec2)));
    }

    // 绘制球体
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}