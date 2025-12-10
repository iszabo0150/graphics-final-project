#include "screenrenderer.h"
#include "utils/shaderloader.h"
#include <iostream>

ScreenRenderer::ScreenRenderer()
    : m_shader(0), m_quadVAO(0), m_quadVBO(0) {}

ScreenRenderer::~ScreenRenderer() {
    cleanup();
}

void ScreenRenderer::initialize() {
    
    // this shader literally just copies the 
    // screen color data to the screen
    m_shader = ShaderLoader::createShaderProgram(
        ":/resources/shaders/copy.vert",
        ":/resources/shaders/copy.frag"
    );
    
    initializeFullscreenQuad();
}

void ScreenRenderer::cleanup() {

    if (m_quadVAO) {
        glDeleteVertexArrays(1, &m_quadVAO);
        m_quadVAO = 0;
    }

    if (m_quadVBO) {
        glDeleteBuffers(1, &m_quadVBO);
        m_quadVBO = 0;
    }

    if (m_shader) {
        glDeleteProgram(m_shader);
        m_shader = 0;
    }

}

void ScreenRenderer::initializeFullscreenQuad() {
    
    std::vector<GLfloat> quadData = {
        
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,  
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,  
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f, 
        
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,  
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,  
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f

    };
    
    glGenBuffers(1, &m_quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, quadData.size() * sizeof(GLfloat),
                 quadData.data(), GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &m_quadVAO);
    glBindVertexArray(m_quadVAO);
    
    // pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    
    // uv
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                         (void*)(3 * sizeof(GLfloat)));
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ScreenRenderer::renderToScreen(GLuint texture, int width, int height) {

    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);
    
    glUseProgram(m_shader);
    
    // bind texture
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(m_shader, "textureSampler"), 7);
    
    // draw fullscreen quad
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    glEnable(GL_DEPTH_TEST);
    glUseProgram(0);
}
