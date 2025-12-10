#include "godrayrenderer.h"
#include "glm/ext/matrix_transform.hpp"
#include "utils/sceneparser.h"
#include "utils/shaderloader.h"
#include "shapes/shapetesselator.h"
#include "shaperenderer.h"
#include "utils/scenedata.h"
#include <iostream>

CrepuscularRenderer::CrepuscularRenderer()
    : m_crepuscularShader(0), m_occlusionShader(0), m_fbo(0), m_outputTexture(0),
      m_occlusionFBO(0), m_occlusionTexture(0), m_occlusionDepth(0),
      m_quadVAO(0), m_quadVBO(0),
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
        ":/resources/shaders/occlusion.vert",
        ":/resources/shaders/occlusion.frag"
    );

    m_exposure = exposure;
    m_decay = decay;
    m_density = density;
    m_weight = weight;
    m_samples = samples;
    m_defaultFBO = 0;
    
    initializeFullscreenQuad();

}

void CrepuscularRenderer::cleanup() {

    if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
    if (m_outputTexture) glDeleteTextures(1, &m_outputTexture);

    if (m_occlusionFBO) glDeleteFramebuffers(1, &m_occlusionFBO);
    if (m_occlusionTexture) glDeleteTextures(1, &m_occlusionTexture);
    if (m_occlusionDepth) glDeleteRenderbuffers(1, &m_occlusionDepth);

    if (m_quadVAO) glDeleteVertexArrays(1, &m_quadVAO);
    if (m_quadVBO) glDeleteBuffers(1, &m_quadVBO);

    if (m_crepuscularShader) glDeleteProgram(m_crepuscularShader);
    if (m_occlusionShader) glDeleteProgram(m_occlusionShader);

}

void CrepuscularRenderer::initializeFullscreenQuad() {


    std::vector<GLfloat> quad = {
        -1.0f,  1.0f,  0.0f, 1.0f,  // top-left
        -1.0f, -1.0f,  0.0f, 0.0f,  // bottom-left
         1.0f, -1.0f,  1.0f, 0.0f,  // bottom-right

        -1.0f,  1.0f,  0.0f, 1.0f,  // top-left
         1.0f, -1.0f,  1.0f, 0.0f,  // bottom-right
         1.0f,  1.0f,  1.0f, 1.0f   // top-right
    };

    glGenBuffers(1, &m_quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, quad.size() * sizeof(GLfloat), 
                 quad.data(), GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &m_quadVAO);
    glBindVertexArray(m_quadVAO);
    
    // position (2D, not 3D)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    
    // uv coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 
                         (void*)(2 * sizeof(GLfloat)));
    
    glBindVertexArray(0);

}

void CrepuscularRenderer::initializeFBO(int width, int height) {

    m_fboWidth = width;
    m_fboHeight = height;

    // end scene fbo
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                          GL_TEXTURE_2D, m_outputTexture, 0);


    // making occlusion fbo at HALF RESOLUTION for performance
    int occlusionWidth = width / 2;
    int occlusionHeight = height / 2;
    if (occlusionWidth < 1) occlusionWidth = 1;
    if (occlusionHeight < 1) occlusionHeight = 1;
    
    m_occlusionWidth = occlusionWidth;
    m_occlusionHeight = occlusionHeight;
    
    if (m_occlusionFBO) glDeleteFramebuffers(1, &m_occlusionFBO);
    if (m_occlusionTexture) glDeleteTextures(1, &m_occlusionTexture);
    if (m_occlusionDepth) glDeleteRenderbuffers(1, &m_occlusionDepth);

    glGenFramebuffers(1, &m_occlusionFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_occlusionFBO);

    glGenTextures(1, &m_occlusionTexture);
    glBindTexture(GL_TEXTURE_2D, m_occlusionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, occlusionWidth, occlusionHeight, 0, 
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                          GL_TEXTURE_2D, m_occlusionTexture, 0);
    
    // create depth renderbuffer for proper depth testing
    glGenRenderbuffers(1, &m_occlusionDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, m_occlusionDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, occlusionWidth, occlusionHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, m_occlusionDepth);

    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);

}

void CrepuscularRenderer::renderOcclusionMaskInternal(const glm::mat4& viewMatrix,
                                              const glm::mat4& projectionMatrix,
                                              const RenderData& renderData,
                                              ShapeRenderer& shapeRenderer) {

    // save me.
    glBindFramebuffer(GL_FRAMEBUFFER, m_occlusionFBO);
    glViewport(0, 0, m_occlusionWidth, m_occlusionHeight);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(m_occlusionShader);
    
    // upload view and projection matrices
    glUniformMatrix4fv(glGetUniformLocation(m_occlusionShader, "view"),
                       1, GL_FALSE, &viewMatrix[0][0]);

    glUniformMatrix4fv(glGetUniformLocation(m_occlusionShader, "proj"),
                       1, GL_FALSE, &projectionMatrix[0][0]);
    

    GLint modelLocation = glGetUniformLocation(m_occlusionShader, "model");
    GLint colorLocation = glGetUniformLocation(m_occlusionShader, "occlusionColor");
    glUniform4f(colorLocation, 0.0f, 0.0f, 0.0f, 1.0f);
    
    for (const auto& shape : renderData.shapes) {
        if (shape.primitive.type == PrimitiveType::PRIMITIVE_MESH) {
            continue; // meshes handled elsewhere, skip for occlusion
        }

        try {
            GLPrimitiveData primitiveData = shapeRenderer.getPrimitiveData(shape.primitive.type);
            if (primitiveData.vao == 0 || primitiveData.vertexCount == 0) {
                continue;
            }

            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &shape.ctm[0][0]);
            glBindVertexArray(primitiveData.vao);
            glDrawArrays(GL_TRIANGLES, 0, primitiveData.vertexCount);
            glBindVertexArray(0);
        } catch (...) {
            continue;
        }
    }


    // render all lights as white spheres
    glUniform4f(colorLocation, 1.0f, 1.0f, 1.0f, 1.0f);

    for (const auto& light : renderData.lights) {

        glm::mat4 lightModel = glm::mat4(1.0f);

        // rendering the sun to be gigantic.
        if (light.type == LightType::LIGHT_DIRECTIONAL) {

            glm::vec3 lightDir = glm::normalize(glm::vec3(light.dir));
            glm::vec3 lightWorldPos = -lightDir * 100.0f;
            lightModel = glm::translate(lightModel, lightWorldPos);
            lightModel = glm::scale(lightModel, glm::vec3(25.0f));

        } else if (light.type == LightType::LIGHT_POINT) {

            lightModel = glm::translate(lightModel, glm::vec3(light.pos));
            lightModel = glm::scale(lightModel, glm::vec3(0.5f));

        } else {
            lightModel = glm::translate(lightModel, glm::vec3(light.pos));
            lightModel = glm::scale(lightModel, glm::vec3(0.3f));
        }

        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &lightModel[0][0]);

        try {
            GLuint sphereVAO = shapeRenderer.getVAO(PrimitiveType::PRIMITIVE_SPHERE);
            int sphereVertices = shapeRenderer.getVertexCount(PrimitiveType::PRIMITIVE_SPHERE);
            if (sphereVAO != 0 && sphereVertices > 0) {
                glBindVertexArray(sphereVAO);
                glDrawArrays(GL_TRIANGLES, 0, sphereVertices);
                glBindVertexArray(0);
            }
        } catch (...) {
            continue;
        }
    }
    
    
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);

}

void CrepuscularRenderer::renderOcclusion(const glm::mat4& viewMatrix,
                                          const glm::mat4& projectionMatrix,
                                          int width, int height,
                                          const RenderData& renderData,
                                          ShapeRenderer& shapeRenderer) {

    static int lastWidth = 0, lastHeight = 0;
    if (width != lastWidth || height != lastHeight) {
        initializeFBO(width, height);
        lastWidth = width;
        lastHeight = height;
    }

    // render occlusion mask to occlusion FBO
    renderOcclusionMaskInternal(viewMatrix, projectionMatrix, renderData, shapeRenderer);

}

void CrepuscularRenderer::blendCrepuscular(const glm::mat4& viewMatrix,
                                           const glm::mat4& projectionMatrix,
                                           int width, int height,
                                           const RenderData& renderData) {

    glViewport(0, 0, width, height);
    glUseProgram(m_crepuscularShader);

    // collect light screen positions
    std::vector<glm::vec4> lightPositions;
    for (const auto& light : renderData.lights) {
        glm::vec3 lightWorldPos;
        if (light.type == LightType::LIGHT_DIRECTIONAL) {
            glm::vec3 lightDir = glm::normalize(glm::vec3(light.dir));
            lightWorldPos = -lightDir * 100.0f;
        } else if (light.type == LightType::LIGHT_POINT) {
            lightWorldPos = glm::vec3(light.pos);
        } else {
            lightWorldPos = glm::vec3(light.pos);
        }

        glm::vec4 clip = projectionMatrix * viewMatrix * glm::vec4(lightWorldPos, 1.0f);
        glm::vec4 ndc = clip / clip.w;
        glm::vec4 screen = (ndc + 1.0f) * 0.5f;
        lightPositions.push_back(screen);
    }

    // setting all uniforms
    glUniform1i(glGetUniformLocation(m_crepuscularShader, "blurParams.sampleCount"), m_samples);
    glUniform1f(glGetUniformLocation(m_crepuscularShader, "blurParams.blurDensity"), m_density);
    glUniform1f(glGetUniformLocation(m_crepuscularShader, "blurParams.sampleWeight"), m_weight);
    glUniform1f(glGetUniformLocation(m_crepuscularShader, "blurParams.decayFactor"), m_decay);
    glUniform1f(glGetUniformLocation(m_crepuscularShader, "blurParams.blurExposure"), m_exposure);

    if (!lightPositions.empty()) {
        glUniform4fv(glGetUniformLocation(m_crepuscularShader, "lightPositionsScreen"),
                     lightPositions.size(), &lightPositions[0][0]);
    }
    glUniform1i(glGetUniformLocation(m_crepuscularShader, "lightCount"), static_cast<int>(lightPositions.size()));
    
    // bind occlusion texture only (scene already on screen)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_occlusionTexture);
    glUniform1i(glGetUniformLocation(m_crepuscularShader, "occlusionTexture"), 0);
    
    // draw fullscreen quad
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    glEnable(GL_DEPTH_TEST);
    glUseProgram(0);
}

/* OLD APPROACH: Renders to FBO then copies to screen (not used anymore)
void CrepuscularRenderer::applyCrepuscularRays_OLD(GLuint sceneTexture, GLuint depthTexture, 
                                                   const glm::vec3& lightPosition, 
                                                   const glm::mat4& viewMatrix, 
                                                   const glm::mat4& projectionMatrix,
                                                   int width, int height,
                                                   const RenderData& renderData, 
                                                   ShapeRenderer& shapeRenderer) {

    static int lastWidth = 0, lastHeight = 0;
    if (width != lastWidth || height != lastHeight) {
        initializeFBO(width, height);
        lastWidth = width;
        lastHeight = height;
    }

    // Render the occlusion mask
    renderOcclusionMask(lightPosition, viewMatrix, projectionMatrix, renderData, shapeRenderer);

    // Render to internal FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_crepuscularShader);

    // Project light position to screen space
    glm::mat4 viewProj = projectionMatrix * viewMatrix;
    glm::vec4 light_clipspace = viewProj * glm::vec4(lightPosition, 1.0f);
    glm::vec3 light_ndc = glm::vec3(light_clipspace) / light_clipspace.w;
    glm::vec2 light_screen_pos = glm::vec2(light_ndc.x * 0.5f + 0.5f,
                                         light_ndc.y * 0.5f + 0.5f);
    
    // Set all uniforms
    glUniform2fv(glGetUniformLocation(m_crepuscularShader, "lightScreenPosition"), 1, 
                 &light_screen_pos[0]);
    glUniform1f(glGetUniformLocation(m_crepuscularShader, "exposure"), m_exposure);
    glUniform1f(glGetUniformLocation(m_crepuscularShader, "decay"), m_decay);
    glUniform1f(glGetUniformLocation(m_crepuscularShader, "density"), m_density);
    glUniform1f(glGetUniformLocation(m_crepuscularShader, "weight"), m_weight);
    glUniform1i(glGetUniformLocation(m_crepuscularShader, "samples"), m_samples);
    
    // Bind both textures
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, sceneTexture);
    glUniform1i(glGetUniformLocation(m_crepuscularShader, "sceneTexture"), 5);
    
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, m_occlusionTexture);
    glUniform1i(glGetUniformLocation(m_crepuscularShader, "occlusionTexture"), 6);
    
    // Draw fullscreen quad
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);
}
*/
