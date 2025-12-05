#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QImage>
#include <iostream>
#include <algorithm>
#include <cmath>

#include "settings.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "utils/shaderloader.h"
#include "utils/scenedata.h"
#include "tesselation/shapetesselation.h"

namespace {

static float clampf(float x, float lo, float hi) {
    return std::max(lo, std::min(hi, x));
}

ParticleSystem::Emitter makeWinterSnowEmitter() {
    ParticleSystem::Emitter e;
    e.enabled      = true;
    e.maxParticles = 22000;
    e.spawnRate    = 2200.f;

    e.baseVelocity   = glm::vec3(0.f, -1.6f, 0.f);
    e.velocityJitter = glm::vec3(0.7f, 0.25f, 0.7f);

    e.acceleration = glm::vec3(0.f, -0.25f, 0.f);
    e.drag         = 0.10f;

    e.lifeMin = 3.0f;
    e.lifeMax = 7.0f;

    e.sizeMin = 2.0f;
    e.sizeMax = 6.0f;

    e.colorStart = glm::vec4(1.f, 1.f, 1.f, 0.85f);
    e.colorEnd   = glm::vec4(1.f, 1.f, 1.f, 0.0f);

    e.windDir       = glm::vec3(1.f, 0.f, 0.f);
    e.windStrength  = 0.6f;
    e.windFrequency = 0.5f;

    return e;
}

ParticleSystem::Emitter makeSpringRainEmitter() {
    ParticleSystem::Emitter e;
    e.enabled      = true;
    e.maxParticles = 26000;
    e.spawnRate    = 4500.f;

    e.baseVelocity   = glm::vec3(0.f, -8.5f, 0.f);
    e.velocityJitter = glm::vec3(0.3f, 2.0f, 0.3f);

    e.acceleration = glm::vec3(0.f, -4.0f, 0.f);
    e.drag         = 0.0f;

    e.lifeMin = 0.5f;
    e.lifeMax = 1.2f;

    e.sizeMin = 5.0f;
    e.sizeMax = 8.0f;

    e.colorStart = glm::vec4(0.75f, 0.80f, 0.90f, 0.80f);
    e.colorEnd   = glm::vec4(0.75f, 0.80f, 0.90f, 0.0f);

    e.windDir       = glm::vec3(0.4f, 0.f, 0.2f);
    e.windStrength  = 2.0f;
    e.windFrequency = 1.5f;

    return e;
}

ParticleSystem::Emitter makeSummerFireflyEmitter() {
    ParticleSystem::Emitter e;
    e.enabled      = true;
    e.maxParticles = 220;
    e.spawnRate    = 35.f;

    e.baseVelocity   = glm::vec3(0.f, 0.15f, 0.f);
    e.velocityJitter = glm::vec3(0.35f, 0.20f, 0.35f);

    e.acceleration = glm::vec3(0.f);
    e.drag         = 0.25f;

    e.lifeMin = 3.5f;
    e.lifeMax = 8.0f;

    e.sizeMin = 5.0f;
    e.sizeMax = 8.0f;

    e.colorStart = glm::vec4(0.95f, 1.00f, 0.55f, 0.90f);
    e.colorEnd   = glm::vec4(0.95f, 1.00f, 0.55f, 0.0f);

    e.windDir       = glm::vec3(0.2f, 0.f, 0.2f);
    e.windStrength  = 0.6f;
    e.windFrequency = 0.5f;

    return e;
}

ParticleSystem::Emitter makeAutumnLeavesEmitter() {
    ParticleSystem::Emitter e;
    e.enabled      = true;
    e.maxParticles = 450;
    e.spawnRate    = 110.f;

    e.baseVelocity   = glm::vec3(0.f, -1.0f, 0.f);
    e.velocityJitter = glm::vec3(1.5f, 0.5f, 1.5f);

    e.acceleration = glm::vec3(0.f, -0.8f, 0.f);
    e.drag         = 0.30f;

    e.lifeMin = 3.0f;
    e.lifeMax = 6.0f;

    e.sizeMin = 6.0f;
    e.sizeMax = 10.0f;

    e.colorStart = glm::vec4(0.92f, 0.55f, 0.15f, 0.90f);
    e.colorEnd   = glm::vec4(0.60f, 0.20f, 0.05f, 0.0f);

    e.windDir       = glm::vec3(1.0f, 0.f, 0.3f);
    e.windStrength  = 1.0f;
    e.windFrequency = 0.4f;

    return e;
}

} // namespace

// ================== Rendering the Scene

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width() / 2, size().height() / 2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // default interactive camera state
    m_camPos  = glm::vec3(0.f, 0.f, 5.f);
    m_camLook = glm::vec3(0.f, 0.f, -1.f);
    m_camUp   = glm::vec3(0.f, 1.f, 0.f);
    m_cameraInitialized = false;

    // GL related members start at safe defaults
    m_shaderProgram = 0;
    m_VAO = 0;
    m_VBO = 0;

    // default post mode
    m_postMode = 0;

    // default season
    m_particleSeason = Season::Winter;

    // default emitter preset (actual enabling is driven by settings.extraCredit1)
    m_particles.setEmitter(makeWinterSnowEmitter());
    m_particles.setEnabled(false);

    m_bloomEnabled = false;
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    m_particles.destroyGL();
    m_post.destroy();

    for (auto &p : m_textureCache) {
        glDeleteTextures(1, &p.second);
    }
    m_textureCache.clear();

    if (m_VBO != 0) {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
    if (m_VAO != 0) {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    if (m_shaderProgram != 0) {
        glDeleteProgram(m_shaderProgram);
        m_shaderProgram = 0;
    }

    this->doneCurrent();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000 / 60);
    m_elapsedTimer.start();

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: "
                  << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version "
              << glewGetString(GLEW_VERSION) << std::endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glViewport(0, 0,
               size().width() * m_devicePixelRatio,
               size().height() * m_devicePixelRatio);

    int fw = int(size().width() * m_devicePixelRatio);
    int fh = int(size().height() * m_devicePixelRatio);
    if (fw > 0 && fh > 0) {
        m_post.init(fw, fh);
    }

    m_bloomEnabled = settings.extraCredit2;
    m_post.setBloomEnabled(m_bloomEnabled);
    m_post.setBloomStrength(settings.bloomStrength);

    float aspect = (height() > 0)
                       ? float(width()) / float(height())
                       : 1.f;
    m_camera.setAspect(aspect);

    float nearPlane = settings.nearPlane;
    float farPlane  = settings.farPlane;
    if (nearPlane <= 0.f || nearPlane >= farPlane) {
        nearPlane = 0.1f;
        farPlane  = 50.f;
    }
    m_camera.setClipPlanes(nearPlane, farPlane);

    try {
        m_shaderProgram = ShaderLoader::createShaderProgram(
            ":/resources/shaders/default.vert",
            ":/resources/shaders/default.frag"
            );
    } catch (const std::runtime_error &e) {
        std::cerr << "Shader compile/link error: " << e.what() << std::endl;
        m_shaderProgram = 0;
    }

    if (m_shaderProgram != 0) {
        glUseProgram(m_shaderProgram);

        m_uModel        = glGetUniformLocation(m_shaderProgram, "u_Model");
        m_uView         = glGetUniformLocation(m_shaderProgram, "u_View");
        m_uProj         = glGetUniformLocation(m_shaderProgram, "u_Proj");
        m_uKa           = glGetUniformLocation(m_shaderProgram, "k_a");
        m_uKd           = glGetUniformLocation(m_shaderProgram, "k_d");
        m_uKs           = glGetUniformLocation(m_shaderProgram, "k_s");
        m_uShininess    = glGetUniformLocation(m_shaderProgram, "shininess");
        m_uCamPos       = glGetUniformLocation(m_shaderProgram, "u_CamPos");

        m_uMatAmbient   = glGetUniformLocation(m_shaderProgram, "u_MatAmbient");
        m_uMatDiffuse   = glGetUniformLocation(m_shaderProgram, "u_MatDiffuse");
        m_uMatSpecular  = glGetUniformLocation(m_shaderProgram, "u_MatSpecular");

        m_uNumLights          = glGetUniformLocation(m_shaderProgram, "u_NumLights");
        m_uLightPosArray      = glGetUniformLocation(m_shaderProgram, "u_LightPos[0]");
        m_uLightDirArray      = glGetUniformLocation(m_shaderProgram, "u_LightDir[0]");
        m_uLightColorArray    = glGetUniformLocation(m_shaderProgram, "u_LightColor[0]");
        m_uLightFuncArray     = glGetUniformLocation(m_shaderProgram, "u_LightFunc[0]");
        m_uLightTypeArray     = glGetUniformLocation(m_shaderProgram, "u_LightType[0]");
        m_uLightAngleArray    = glGetUniformLocation(m_shaderProgram, "u_LightAngle[0]");
        m_uLightPenumbraArray = glGetUniformLocation(m_shaderProgram, "u_LightPenumbra[0]");

        m_uUseTexture     = glGetUniformLocation(m_shaderProgram, "u_UseTexture");
        m_uTextureSampler = glGetUniformLocation(m_shaderProgram, "u_Texture");
        m_uTexRepeat      = glGetUniformLocation(m_shaderProgram, "u_TexRepeat");
        m_uTexBlend       = glGetUniformLocation(m_shaderProgram, "u_TexBlend");
        m_uShapeType      = glGetUniformLocation(m_shaderProgram, "u_ShapeType");

        if (m_uTextureSampler >= 0) {
            glUniform1i(m_uTextureSampler, 0);
        }
        if (m_uNumLights >= 0) {
            glUniform1i(m_uNumLights, 0);
        }

        glUseProgram(0);
    }

    // initialize particle GL resources (after glewInit)
    m_particles.initializeGL();
    m_particles.setEnabled(settings.extraCredit1);
    applyParticleEmitterFromSettings();
}

// load or reuse texture for a given material
GLuint Realtime::getOrCreateTexture(const SceneMaterial &mat) {
    const SceneFileMap &tf = mat.textureMap;
    if (!tf.isUsed || tf.filename.empty()) {
        return 0;
    }

    auto it = m_textureCache.find(tf.filename);
    if (it != m_textureCache.end()) {
        return it->second;
    }

    QString qPath = QString::fromStdString(tf.filename);
    QImage img(qPath);
    if (img.isNull()) {
        std::cerr << "Failed to load texture: " << tf.filename << std::endl;
        return 0;
    }

    QImage glImg = img.convertToFormat(QImage::Format_RGBA8888);

    GLuint texID = 0;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA8,
                 glImg.width(),
                 glImg.height(),
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 glImg.constBits());

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_textureCache[tf.filename] = texID;
    return texID;
}

// build one big VBO of all shapes in the scene, with interleaved pos/normal.
void Realtime::rebuildSceneGeometry() {
    m_shapesGL.clear();

    if (m_shaderProgram == 0 || m_renderData.shapes.empty()) {
        std::cout << "[rebuildSceneGeometry] no shader or no shapes, skipping\n";
        return;
    }

    if (m_VAO == 0) {
        glGenVertexArrays(1, &m_VAO);
    }
    if (m_VBO == 0) {
        glGenBuffers(1, &m_VBO);
    }

    std::vector<float> vertexData;
    vertexData.reserve(1024);

    for (const RenderShapeData &shape : m_renderData.shapes) {
        int p1 = settings.shapeParameter1;
        int p2 = settings.shapeParameter2;

        ShapeMeshData mesh = generateShapeMesh(shape.primitive, p1, p2);
        if (mesh.vertices.empty()) continue;

        GLShapeInstance inst;
        inst.first    = static_cast<GLint>(vertexData.size() / 6);
        inst.count    = static_cast<GLsizei>(mesh.vertices.size() / 6);
        inst.model    = shape.ctm;
        inst.material = shape.primitive.material;
        inst.type     = shape.primitive.type;

        const SceneMaterial &mat = shape.primitive.material;
        float ru = (mat.textureMap.repeatU == 0.f) ? 1.f : mat.textureMap.repeatU;
        float rv = (mat.textureMap.repeatV == 0.f) ? 1.f : mat.textureMap.repeatV;
        inst.texRepeat = glm::vec2(ru, rv);
        inst.texBlend  = mat.blend;

        inst.textureID  = getOrCreateTexture(mat);
        inst.hasTexture = (inst.textureID != 0);

        vertexData.insert(vertexData.end(), mesh.vertices.begin(), mesh.vertices.end());
        m_shapesGL.push_back(inst);
    }

    std::cout << "[rebuildSceneGeometry] shapes: " << m_shapesGL.size()
              << ", total vertices: " << (vertexData.size() / 6) << std::endl;

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    if (!vertexData.empty()) {
        glBufferData(GL_ARRAY_BUFFER,
                     vertexData.size() * sizeof(float),
                     vertexData.data(),
                     GL_STATIC_DRAW);

        GLsizei stride = 6 * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    } else {
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Realtime::applyParticleEmitterFromSettings() {
    // which season is active? (checkboxes are treated like radio buttons in the UI)
    Season desired = Season::Winter;
    if (settings.particlesSpring)      desired = Season::Spring;
    else if (settings.particlesSummer) desired = Season::Summer;
    else if (settings.particlesAutumn) desired = Season::Autumn;
    else                               desired = Season::Winter;

    bool seasonChanged = (desired != m_particleSeason);
    if (seasonChanged) {
        m_particleSeason = desired;

        ParticleSystem::Emitter preset;
        switch (m_particleSeason) {
        case Season::Winter: preset = makeWinterSnowEmitter();     break;
        case Season::Spring: preset = makeSpringRainEmitter();     break;
        case Season::Summer: preset = makeSummerFireflyEmitter();  break;
        case Season::Autumn: preset = makeAutumnLeavesEmitter();   break;
        default:             preset = makeWinterSnowEmitter();     break;
        }

        m_particles.setEmitter(preset);
    }

    // Always refit spawn box to the current camera so particles are visible per scene.
    float nearPlane = settings.nearPlane;
    float farPlane  = settings.farPlane;
    if (nearPlane <= 0.f || nearPlane >= farPlane) {
        nearPlane = 0.1f;
        farPlane  = 50.f;
    }

    glm::vec3 forward = m_camLook;
    float fLen = glm::length(forward);
    if (fLen < 1e-6f) forward = glm::vec3(0.f, 0.f, -1.f);
    else forward = forward / fLen;

    float dist = 2.0f;
    dist = std::max(dist, nearPlane * 8.0f);
    dist = std::min(dist, farPlane * 0.35f);

    float base = clampf(farPlane * 0.15f, 1.25f, 14.0f);

    float exMul = 1.0f;
    float eyMul = 0.8f;
    float ezMul = 1.0f;
    float yOffsetMul = 0.0f;

    switch (m_particleSeason) {
    case Season::Winter: exMul = 1.25f; eyMul = 0.95f; ezMul = 1.25f; yOffsetMul = 0.55f; break;
    case Season::Spring: exMul = 1.35f; eyMul = 1.10f; ezMul = 1.35f; yOffsetMul = 0.75f; break;
    case Season::Summer: exMul = 0.70f; eyMul = 0.45f; ezMul = 0.70f; yOffsetMul = -0.10f; break;
    case Season::Autumn: exMul = 0.95f; eyMul = 0.75f; ezMul = 0.95f; yOffsetMul = 0.25f; break;
    default: break;
    }

    float ex = base * exMul;
    float ey = base * eyMul;
    float ez = base * ezMul;

    glm::vec3 center = m_camPos + forward * dist;
    center.y += base * yOffsetMul;

    auto &e = m_particles.emitter();
    e.spawnMin = center + glm::vec3(-ex, -ey, -ez);
    e.spawnMax = center + glm::vec3( ex,  ey,  ez);

    if (seasonChanged) {
        m_particles.reset();
    }
}

void Realtime::paintGL() {
    // Respect whichever framebuffer is currently bound (screen or screenshot FBO)
    GLint targetFbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &targetFbo);

    // Respect caller-defined viewport size (saveViewportImage sets this)
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    int fw = vp[2];
    int fh = vp[3];

    if (fw <= 0 || fh <= 0) return;

    // Sync bloom toggles + strength from settings
    bool bloomNow = settings.extraCredit2;
    if (bloomNow != m_bloomEnabled) {
        m_bloomEnabled = bloomNow;
        m_post.setBloomEnabled(m_bloomEnabled);
    }
    m_post.setBloomStrength(settings.bloomStrength);

    if (!m_post.ready()) {
        m_post.init(fw, fh);
        m_post.setBloomEnabled(m_bloomEnabled);
        m_post.setBloomStrength(settings.bloomStrength);
    } else {
        m_post.ensureSize(fw, fh);
    }

    bool usePost  = m_post.ready();
    bool useBloom = usePost && m_bloomEnabled && m_post.bloomReady();

    if (usePost) {
        m_post.beginScenePass(fw, fh);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, targetFbo);
        glViewport(0, 0, fw, fh);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    glm::mat4 view = m_cameraInitialized
                         ? glm::lookAt(m_camPos, m_camPos + m_camLook, m_camUp)
                         : m_camera.getViewMatrix();

    glm::mat4 proj = m_camera.getProjMatrix();

    bool canDrawScene = (m_shaderProgram != 0 && m_VAO != 0 && !m_shapesGL.empty());

    if (canDrawScene) {
        glUseProgram(m_shaderProgram);

        if (m_uView >= 0) glUniformMatrix4fv(m_uView, 1, GL_FALSE, glm::value_ptr(view));
        if (m_uProj >= 0) glUniformMatrix4fv(m_uProj, 1, GL_FALSE, glm::value_ptr(proj));

        const SceneGlobalData &globals = m_renderData.globalData;
        if (m_uKa >= 0) glUniform1f(m_uKa, globals.ka);
        if (m_uKd >= 0) glUniform1f(m_uKd, globals.kd);
        if (m_uKs >= 0) glUniform1f(m_uKs, globals.ks);

        if (m_uCamPos >= 0) {
            glm::vec3 eye = m_cameraInitialized ? m_camPos : m_camera.getEye();
            glm::vec4 camPos(eye, 1.f);
            glUniform4fv(m_uCamPos, 1, glm::value_ptr(camPos));
        }

        const int MAX_LIGHTS = 8;
        int totalLights = static_cast<int>(m_renderData.lights.size());
        int numLights = std::min(totalLights, MAX_LIGHTS);

        if (m_uNumLights >= 0) glUniform1i(m_uNumLights, numLights);

        if (numLights > 0) {
            std::vector<glm::vec3> pos(numLights);
            std::vector<glm::vec3> dir(numLights);
            std::vector<glm::vec3> col(numLights);
            std::vector<glm::vec3> func(numLights);
            std::vector<int>       type(numLights);
            std::vector<float>     angle(numLights);
            std::vector<float>     penumbra(numLights);

            for (int i = 0; i < numLights; ++i) {
                const SceneLightData &L = m_renderData.lights[i];
                pos[i]      = glm::vec3(L.pos);
                dir[i]      = glm::vec3(L.dir);
                col[i]      = glm::vec3(L.color);
                func[i]     = glm::vec3(L.function);
                angle[i]    = L.angle;
                penumbra[i] = L.penumbra;

                switch (L.type) {
                case LightType::LIGHT_POINT:       type[i] = 0; break;
                case LightType::LIGHT_DIRECTIONAL: type[i] = 1; break;
                case LightType::LIGHT_SPOT:        type[i] = 2; break;
                default:                           type[i] = 0; break;
                }
            }

            if (m_uLightPosArray >= 0)      glUniform3fv(m_uLightPosArray,      numLights, glm::value_ptr(pos[0]));
            if (m_uLightDirArray >= 0)      glUniform3fv(m_uLightDirArray,      numLights, glm::value_ptr(dir[0]));
            if (m_uLightColorArray >= 0)    glUniform3fv(m_uLightColorArray,    numLights, glm::value_ptr(col[0]));
            if (m_uLightFuncArray >= 0)     glUniform3fv(m_uLightFuncArray,     numLights, glm::value_ptr(func[0]));
            if (m_uLightTypeArray >= 0)     glUniform1iv(m_uLightTypeArray,     numLights, type.data());
            if (m_uLightAngleArray >= 0)    glUniform1fv(m_uLightAngleArray,    numLights, angle.data());
            if (m_uLightPenumbraArray >= 0) glUniform1fv(m_uLightPenumbraArray, numLights, penumbra.data());
        } else {
            if (m_uNumLights >= 0) glUniform1i(m_uNumLights, 0);
        }

        glBindVertexArray(m_VAO);

        for (const GLShapeInstance &inst : m_shapesGL) {
            if (m_uModel >= 0) glUniformMatrix4fv(m_uModel, 1, GL_FALSE, glm::value_ptr(inst.model));

            const SceneMaterial &mat = inst.material;

            if (m_uShapeType >= 0) glUniform1i(m_uShapeType, static_cast<int>(inst.type));

            if (m_uMatAmbient >= 0)  { glm::vec3 amb  = glm::vec3(mat.cAmbient);  glUniform3fv(m_uMatAmbient,  1, glm::value_ptr(amb)); }
            if (m_uMatDiffuse >= 0)  { glm::vec3 diff = glm::vec3(mat.cDiffuse);  glUniform3fv(m_uMatDiffuse,  1, glm::value_ptr(diff)); }
            if (m_uMatSpecular >= 0) { glm::vec3 spec = glm::vec3(mat.cSpecular); glUniform3fv(m_uMatSpecular, 1, glm::value_ptr(spec)); }
            if (m_uShininess >= 0) glUniform1f(m_uShininess, mat.shininess);

            if (inst.hasTexture && m_uUseTexture >= 0) {
                glUniform1i(m_uUseTexture, 1);
                if (m_uTexRepeat >= 0) glUniform2fv(m_uTexRepeat, 1, glm::value_ptr(inst.texRepeat));
                if (m_uTexBlend >= 0)  glUniform1f(m_uTexBlend, inst.texBlend);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, inst.textureID);
            } else if (m_uUseTexture >= 0) {
                glUniform1i(m_uUseTexture, 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            glDrawArrays(GL_TRIANGLES, inst.first, inst.count);
        }

        glBindVertexArray(0);
        glUseProgram(0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    } else {
        glUseProgram(0);
        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Draw particles into the same pass (post-processing affects them too)
    if (settings.extraCredit1) {
        m_particles.render(view, proj);
    }

    if (usePost) {
        int mode = useBloom ? 3 : m_postMode;
        m_post.endToTarget((GLuint)targetFbo, fw, fh, mode);
    }
}

void Realtime::resizeGL(int w, int h) {
    glViewport(0, 0,
               size().width() * m_devicePixelRatio,
               size().height() * m_devicePixelRatio);

    float aspect = (h > 0) ? float(w) / float(h) : 1.f;
    m_camera.setAspect(aspect);

    int fw = int(size().width() * m_devicePixelRatio);
    int fh = int(size().height() * m_devicePixelRatio);
    if (fw > 0 && fh > 0) {
        m_post.ensureSize(fw, fh);
    }
}

void Realtime::sceneChanged() {
    makeCurrent();

    m_particles.reset();

    bool ok = SceneParser::parse(settings.sceneFilePath, m_renderData);
    if (!ok) {
        std::cerr << "Failed to parse scene file: "
                  << settings.sceneFilePath << std::endl;
    } else {
        std::cout << "Loaded scene with "
                  << m_renderData.shapes.size() << " shapes and "
                  << m_renderData.lights.size() << " lights" << std::endl;

        float aspect = (height() > 0) ? float(width()) / float(height()) : 1.f;
        m_camera.setFromScene(m_renderData.cameraData, aspect);

        float nearPlane = settings.nearPlane;
        float farPlane  = settings.farPlane;
        if (nearPlane <= 0.f || nearPlane >= farPlane) {
            nearPlane = 0.1f;
            farPlane  = 50.f;
        }
        m_camera.setClipPlanes(nearPlane, farPlane);

        glm::mat4 baseView = m_camera.getViewMatrix();
        glm::mat4 camWorld = glm::inverse(baseView);

        glm::vec3 up   = glm::normalize(glm::vec3(camWorld[1]));
        glm::vec3 back = glm::normalize(glm::vec3(camWorld[2]));
        glm::vec3 pos  = glm::vec3(camWorld[3]);

        m_camPos  = pos;
        m_camUp   = up;
        m_camLook = -back;
        m_cameraInitialized = true;

        // Always refit the emitter to this scene's camera
        applyParticleEmitterFromSettings();

        rebuildSceneGeometry();
    }

    update();
}

void Realtime::settingsChanged() {
    float nearPlane = settings.nearPlane;
    float farPlane  = settings.farPlane;
    if (nearPlane <= 0.f || nearPlane >= farPlane) {
        nearPlane = 0.1f;
        farPlane  = 50.f;
    }
    m_camera.setClipPlanes(nearPlane, farPlane);

    makeCurrent();
    rebuildSceneGeometry();

    m_particles.setEnabled(settings.extraCredit1);
    applyParticleEmitterFromSettings();

    m_bloomEnabled = settings.extraCredit2;
    m_post.setBloomEnabled(m_bloomEnabled);
    m_post.setBloomStrength(settings.bloomStrength);

    update();
}

// ================== Camera Movement

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;

    // Post-Process Demo Purposes
    if (event->key() == Qt::Key_G) {
        m_postMode = (m_postMode == 2) ? 0 : 2; // toggle grayscale
        update();
    }
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown && m_cameraInitialized) {
        int posX = int(event->position().x());
        int posY = int(event->position().y());
        int deltaX = posX - int(m_prev_mouse_pos.x);
        int deltaY = posY - int(m_prev_mouse_pos.y);
        m_prev_mouse_pos = glm::vec2(posX, posY);

        const float sensitivity = 0.005f;
        float yaw   = -float(deltaX) * sensitivity;
        float pitch = -float(deltaY) * sensitivity;

        if (std::abs(yaw) > 1e-6f) {
            glm::mat4 R_yaw = glm::rotate(glm::mat4(1.f), yaw, glm::vec3(0.f, 1.f, 0.f));
            m_camLook = glm::vec3(R_yaw * glm::vec4(m_camLook, 0.f));
            m_camUp   = glm::vec3(R_yaw * glm::vec4(m_camUp,   0.f));
        }

        glm::vec3 right = glm::normalize(glm::cross(m_camLook, m_camUp));

        if (std::abs(pitch) > 1e-6f) {
            glm::mat4 R_pitch = glm::rotate(glm::mat4(1.f), pitch, right);
            m_camLook = glm::vec3(R_pitch * glm::vec4(m_camLook, 0.f));
            m_camUp   = glm::vec3(R_pitch * glm::vec4(m_camUp,   0.f));
        }

        m_camLook = glm::normalize(m_camLook);
        right     = glm::normalize(glm::cross(m_camLook, m_camUp));
        m_camUp   = glm::normalize(glm::cross(right, m_camLook));

        if (settings.extraCredit1) {
            applyParticleEmitterFromSettings();
        }

        update();
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    (void)event;

    if (m_cameraInitialized) {
        const float speed = 5.f;
        glm::vec3 move(0.f);

        glm::vec3 lookDir = glm::normalize(m_camLook);
        glm::vec3 upDir   = glm::normalize(m_camUp);
        glm::vec3 right   = glm::normalize(glm::cross(lookDir, upDir));

        if (m_keyMap[Qt::Key_W]) move += lookDir;
        if (m_keyMap[Qt::Key_S]) move -= lookDir;
        if (m_keyMap[Qt::Key_A]) move -= right;
        if (m_keyMap[Qt::Key_D]) move += right;
        if (m_keyMap[Qt::Key_Space])   move += glm::vec3(0.f, 1.f, 0.f);
        if (m_keyMap[Qt::Key_Control]) move -= glm::vec3(0.f, 1.f, 0.f);

        if (glm::length(move) > 1e-6f) {
            move = glm::normalize(move) * speed * deltaTime;
            m_camPos += move;

            if (settings.extraCredit1) {
                applyParticleEmitterFromSettings();
            }
        }
    }

    if (settings.extraCredit1) {
        m_particles.update(deltaTime);
    }

    update();
}

void Realtime::tick(QTimerEvent *event) {
    timerEvent(event);
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int fixedWidth = viewport[2];
    int fixedHeight = viewport[3];

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}
