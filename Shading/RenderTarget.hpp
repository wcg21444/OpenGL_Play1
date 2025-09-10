#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <iostream>

#include "Texture.hpp"

// FBO 封装
class RenderTarget
{
private:
    unsigned int m_ID;
    std::vector<GLenum> m_attachments;
    int width;
    int height;

public:
    RenderTarget(int _width, int _height)
        : width(_width), height(_height)
    {
        glGenFramebuffers(1, &m_ID);
    }
    ~RenderTarget()
    {
        glDeleteFramebuffers(1, &m_ID);
    }

    void bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_ID);
    }
    void unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void attachColorTexture2D(TextureID textureID, GLenum attachment)
    {
        bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, textureID, 0);
        if (std::find(m_attachments.begin(), m_attachments.end(), attachment) == m_attachments.end())
        {
            m_attachments.push_back(attachment);
        }
    }

    void attachDepthTexture2D(TextureID textureID)
    {
        bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);
    }

    void disableDrawColor()
    {
        bind();
        glDrawBuffer(GL_NONE);
    }

    void disableReadColor()
    {
        bind();
        glReadBuffer(GL_NONE);
    }

    void enableColorAttachments()
    {
        bind();
        glDrawBuffers(m_attachments.size(), m_attachments.data());
    }
    void resize(int _width, int _height)
    {
        width = _width;
        height = _height;
    }

    /// @brief
    /// @param options GL_COLOR_BUFFER_BIT , GL_DEPTH_BUFFER_BIT
    void clearBuffer(GLenum options,glm::vec4 clearColor = glm::vec4(0.0f,0.0f,0.0f,1.0f))
    {
        bind();
        glClearColor(clearColor.r,clearColor.g,clearColor.b,clearColor.a);
        glClear(options);
    }
    void setViewport()
    {
        glViewport(0, 0, width, height);
    }
};