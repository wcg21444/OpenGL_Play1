#pragma once
#include "Pass.hpp"
#include "../Shading/Texture.hpp"
#include "../Shading/RenderTarget.hpp"

class TextureArrayUnfoldPass : public Pass
{
private:
    RenderTarget renderTarget;
    std::vector<Texture2D> unfoldTextures;

private:
    void initializeGLResources() override
    {
    }
    void cleanUpGLResources() override {}

public:
    TextureArrayUnfoldPass(int _vp_width, int _vp_height, std::string _vs_path,
                           std::string _fs_path) : Pass(_vp_width, _vp_height, _vs_path, _fs_path),
                                                   renderTarget(_vp_width, _vp_height)
    {
        
        initializeGLResources();
        contextSetup();
    }
    ~TextureArrayUnfoldPass() { cleanUpGLResources(); }

    void contextSetup() override
    {
    }
    void resize(int _width, int _height) override
    {
        vp_width = _width;
        vp_height = _height;
        contextSetup();
        // Do nothing
    }
    std::vector<TextureID> getTexturesID()
    {
        std::vector<TextureID> unfoldTexIDs;
        for (const auto &tex : unfoldTextures)
        {
            unfoldTexIDs.push_back(tex.ID);
        }
        return unfoldTexIDs;
        return {};
    }
    void render(Texture2DArray &texture2DArray)
    {

        renderTarget.bind();
        shaders.use();

        unfoldTextures.resize(texture2DArray.Depth);
        for (auto &tex : unfoldTextures)
        {
            if (!tex.ID)
            {
                tex.generate(vp_width, vp_height, GL_RGBA16F, GL_RGBA, GL_FLOAT, NULL);
            }
        }
        for (int i = 0; i < texture2DArray.Depth; ++i)
        {
            renderTarget.setViewport();

            renderTarget.attachColorTexture2D(unfoldTextures[i].ID, GL_COLOR_ATTACHMENT0);
            renderTarget.clearBuffer(GL_COLOR_BUFFER_BIT, glm::vec4(0.7f, 0.0f, 0.0f, 1.0f));

            shaders.setUniform("layer", i);
            shaders.setInt("width", vp_width);
            shaders.setInt("height", vp_height);
            shaders.setTextureAuto(texture2DArray.ID, GL_TEXTURE_2D_ARRAY, 0, "textureArray");
            Renderer::DrawQuad();
        }
        renderTarget.unbind();
    }
};
