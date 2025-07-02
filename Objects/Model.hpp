#pragma once

#include "Mesh.hpp"
#include "Object.hpp"

class Model : public Object
{
private:
public:
    Model(const std::string _name = "Model")
    {
        setName(_name);
    }
    void spawnMesh()
    {
    }
    void draw(glm::mat4 modelMatrix, Shader &shaders)
    {
        for (auto &mesh : meshes)
        {
            mesh.draw(modelMatrix, shaders /*mesh_shaders*/);
        }
    }

public:
    std::vector<Mesh> meshes;
};
