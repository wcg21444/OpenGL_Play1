#include "Model.hpp"
#include <glm/glm.hpp>
#include <vector>

Model::Model(const std::string _name)
{
    setName(_name);
}

void Model::spawnMesh()
{
    // 实现可选
}

void Model::draw(glm::mat4 modelMatrix, Shader &shaders)
{
    for (auto &mesh : meshes)
    {
        mesh.draw(modelMatrix, shaders);
    }
}
