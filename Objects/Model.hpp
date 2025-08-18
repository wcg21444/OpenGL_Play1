#pragma once

#include "Mesh.hpp"
#include "Object.hpp"

class Model : public Object
{
public:
    Model(const std::string _name = "Model");
    void spawnMesh();
    void draw(glm::mat4 modelMatrix, Shader &shaders) override;
    std::vector<Mesh> meshes;
};
