#pragma once

#include "Object.hpp"

class Mesh : public Object
{
public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };
    struct Texture
    {
        unsigned int id;
        std::string type;
        std::string path;
    };

public:
    // mesh data
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        setupMesh();
    }
    void draw(glm::mat4 modelMatrix, Shader &shaders)
    {
        // TODO: setup shaders to render Material
        //  draw mesh
        glBindVertexArray(VAO);
        // Easy texture vector protect
        if (textures.size() >= 1)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, textures[0].id); // diffuse texture
            shaders.setInt("texture_diff", 1);
        }
        if (textures.size() >= 2)
        {
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, textures[1].id); // specular texture
            shaders.setInt("texture_spec", 2);
        }
        // 存在贴图,才开启贴图着色
        if (textures.size() >= 1)
        {
            shaders.setInt("enable_tex", 1);
        }
        shaders.setMat4("model", modelMatrix);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        shaders.setInt("enable_tex", 0);
        glBindVertexArray(0);
    }

private:
    //  render data
    GLuint VAO, VBO, EBO;

    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                     &indices[0], GL_STATIC_DRAW);

        // vertex positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));

        glBindVertexArray(0);
    }
};