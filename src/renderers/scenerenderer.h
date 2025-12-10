#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include "utils/scenedata.h"
#include "renderers/shaperenderer.h"
#include "renderers/lightrenderer.h"
#include "utils/sceneparser.h"
#include "camera/camera.h"
#include "utils/terraingenerator.h"

class SceneRenderer {
public:

    void initialize(GLuint texture_shader);
    void cleanup();
    void resize(int width, int height);
    void setDefaultFBO(GLuint fbo) { m_defaultFBO = fbo; }

    void render(const RenderData& renderData, const Camera& camera, 
                ShapeRenderer& shapeRenderer, const Shadow &shadow);
    
    GLuint getSceneTexture() const { return m_sceneTexture; }
    GLuint getDepthTexture() const { return m_depthTexture; }
    GLuint getSceneFBO() const { return m_sceneFBO; }

    void paintTexture(const Camera& camera);
    void paintTerrain(const Camera& camera);

private:
    
    void paintTerrainInternal(const Camera& camera);
    
    void initializeFBO(int width, int height);

    GLuint m_shader;
    GLuint m_terrain_shader;

    void setupShadowUniform(const Shadow& shadow);
    void setupCameraUniforms(const Camera& camera, glm::vec3 cameraPos);
    void setupShapeUniforms(const RenderShapeData& shape, const SceneMaterial& material);
    void setupLightUniforms(const std::vector<SceneLightData>& lights, SceneGlobalData globalData);
    void setupTextureUniforms(const SceneMaterial& material);
    GLuint loadTexture(const std::string& filename, bool isBump=false, GLuint slot=0);

    GLuint m_defaultFBO;

    // scene fbo info
    GLuint m_sceneFBO;
    GLuint m_sceneTexture;
    GLuint m_depthTexture;

    int m_fboWidth;
    int m_fboHeight;


    void loadSkybox();
    void loadTerrain();


    std::map<std::string, GLuint> m_textureCache;

    // terrain
    GLuint m_terrain_vao;
    GLuint m_terrain_vbo;
    TerrainGenerator m_terrain;

    // skybox
    GLuint m_skybox_texture;
    GLuint m_texture_shader;

    GLint m_loc_cubeMap;
    GLint m_loc_skyboxView;
    GLint m_loc_skyboxProj;

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

    // std::vector<std::string> m_faces = {":/resources/images/right.jpg",
    //                                     ":/resources/images/left.jpg",
    //                                     ":/resources/images/bottom.jpg",
    //                                     ":/resources/images/top.jpg",
    //                                     ":/resources/images/front.jpg",
    //                                     ":/resources/images/back.jpg"};
    std::vector<std::string> m_faces = {":/resources/images/pz.png",
                                        ":/resources/images/nz.png",
                                        ":/resources/images/ny.png",
                                        ":/resources/images/py.png",
                                        ":/resources/images/px.png",
                                        ":/resources/images/nx.png"};
};

#endif // SCENERENDERER_H
