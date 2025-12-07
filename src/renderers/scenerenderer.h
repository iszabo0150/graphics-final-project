#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "utils/scenedata.h"
#include "renderers/shaperenderer.h"
#include "utils/sceneparser.h"
#include "camera/camera.h"

class SceneRenderer {
public:
    void initialize();
    void render(const RenderData& data, const Camera& camera, ShapeRenderer& shapes);
    void cleanup();

private:
    GLuint m_shader;

    void setupCameraUniforms(const Camera& camera, glm::vec3 cameraPos);

    void setupShapeUniforms(const RenderShapeData& shape, const SceneMaterial& material);

    void setupLightUniforms(const std::vector<SceneLightData>& lights, SceneGlobalData globalData);

    void setupTextureUniforms(const SceneMaterial& material);

    GLuint loadTexture(const std::string& filename, bool isBump=false);

    std::map<std::string, GLuint> m_textureCache;
};

#endif // SCENERENDERER_H
