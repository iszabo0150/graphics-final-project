#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

class ShapeRenderer;
struct RenderData;

class CrepuscularRenderer {

    public:
        CrepuscularRenderer();
        ~CrepuscularRenderer();

        void initialize(float exposure, float decay, float density,
                        float weight, int samples);
        void cleanup();
        void setDefaultFBO(GLuint fbo) { m_defaultFBO = fbo; }

        void applyCrepuscularRays(GLuint sceneTexture, GLuint depthTexture,
                      const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                      int width, int height, const RenderData& renderData,
                      ShapeRenderer& shapeRenderer);
        
        void renderGodRaysToScreen(GLuint sceneTexture, GLuint depthTexture,
                       const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                       int width, int height, const RenderData& renderData,
                       ShapeRenderer& shapeRenderer);

        GLuint getOutputTexture() const { return m_outputTexture; }
        GLuint getFBO() const { return m_fbo; }

    private:

        void initializeFBO(int width, int height);
        void initializeFullscreenQuad();

        void initializeDirectionalGeometry();
        void renderOcclusionMask(const glm::mat4& viewMatrix,
                     const glm::mat4& projectionMatrix,
                     const RenderData& renderData,
                     ShapeRenderer& shapeRenderer);


        GLuint m_crepuscularShader;
        GLuint m_occlusionShader;
        
        GLuint m_fbo;
        GLuint m_outputTexture;

        // occlusion fbo
        GLuint m_occlusionFBO;
        GLuint m_occlusionTexture;
        GLuint m_occlusionDepth;

        // fullscreen quad
        GLuint m_quadVAO;
        GLuint m_quadVBO;

        //
        int m_fboWidth;
        int m_fboHeight;
        int m_occlusionWidth;
        int m_occlusionHeight;

        // default fbo
        GLuint m_defaultFBO;

        // crepuscular rays parameters
        float m_exposure;
        float m_decay;
        float m_density;
        float m_weight;
        int m_samples;

};
