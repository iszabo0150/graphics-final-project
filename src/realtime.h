#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <vector>
#include <string>

#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTimer>
#include <QTime>

#include "utils/sceneparser.h"
#include "utils/camera/camera.h"
#include "postprocess.h"
#include "particlesystem.h"

class Realtime : public QOpenGLWidget
{
    Q_OBJECT

public:
    enum class Season { Winter, Spring, Summer, Autumn };

    Realtime(QWidget *parent = nullptr);
    void finish();
    void sceneChanged();
    void settingsChanged();
    void saveViewportImage(std::string filePath);

public slots:
    void tick(QTimerEvent *event);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    void rebuildSceneGeometry();
    GLuint getOrCreateTexture(const SceneMaterial &mat);

    // Particles (season selection + spawn region that follows current scene camera)
    void applyParticleEmitterFromSettings();

    // Tick
    int m_timer = 0;
    QElapsedTimer m_elapsedTimer;

    // Input
    bool m_mouseDown = false;
    glm::vec2 m_prev_mouse_pos;
    std::unordered_map<Qt::Key, bool> m_keyMap;

    // Device pixel ratio
    double m_devicePixelRatio = 1.0;

    // Shader program
    GLuint m_shaderProgram = 0;

    // Scene geometry buffers
    GLuint m_VAO = 0;
    GLuint m_VBO = 0;

    // Uniform locations
    GLint m_uModel        = -1;
    GLint m_uView         = -1;
    GLint m_uProj         = -1;
    GLint m_uKa           = -1;
    GLint m_uKd           = -1;
    GLint m_uKs           = -1;
    GLint m_uShininess    = -1;
    GLint m_uCamPos       = -1;

    GLint m_uMatAmbient   = -1;
    GLint m_uMatDiffuse   = -1;
    GLint m_uMatSpecular  = -1;

    GLint m_uNumLights          = -1;
    GLint m_uLightPosArray      = -1;
    GLint m_uLightDirArray      = -1;
    GLint m_uLightColorArray    = -1;
    GLint m_uLightFuncArray     = -1;
    GLint m_uLightTypeArray     = -1;
    GLint m_uLightAngleArray    = -1;
    GLint m_uLightPenumbraArray = -1;

    GLint m_uUseTexture     = -1;
    GLint m_uTextureSampler = -1;
    GLint m_uTexRepeat      = -1;
    GLint m_uTexBlend       = -1;
    GLint m_uShapeType      = -1;

    struct GLShapeInstance {
        GLint   first = 0;
        GLsizei count = 0;
        glm::mat4 model = glm::mat4(1.f);
        SceneMaterial material;

        PrimitiveType type = PrimitiveType::PRIMITIVE_CUBE;
        bool   hasTexture = false;
        GLuint textureID  = 0;
        glm::vec2 texRepeat = glm::vec2(1.f, 1.f);
        float  texBlend = 0.f;
    };

    std::vector<GLShapeInstance> m_shapesGL;

    RenderData m_renderData;
    Camera     m_camera;

    // Interactive camera state
    glm::vec3 m_camPos  = glm::vec3(0.f, 0.f, 5.f);
    glm::vec3 m_camLook = glm::vec3(0.f, 0.f, -1.f);
    glm::vec3 m_camUp   = glm::vec3(0.f, 1.f, 0.f);
    bool      m_cameraInitialized = false;

    // Texture cache
    std::unordered_map<std::string, GLuint> m_textureCache;

    // Post-processing
    PostProcess m_post;
    int  m_postMode = 0;
    bool m_bloomEnabled = false;

    // Particles
    ParticleSystem m_particles;
    Season m_particleSeason = Season::Winter;
};
