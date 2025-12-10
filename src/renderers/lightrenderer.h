#ifndef LIGHTRENDERER_H
#define LIGHTRENDERER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <QOpenGLWidget>
#include "utils/scenedata.h"
#include "utils/sceneparser.h"
#include "renderers/shaperenderer.h"

struct Shadow {
    GLuint fbo;
    GLuint depth_map;
};

class LightRenderer {
public:
    void initialize(ShapeRenderer renderer, GLuint texture_shader);
    void render(const RenderData& renderData, GLuint screenWidth, GLuint screenHeight);
    static glm::mat4 calculateLightMatrix(SceneLight *light, glm::vec3 position, glm::vec3 dir);
    Shadow getShadow();
    void setShapes(ShapeRenderer renderer);

private:
    void makeShadowFBO();
    void renderDepth(const RenderData& renderData);
    void paintTexture(GLuint texture);

    GLuint m_depth_shader;
    GLuint m_texture_shader;
    GLuint m_default_fbo;

    GLuint SHADOW_WIDTH = 1024;
    GLuint SHADOW_HEIGHT = 1024;

    GLuint m_fullscreen_vbo;
    GLuint m_fullscreen_vao;

    ShapeRenderer m_shape_renderer;

    glm::mat4 m_model = glm::mat4(1.0f);

    Shadow m_shadow;

    GLuint m_texture;

};

#endif // LIGHTRENDERER_H
