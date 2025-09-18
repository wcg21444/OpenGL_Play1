#pragma once
#include "Pass.hpp"
#include "../Shading/Texture.hpp"
#include "../Shading/RenderTarget.hpp"

class Texture2DArrayTestPass : public Pass
{
private:
    Texture2D textureReadR;
    Texture2D textureReadG;
    Texture2D textureReadB;
    std::shared_ptr<Texture2DArray> texture2DArray = nullptr;
    RenderTarget renderTargetWrite;
    RenderTarget renderTargetRead;

    Shader writeShaders;
    Shader readShaders;

private:
    void initializeGLResources() override
    {
        textureReadR.generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
        textureReadG.generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
        textureReadB.generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
        texture2DArray->generate(vp_width, vp_height, 3, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
    }
    void cleanUpGLResources() override {}

public:
    Texture2DArrayTestPass(int _vp_width, int _vp_height, std::string _vs_path,
                           std::string _fs_path) : Pass(_vp_width, _vp_height, _vs_path, _fs_path),
                                                   renderTargetWrite(_vp_width, _vp_height),
                                                   renderTargetRead(_vp_width, _vp_height)
    {
        writeShaders = Shader("Shaders/screenQuad.vs", "Shaders/Texture2DArray/write.fs");
        readShaders = Shader("Shaders/screenQuad.vs", "Shaders/Texture2DArray/read.fs");
        texture2DArray = std::make_shared<Texture2DArray>();
        initializeGLResources();
        contextSetup();
    }
    ~Texture2DArrayTestPass() { cleanUpGLResources(); }

    void reloadCurrentShaders() override
    {
        Pass::reloadCurrentShaders();
        writeShaders = Shader("Shaders/screenQuad.vs", "Shaders/Texture2DArray/write.fs");
        readShaders = Shader("Shaders/screenQuad.vs", "Shaders/Texture2DArray/read.fs");
    }

    void contextSetup() override
    {
        renderTargetRead.bind();
        {
            renderTargetRead.attachColorTexture2D(textureReadR.ID, GL_COLOR_ATTACHMENT0);
            renderTargetRead.attachColorTexture2D(textureReadG.ID, GL_COLOR_ATTACHMENT1);
            renderTargetRead.attachColorTexture2D(textureReadB.ID, GL_COLOR_ATTACHMENT2);
            renderTargetRead.enableColorAttachments();
            renderTargetRead.checkStatus();
        }
        renderTargetRead.unbind();
    }
    void resize(int _width, int _height) override
    {
        vp_width = _width;
        vp_height = _height;
        textureReadR.resize(_width, _height);
        textureReadG.resize(_width, _height);
        textureReadB.resize(_width, _height);
        texture2DArray->resize(_width, _height);
        contextSetup();
    }
    auto getTextures() { return std::make_tuple(textureReadR.ID, textureReadG.ID, textureReadB.ID); }
    auto getTextureArrayID() { return std::make_tuple(texture2DArray->ID, texture2DArray->Depth); }
    auto getTextureArray() { return texture2DArray; }
    void render()
    {

        renderTargetWrite.bind();
        renderTargetWrite.setViewport();

        writeShaders.use();
        for (size_t i = 0; i < 3; ++i)
        {
            renderTargetWrite.attachColorTexture2DArray(texture2DArray->ID, GL_COLOR_ATTACHMENT0, i);
            renderTargetWrite.clearBuffer(GL_COLOR_BUFFER_BIT, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

            writeShaders.setInt("width", vp_width);
            writeShaders.setInt("height", vp_height);
            writeShaders.setInt("layer", i);

            Renderer::DrawQuad();
        }
        renderTargetWrite.unbind();

        renderTargetRead.bind();
        renderTargetRead.setViewport();
        renderTargetRead.clearBuffer(GL_COLOR_BUFFER_BIT, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        readShaders.use();

        readShaders.setInt("width", vp_width);
        readShaders.setInt("height", vp_height);
        readShaders.setTextureAuto(texture2DArray->ID, GL_TEXTURE_2D_ARRAY, 0, "textureArray");
        Renderer::DrawQuad();

        renderTargetRead.unbind();
    }
};
