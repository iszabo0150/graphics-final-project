#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

class CrepuscularRenderer {

    public:
        CrepuscularRenderer();
        ~CrepuscularRenderer();

        void initialize(float exposure, float decay, float density,
                        float weight, int samples);
        void cleanup();

        void applyCrepuscularRays(GLuint sceneTexture, GLuint depthTexture,
                                  const glm::vec3& lightPosition, const glm::mat4& viewMatrix,
                                  const glm::mat4& projectionMatrix, int width, int height);

        GLuint getOutputTexture() const { return m_outputTexture; }
        GLuint getFBO() const { return m_fbo; }

    private:

        void initializeFBO(int width, int height);
        void initializeFullscreenQuad();

        void initializeDirectionalGeometry();
        void renderOcclusionMask(const glm::vec3& lightPosition,
                                 const glm::mat4& viewMatrix,
                                 const glm::mat4& projectionMatrix);

        // sun geometry
        GLuint m_directionalVAO;
        GLuint m_directionalVBO;
        int m_directionalVerticies;

        GLuint m_crepuscularShader;
        GLuint m_occlusionShader;
        
        GLuint m_fbo;
        GLuint m_outputTexture;

        // occlusion fbo
        GLuint m_occlusionFBO;
        GLuint m_occlusionTexture;

        // fullscreen quad
        GLuint m_quadVAO;
        GLuint m_quadVBO;

        // default fbo
        GLuint m_defaultFBO;

        // crepuscular rays parameters
        float m_exposure;
        float m_decay;
        float m_density;
        float m_weight;
        int m_samples;

};
