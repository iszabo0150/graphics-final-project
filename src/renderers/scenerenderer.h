#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "utils/scenedata.h"
#include "renderers/shaperenderer.h"
#include "renderers/lightrenderer.h"
#include "utils/sceneparser.h"
#include "camera/camera.h"

class SceneRenderer {
public:
    void initialize(GLuint texture_shader);
    void render(const RenderData& renderData, const Camera& camera, ShapeRenderer& shapeRenderer, const Shadow &shadow);
    void cleanup();
    void paintTexture(const Camera& camera);

private:
    GLuint m_shader;

    void setupShadowUniform(const Shadow& shadow);

    void setupCameraUniforms(const Camera& camera, glm::vec3 cameraPos);

    void setupShapeUniforms(const RenderShapeData& shape, const SceneMaterial& material);

    void setupLightUniforms(const std::vector<SceneLightData>& lights, SceneGlobalData globalData);

    void setupTextureUniforms(const SceneMaterial& material);

    void loadSkybox();

    GLuint loadTexture(const std::string& filename, bool isBump=false, GLuint slot=0);

    std::map<std::string, GLuint> m_textureCache;
    GLuint m_skybox_texture;
    GLuint m_texture_shader;

    std::vector<float> m_skybox_vertices = {   -1.0f,  1.0f, -1.0f,
                                            -1.0f, -1.0f, -1.0f,
                                            1.0f, -1.0f, -1.0f,
                                            1.0f, -1.0f, -1.0f,
                                            1.0f,  1.0f, -1.0f,
                                            -1.0f,  1.0f, -1.0f,

                                            -1.0f, -1.0f,  1.0f,
                                            -1.0f, -1.0f, -1.0f,
                                            -1.0f,  1.0f, -1.0f,
                                            -1.0f,  1.0f, -1.0f,
                                            -1.0f,  1.0f,  1.0f,
                                            -1.0f, -1.0f,  1.0f,

                                            1.0f, -1.0f, -1.0f,
                                            1.0f, -1.0f,  1.0f,
                                            1.0f,  1.0f,  1.0f,
                                            1.0f,  1.0f,  1.0f,
                                            1.0f,  1.0f, -1.0f,
                                            1.0f, -1.0f, -1.0f,

                                            -1.0f, -1.0f,  1.0f,
                                            -1.0f,  1.0f,  1.0f,
                                            1.0f,  1.0f,  1.0f,
                                            1.0f,  1.0f,  1.0f,
                                            1.0f, -1.0f,  1.0f,
                                            -1.0f, -1.0f,  1.0f,

                                            -1.0f,  1.0f, -1.0f,
                                            1.0f,  1.0f, -1.0f,
                                            1.0f,  1.0f,  1.0f,
                                            1.0f,  1.0f,  1.0f,
                                            -1.0f,  1.0f,  1.0f,
                                            -1.0f,  1.0f, -1.0f,

                                            -1.0f, -1.0f, -1.0f,
                                            -1.0f, -1.0f,  1.0f,
                                            1.0f, -1.0f, -1.0f,
                                            1.0f, -1.0f, -1.0f,
                                            -1.0f, -1.0f,  1.0f,
                                            1.0f, -1.0f,  1.0f};

    GLuint m_skybox_vao;
    GLuint m_skybox_vbo;
    glm::mat4 m_view;

    std::vector<std::string> m_faces = {":/resources/images/right.jpg",
                                        ":/resources/images/left.jpg",
                                        ":/resources/images/bottom.jpg",
                                        ":/resources/images/top.jpg",
                                        ":/resources/images/front.jpg",
                                        ":/resources/images/back.jpg"};
};

#endif // SCENERENDERER_H
