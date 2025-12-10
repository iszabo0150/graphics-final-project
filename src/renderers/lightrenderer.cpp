#include "lightrenderer.h"
#include "utils/shaderloader.h"
#include "utils/sceneparser.h"
#include "renderers/shaperenderer.h"
#include "realtime/realtime.h"

#include <QOpenGLWidget>
#include <glm/gtc/matrix_transform.hpp>

void LightRenderer::initialize(ShapeRenderer* renderer, GLuint texture_shader) {
    m_depth_shader = ShaderLoader::createShaderProgram(":/resources/shaders/depth.vert", ":/resources/shaders/depth.frag");
    m_texture_shader = texture_shader;
    m_default_fbo = 2; // was previously 2
    m_shape_renderer = renderer; // pass in shape info

    std::vector<GLfloat> fullscreen_quad_data =
        { // positions (3), uv coords (2)
            -1.f,  1.f, 0.0f, 0.0f, 1.0f,
            -1.f, -1.f, 0.0f, 0.0f, 0.0f,
            1.f, -1.f, 0.0f, 1.0f, 0.0f,
            1.f,  1.f, 0.0f, 1.0f, 1.0f,
            -1.f,  1.f, 0.0f, 0.0f, 1.0f,
            1.f, -1.f, 0.0f, 1.0f, 0.0f
        };

    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size() * sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    makeShadowFBO();
}

void LightRenderer::makeShadowFBO() {
    glGenFramebuffers(1, &m_shadow.fbo);
    glGenTextures(1, &m_shadow.depth_map);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_shadow.depth_map);

    // 1024 x 1024 16-bit depth texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT,
                 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    glBindFramebuffer(GL_FRAMEBUFFER, m_shadow.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadow.depth_map, 0);
    // tell OpenGL we aren't rendering color data
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Incomplete Depth FBO" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_default_fbo);
}

void LightRenderer::render(const RenderData& renderData, GLuint screenWidth, GLuint screenHeight) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_depth_shader);

    // support for a single light (the sun)
    const SceneLightData *light = &renderData.lights[0];
    GLint locMat = glGetUniformLocation(m_depth_shader, "lightMatrix");
    glUniformMatrix4fv(locMat, 1, GL_FALSE, &light->matrix[0][0]);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadow.fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderDepth(renderData);

    glBindFramebuffer(GL_FRAMEBUFFER, m_default_fbo);
    // ============= reset the viewport
    glViewport(0, 0, screenWidth, screenHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // paintTexture(m_shadow.depth_map);
}

void LightRenderer::setShapes(ShapeRenderer* renderer) {
    m_shape_renderer = renderer;
}

/*
 * Renders the scene from the light's perspective and saves
 * the resulting depth map in m_shadow_fbo
 */
void LightRenderer::renderDepth(const RenderData& renderData) {
    for (const RenderShapeData& shape : renderData.shapes) {

        // Set model matrix uniform (same for both primitives and meshes)
        m_model = shape.ctm;
        GLint modelMatLoc = glGetUniformLocation(m_depth_shader, "modelMatrix");
        glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, &m_model[0][0]);

        glActiveTexture(GL_TEXTURE0);

        if (shape.primitive.type == PrimitiveType::PRIMITIVE_MESH) {
            // Handle mesh
            MeshGLData meshData = m_shape_renderer->getMeshData(shape.primitive.meshfile);

            if (meshData.vertexCount == 0) {
                continue;  // Skip if mesh failed to load
            }

            glBindVertexArray(meshData.vao);
            glDrawArrays(GL_TRIANGLES, 0, meshData.vertexCount);
            glBindVertexArray(0);
        } else {
            // Handle primitive shapes
            GLPrimitiveData shapeData = m_shape_renderer->getPrimitiveData(shape.primitive.type);
            glBindVertexArray(shapeData.vao);
            glDrawArrays(GL_TRIANGLES, 0, shapeData.vertexCount);
            glBindVertexArray(0);
        }
    }
}

Shadow LightRenderer::getShadow() {
    return m_shadow;
}

// used for debugging, renders depth map
void LightRenderer::paintTexture(GLuint texture) {
    glUseProgram(m_texture_shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    GLuint loc_texture = glGetUniformLocation(m_texture_shader, "myTexture");
    glUniform1i(loc_texture, 0);

    glBindVertexArray(m_fullscreen_vao);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

glm::mat4 LightRenderer::calculateLightMatrix(SceneLight *light, glm::vec3 position, glm::vec3 dir) {

    glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
    glm::vec3 lightInvDir = -1.0f * dir;
    glm::mat4 lightProjection, lightView;

    switch(light->type) {
    case LightType::LIGHT_POINT:
        break;
    case LightType::LIGHT_DIRECTIONAL:
        // calculate light space matrix for shadow mapping
        // encompasses everything in the axis-aligned box (-10, 10), (-10, 10), (near, far)
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 10.0f);
        lightView = glm::lookAt(lightInvDir, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        lightSpaceMatrix = lightProjection * lightView;
        break;

    case LightType::LIGHT_SPOT:
        // change orthographic mat into perspective mat
        glm::vec3 look = glm::vec3(dir);
        glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);

        glm::vec3 w = -glm::normalize(glm::vec3(look));
        glm::vec3 v_prime = up - (glm::dot(up, w)) * w;
        glm::vec3 v = glm::normalize(v_prime);
        glm::vec3 u = glm::cross(v, w);

        glm::mat4 mT = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
                                 0.0f, 1.0f, 0.0f, 0.0f,
                                 0.0f, 0.0f, 1.0f, 0.0f,
                                 -position[0], -position[1], -position[2], 1.0f);

        glm::mat4 rot = glm::mat4(glm::vec4(u, 0.0f),
                                  glm::vec4(v, 0.0f),
                                  glm::vec4(w, 0.0f),
                                  glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        glm::mat4 mR = glm::transpose(rot);

        lightView = mR * mT;

        float far = 10.f;
        float near = 3.0f;
        float theta = light->angle * 2.0f;

        glm::mat4 scaleMat = glm::mat4(1.0f / (far * tan(theta / 2.0f)), 0.0f, 0.0f, 0.0f,
                                       0.0f, 1.0f / (far * tan(theta / 2.0f)), 0.0f, 0.0f,
                                       0.0f, 0.0f, 1.0f / far, 0.0f,
                                       0.0f, 0.0f, 0.0f, 1.0f);

        float c = -near / far;

        glm::mat4 unhingingMat = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
                                           0.0f, 1.0f, 0.0f, 0.0f,
                                           0.0f, 0.0f, 1.0f / (1.0f + c), -1.0f,
                                           0.0f, 0.0f, -c / (1.0f + c), 0.0f);

        glm::mat4 glFix = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 1.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, -2.0f, 0.0f,
                                    0.0f, 0.0f, -1.0f, 1.0f);


        lightProjection = glFix * unhingingMat * scaleMat;
        lightSpaceMatrix = lightProjection * lightView;
        break;
    }

    return lightSpaceMatrix;

}
