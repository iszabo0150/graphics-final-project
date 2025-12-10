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
    void render(const RenderData& renderData, const Camera& camera, ShapeRenderer& shapeRenderer, const Shadow &shadow);
    void cleanup();

private:
    GLuint m_shader;

    void setupShadowUniform(const Shadow& shadow);

    void setupCameraUniforms(const Camera& camera, glm::vec3 cameraPos);

    void setupShapeUniforms(const RenderShapeData& shape, const SceneMaterial& material);

    void setupLightUniforms(const std::vector<SceneLightData>& lights, SceneGlobalData globalData);

    void setupTextureUniforms(const SceneMaterial& material);

    GLuint loadTexture(const std::string& filename, bool isBump=false, GLuint slot=0);

    std::map<std::string, GLuint> m_textureCache;
};

#endif // SCENERENDERER_H
