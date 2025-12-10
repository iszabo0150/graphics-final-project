#include "godrayrenderer.h"
#include "utils/shaderloader.h";
#include <iostream>

CrepuscularRenderer::CrepuscularRenderer()
    : m_shader(0), m_fbo(0), m_outputTexture(0),
      m_quadVAO(0), m_quadVBO(0) {}

CrepuscularRenderer::~CrepuscularRenderer() { cleanup(); }

void CrepuscularRenderer::initialize(float exposure, float decay, float density,
                                     float weight, int samples) {

    m_shader = ShaderLoader::createShaderProgram(
        ":/resources/shaders/texture.vert", 
        ":/resources/shaders/crepuscular.frag"
    );

    m_exposure = exposure;
    m_decay = decay;
    m_density = density;
    m_weight = weight;
    m_samples = samples;

    m_defaultFBO = 4;
    
    initializeFullscreenQuad();

}

void CrepuscularRenderer::cleanup() {

    if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
    if (m_outputTexture) glDeleteTextures(1, &m_outputTexture);
    if (m_quadVAO) glDeleteVertexArrays(1, &m_quadVAO);
    if (m_quadVBO) glDeleteBuffers(1, &m_quadVBO);
    if (m_shader) glDeleteProgram(m_shader);

}

void CrepuscularRenderer::initializeFullscreenQuad() {


    std::vector<GLfloat> quad = {

        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f

    };

    glGenBuffers(1, &m_quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, quad.size() * sizeof(GLfloat), 
                 quad.data(), GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &m_quadVAO);
    glBindVertexArray(m_quadVAO);
    
    // pos coords
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    
    // uv coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 
                         (void*)(3 * sizeof(GLfloat)));
    
    glBindVertexArray(0);

}

void CrepuscularRenderer::initializeFBO(int width, int height) {

    if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
    if (m_outputTexture) glDeleteTextures(1, &m_outputTexture);

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_outputTexture);
    glBindTexture(GL_TEXTURE_2D, m_outputTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, 
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                          GL_TEXTURE_2D, m_outputTexture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);

}

void CrepuscularRenderer::applyCrepuscularRays(GLuint sceneTexture, GLuint depthTexture, const glm::vec3& lightPosition, 
                                               const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                                               int width, int height) {

    static int lastWidth = 0, lastHeight = 0;
    if (width != lastWidth || height != lastHeight) {

        intiializeFBO(width, height);
        lastWidth = width;
        lastHeight = height;

    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_shader);

    // project light position to screen space
    glm::vec4 lightClipSpace = viewProj * glm::vec4(lightPos, 1.0f);
    glm::vec3 lightNDC = glm::vec3(lightClipSpace) / lightClipSpace.w;
    glm::vec2 lightScreenPos = glm::vec2(lightNDC.x * 0.5f + 0.5f, 
                                         lightNDC.y * 0.5f + 0.5f);
    
    // pushing all uniforms to shader
    glUniform2fv(glGetUniformLocation(m_shader, "lightScreenPosition"), 1, 
                 &lightScreenPos[0]);
    glUniform1f(glGetUniformLocation(m_shader, "exposure"), m_exposure);
    glUniform1f(glGetUniformLocation(m_shader, "decay"), m_decay);
    glUniform1f(glGetUniformLocation(m_shader, "density"), m_density);
    glUniform1f(glGetUniformLocation(m_shader, "weight"), m_weight);
    glUniform1i(glGetUniformLocation(m_shader, "samples"), m_samples);
    
    // binding textures from scene
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    glUniform1i(glGetUniformLocation(m_shader, "sceneTexture"), 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUniform1i(glGetUniformLocation(m_shader, "depthTexture"), 1);
    
    // drawing to fullscreen quad
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
                                            
}