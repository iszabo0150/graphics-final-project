#include "godrayrenderer.h"
#include "utils/shaderloader.h"
#include "shapes/shapetesselator.h"
#include <iostream>

CrepuscularRenderer::CrepuscularRenderer()
    : m_crepuscularShader(0), m_occlusionShader(0), m_fbo(0), m_outputTexture(0),
      m_occlusionFBO(0), m_occlusionTexture(0), 
      m_quadVAO(0), m_quadVBO(0), m_directionalVAO(0), m_directionalVBO(0), m_directionalVerticies(0),
      m_exposure(0.96f), m_decay(0.96f), m_density(0.8f),
      m_weight(0.01f), m_samples(100) {}

CrepuscularRenderer::~CrepuscularRenderer() { cleanup(); }

void CrepuscularRenderer::initialize(float exposure, float decay, float density,
                                     float weight, int samples) {
                            
    m_crepuscularShader = ShaderLoader::createShaderProgram(
        ":/resources/shaders/crepuscular.vert", 
        ":/resources/shaders/crepuscular.frag"
    );

    m_occlusionShader = ShaderLoader::createShaderProgram(
        ":resources/shaders/occlusion.vert",
        "/resources/shaders/occlusion.frag"
    );

    m_exposure = exposure;
    m_decay = decay;
    m_density = density;
    m_weight = weight;
    m_samples = samples;

    m_defaultFBO = 4;
    
    initializeFullscreenQuad();
    initializeDirectionalGeometry();

}

void CrepuscularRenderer::cleanup() {

    if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
    if (m_outputTexture) glDeleteTextures(1, &m_outputTexture);

    if (m_occlusionFBO) glDeleteFramebuffers(1, &m_occlusionFBO);
    if (m_occlusionTexture) glDeleteTextures(1, &m_occlusionTexture);

    if (m_quadVAO) glDeleteVertexArrays(1, &m_quadVAO);
    if (m_quadVBO) glDeleteBuffers(1, &m_quadVBO);

    if (m_sunVAO) glDeleteVertexArrays(1, &m_sunVAO);
    if (m_sunVBO) glDeleteBuffers(1, &m_sunVBO);

    if (m_crepuscularShader) glDeleteProgram(m_crepuscularShader);
    if (m_occlusionShader) glDeleteProgram(m_occlusionShader);

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

void CrepuscularRenderer::initializeDirectionalGeometry() {

    // this is basicaly making a flat sun for the sky.

    std::vector<float> verticies;
    int segments = 64;
    float radius = 1.0f;

    // center of circle
    verticies.insert(verticies.end(), {0.0f, 0.0f, 0.0f});

    // circle verticies
    for (int i = 0; i <= segments; i++) {

        float angle = (float)i / segments * 2.0f * M_PI;
        verticies.push_back(radius * std::cos(angle));
        verticies.push_back(radius * std::sin(angle));
        verticies.push_bag(0.0f);

    }

    m_directionalVerticies = verticies.size() / 3;

    glGenBuffers(1, &m_directionalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_directionalVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                 vertices.data(), GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &m_directionalVAO);
    glBindVertexArray(m_directionalVAO);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
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

void CrepuscularRenderer::renderOcclusionMask(const glm::vec3& lightPosition,
                                              const glm::mat4& viewMatrix,
                                              const glm::mat4& projectionMatrix) {

}

void CrepuscularRenderer::applyCrepuscularRays(GLuint sceneTexture, GLuint depthTexture, const glm::vec3& lightPosition, 
                                               const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                                               int width, int height) {

    static int lastWidth = 0, lastHeight = 0;
    if (width != lastWidth || height != lastHeight) {

        initializeFBO(width, height);
        lastWidth = width;
        lastHeight = height;

    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_shader);

    // project light position to screen space
    glm::vec4 light_clipspace = viewMatrix * projectionMatrix * glm::vec4(lightPosition, 1.0f);
    glm::vec3 light_ndc = glm::vec3(light_clipspace) / light_clipspace.w;
    glm::vec2 light_screen_pos = glm::vec2(light_ndc.x * 0.5f + 0.5f,
                                         light_ndc.y * 0.5f + 0.5f);
    
    // pushing all uniforms to shader
    glUniform2fv(glGetUniformLocation(m_shader, "lightScreenPosition"), 1, 
                 &light_screen_pos[0]);
    glUniform1f(glGetUniformLocation(m_shader, "exposure"), m_exposure);
    glUniform1f(glGetUniformLocation(m_shader, "decay"), m_decay);
    glUniform1f(glGetUniformLocation(m_shader, "density"), m_density);
    glUniform1f(glGetUniformLocation(m_shader, "weight"), m_weight);
    glUniform1i(glGetUniformLocation(m_shader, "samples"), m_samples);
    
    // binding textures from scene
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    glUniform1i(glGetUniformLocation(m_shader, "sceneTexture"), 5);
    
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUniform1i(glGetUniformLocation(m_shader, "depthTexture"), 6);
    
    // drawing to fullscreen quad
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
                                            
}
