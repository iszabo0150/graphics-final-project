#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "utils/scenedata.h"
#include "renderers/shaperenderer.h"
#include "renderers/lightrenderer.h"
#include "utils/sceneparser.h"
#include "camera/camera.h"

class SceneRenderer {
public:
    void initialize();
    void cleanup();
    void resize(int width, int height);

    void render(const RenderData& renderData, const Camera& camera, 
                ShapeRenderer& shapeRenderer, const Shadow &shadow);
    
    GLuint getSceneTexture() const { return m_sceneTexture; }
    GLuint getDepthTexture() const { return m_depthTexture; }

private:
    
    void initializeFBO(int width, int height);
    void setupShadowUniform(const Shadow& shadow);
    void setupCameraUniforms(const Camera& camera, glm::vec3 cameraPos);
    void setupShapeUniforms(const RenderShapeData& shape, const SceneMaterial& material);
    void setupLightUniforms(const std::vector<SceneLightData>& lights, SceneGlobalData globalData);
    void setupTextureUniforms(const SceneMaterial& material);
    GLuint loadTexture(const std::string& filename, bool isBump=false, GLuint slot=0);

    GLuint m_shader;
    std::map<std::string, GLuint> m_textureCache;

    GLuint m_defaultFBO;

    // scene fbo info
    GLuint m_sceneFBO;
    GLuint m_sceneTexture;
    GLuint m_depthTexture;

    int m_fboWidth;
    int m_fboHeight;

};

#endif // SCENERENDERER_H
