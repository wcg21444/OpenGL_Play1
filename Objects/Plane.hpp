#include "Shader.hpp"

class Plane
{
private:
    std::vector<float> vertices;
    std::vector<float> indices;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

private:
    void createPlaneVertices(float size, int divisions, int subdivisions)
    {
        float halfSize = size / 2.0f;
        float step = size / divisions;

        // 计算每个顶点的属性数量
        int attribCount = 3 + 3 + 2; // 位置(x,y,z) 法线, UV

        // 生成顶点数据
        for (int z = 0; z <= divisions; z++)
        {
            for (int x = 0; x <= divisions; x++)
            {
                // 位置
                vertices.push_back(-halfSize + x * step); // x
                vertices.push_back(0.0f);                 // y
                vertices.push_back(-halfSize + z * step); // z

                vertices.push_back(0.0f); // nx
                vertices.push_back(1.0f); // ny
                vertices.push_back(0.0f); // nz

                vertices.push_back(static_cast<float>(x) / divisions); // u
                vertices.push_back(static_cast<float>(z) / divisions); // v
            }
        }

        // 生成索引数据
        for (int z = 0; z < divisions; z++)
        {
            for (int x = 0; x < divisions; x++)
            {
                int topLeft = z * (divisions + 1) + x;
                int topRight = topLeft + 1;
                int bottomLeft = (z + 1) * (divisions + 1) + x;
                int bottomRight = bottomLeft + 1;

                // 第一个三角形
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                // 第二个三角形
                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }
    }

public:
    Plane(float size, int divisions, int subdivisions)
    {
        createPlaneVertices(size, divisions, subdivisions);
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        int stride = (3 + 3 + 2) * sizeof(float); // 位置
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void *)(3 * sizeof(float)));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *)(6 * sizeof(float)));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
    }
    void draw(glm::mat4 modelMatrix, Shader &shaders)
    {
        glBindVertexArray(vao);
        shaders.setMat4("model", modelMatrix);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }
};