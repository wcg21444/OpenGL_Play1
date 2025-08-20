#pragma once

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include <unordered_map>
#include <string>
#include <filesystem>
#include <future>
#include <thread>
#include <atomic>
#include <vector>

#include "Utils/DebugOutput.hpp"
#include "../Utils/TextureLoader.hpp"
#include "Model.hpp"

class ModelLoader
{
private:
    inline static GLuint TextureFromFile(const char *file, const char *directory)
    {
        std::string texture_path = std::string(directory) + std::string(file);
        if (tex_file_id.find(texture_path) != tex_file_id.end())
        {
            return tex_file_id[texture_path];
        }
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        static int width, height, nrChannels;
        // stbi_set_flip_vertically_on_load(true);

        unsigned char *data = stbi_load(texture_path.c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }

        stbi_image_free(data);

        tex_file_id.insert(std::pair<std::string, GLuint>(texture_path, texture));
        return texture;
    }
    inline static std::vector<Mesh::Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
    {
        // in Mat
        // do : load texture resources to GL
        // out textures
        std::vector<Mesh::Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            Mesh::Texture texture;
            DebugOutput::AddLog("texture:{}", str.C_Str());
            std::string direction = current_file_path.parent_path().string() + "/";
            texture.id = TextureFromFile(str.C_Str(), direction.c_str());
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
        }
        return textures;
    }
    inline static Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        std::vector<Mesh::Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Mesh::Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Mesh::Vertex vertex;
            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.position = vector;
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;

            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates? //默认不同纹理,纹理坐标相同,所以取0
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.texCoord = vec;
            }
            else
                vertex.texCoord = glm::vec2(0.0f, 0.0f);
            vertices.push_back(vertex);
        }
        // process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
            std::vector<Mesh::Texture> diffuseMaps = loadMaterialTextures(material,
                                                                          aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            std::vector<Mesh::Texture> specularMaps = loadMaterialTextures(material,
                                                                           aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }

        DebugOutput::AddLog("Successfully ProcessMesh vertices:{},indices:{},textures:{}\n", vertices.size(), indices.size(), textures.size());
        return Mesh(vertices, indices, textures);
    }

    /* [in]: scene : Obj file imported in memory
     *  [out]: ptr Model : Model object, which is loaded to OpenGL context
     *  [process]: OpenGL Object Binding needs synchrours operation
     */
    inline static std::unique_ptr<Model> postProcess(const aiScene &scene)
    {
        DebugOutput::AddLog("nums of Children of Root Node:{}\n", scene.mRootNode->mNumChildren);
        std::unique_ptr<Model> model = std::make_unique<Model>();
        for (int i = 0; i < scene.mRootNode->mNumChildren; ++i)
        {
            // DebugOutput::AddLog("nums of Children of Node{}:{}\n", i, scene.mRootNode->mChildren[i]->mNumChildren);
            auto &node = *(scene.mRootNode->mChildren[i]);
            // node.mNumMeshes
            DebugOutput::AddLog("nums of Meshes of Node{}:{}\n", i, node.mNumMeshes);
            auto &mesh_id = node.mMeshes[0];
            auto &mesh = scene.mMeshes[mesh_id];
            // mesh->mName
            DebugOutput::AddLog("Name of Meshes of Node{}:{}\n", i, std::string(mesh->mName.C_Str()));
            auto &mat_id = mesh->mMaterialIndex;
            auto &mat = scene.mMaterials[mat_id];
            DebugOutput::AddLog("Name of Materials of Node{}:{}\n", i, std::string(mat->GetName().C_Str()));
            DebugOutput::AddLog("Nums of DIFFUSE of Node{}:{}\n", i, mat->GetTextureCount(aiTextureType_DIFFUSE));
            DebugOutput::AddLog("Nums of AMBIENT of Node{}:{}\n", i, mat->GetTextureCount(aiTextureType_AMBIENT));
            DebugOutput::AddLog("Nums of SPECULAR of Node{}:{}\n", i, mat->GetTextureCount(aiTextureType_SPECULAR));

            model->meshes.emplace_back(processMesh(mesh, &scene));
        }
        return model;
    }

public:
    using PtrImporter = std::unique_ptr<Assimp::Importer>;
    using ModelLoadFuture = std::future<std::pair<const aiScene *, PtrImporter>>;
    struct ImportingContext
    {
        ModelLoadFuture model_future;
        std::string file_path;
    };
    inline static std::unordered_map<std::string, GLuint> tex_file_id;
    inline static std::filesystem::path current_file_path;
    inline static std::vector<ImportingContext> importing_vec;

public:
    ModelLoader()
    {
    }
    ModelLoadFuture inline static LoadModelAsync(const std::string &path)
    {
        // aiScene 必须在 Importer 上下文环境才有效
        // aiScene 生命周期必须与 Importer 一致
        return std::async(
            std::launch::async, [path]
            {   
                auto importer = std::make_unique<Assimp::Importer>();
                const aiScene* scene = importer->ReadFile(
                    path,
                    aiProcess_CalcTangentSpace |
                    aiProcess_Triangulate |
                    aiProcess_JoinIdenticalVertices |
                    aiProcess_SortByPType);
                
                if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
                    throw std::runtime_error("Load Failed: " + std::string(importer->GetErrorString()));
                }
                return std::make_pair(scene, std::move(importer)); });
    }

    // 发送加载模型请求
    inline static void loadFile(const std::string &pFile)
    {
        importing_vec.emplace_back(ImportingContext{LoadModelAsync(pFile), pFile});
    }

    // TODO 异常处理优化 ; 进度输出;
    /*
    [in]: scene 场景对象
    importing_vec 中将模型加载器加载好的文件 处理 并 加入到 scene 中
    */
    inline static void run(Scene &scene)
    {
        auto it = importing_vec.begin();
        while (it != importing_vec.end())
        {
            if (it->model_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
            {
                current_file_path = std::filesystem::path(it->file_path);
                LoadAndProcessModel(scene, it->model_future);
                it = importing_vec.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    inline static void LoadAndProcessModel(Scene &scene, ModelLoadFuture &model_future)
    {
        try
        {
            auto [raw_model, importer] = model_future.get();
            outputRawModelDebugInfos(raw_model);
            auto &&model = postProcess(*raw_model);
            scene.push_back(std::move(model));
        }
        catch (std::exception &e)
        {
            std::cout << e.what() << std::endl;
        }
    }
    inline static void outputRawModelDebugInfos(const aiScene *raw_model)
    {
        if (raw_model->HasTextures())
            DebugOutput::AddLog("Raw Model has Textures\n");
        if (raw_model->HasMaterials())
            DebugOutput::AddLog("Raw Model has Materials\n");
        if (raw_model->HasMeshes())
            DebugOutput::AddLog("Raw Model has Meshes\n");
        if (raw_model->HasCameras())
            DebugOutput::AddLog("Raw Model has Cameras\n");
        if (raw_model->HasLights())
            DebugOutput::AddLog("Raw Model has Lights\n");
    }
};
