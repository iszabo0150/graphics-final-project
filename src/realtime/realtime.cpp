#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "utils/sceneparser.h"
#include "shapes/shapetesselator.h"
#include "utils/shaderloader.h"

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

void Realtime::applyParticleEmitterFromSettings(bool resetParticles) {
    auto &e = m_particles.emitter();

    if (settings.particlesSpring) {
        e = makeSpringRainEmitter();
    } else if (settings.particlesSummer) {
        e = makeSummerFireflyEmitter();
    } else if (settings.particlesAutumn) {
        e = makeAutumnLeavesEmitter();
    } else {
        // default to winter
        e = makeWinterSnowEmitter();
    }

    // This will also reset if resetParticles == true.
    fitEmitterToCamera(resetParticles);
}

void Realtime::fitEmitterToCamera(bool resetParticles) {
    if (!m_camera) return;

    // Camera world transform from view matrix
    glm::mat4 view = m_camera->getViewMatrix();
    glm::mat4 camWorld = glm::inverse(view);

    glm::vec3 camPos = glm::vec3(camWorld[3]);
    glm::vec3 up     = glm::normalize(glm::vec3(camWorld[1]));
    glm::vec3 back   = glm::normalize(glm::vec3(camWorld[2]));
    glm::vec3 forward = -back;

    // Pick a spawn volume in front of the camera
    float nearPlane = settings.nearPlane;
    float farPlane  = settings.farPlane;
    if (nearPlane <= 0.f || nearPlane >= farPlane) {
        nearPlane = 0.1f;
        farPlane  = 50.f;
    }

    float dist = clampf(farPlane * 0.15f, 2.0f, 12.0f);   // how far in front
    float base = clampf(farPlane * 0.20f, 2.5f, 18.0f);   // box half-size

    glm::vec3 center = camPos + forward * dist + up * (base * 0.35f);

    float ex = base * 1.25f;
    float ey = base * 0.95f;
    float ez = base * 1.25f;

    // These names assume your ParticleSystem has the same emitter API as gear-up
    auto &e = m_particles.emitter();
    e.spawnMin = center + glm::vec3(-ex, -ey, -ez);
    e.spawnMax = center + glm::vec3( ex,  ey,  ez);

    if (resetParticles) {
        m_particles.reset();
    }
}

// ================== Rendering the Scene!

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    // If you must use this function, do not edit anything above this
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    m_shapeRenderer.cleanup();
    m_sceneRenderer.cleanup();
    m_crepuscularRenderer.cleanup();
    m_screenRenderer.cleanup();

    m_particles.destroyGL();
    m_post.destroy();

    this->doneCurrent();
}

void Realtime::initializeGL() {
    std::cout << "initializeGL: start" << std::endl;

    m_devicePixelRatio = this->devicePixelRatio();

    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);


    // Students: anything requiring OpenGL calls when the program starts should be done here
    m_texture_shader = ShaderLoader::createShaderProgram(":/resources/shaders/texture.vert", ":/resources/shaders/texture.frag");

    m_post.init(m_screen_width, m_screen_height);
    m_particles.initializeGL();

    // Start disabled, you’ll toggle via extraCredit buttons
    m_post.setBloomEnabled(false);
    m_particles.setEnabled(false);
    GLuint defaultFBO = defaultFramebufferObject();
    m_defaultFBO = defaultFBO;

    m_shapeRenderer.initialize();
    m_sceneRenderer.setDefaultFBO(defaultFBO);
    m_sceneRenderer.initialize(m_texture_shader);
    m_lightRenderer.initialize(&m_shapeRenderer, m_texture_shader);

    m_crepuscularRenderer.initialize(1.0f, 1.0f, 0.5f, 0.01f, 100);
    m_crepuscularRenderer.setDefaultFBO(defaultFBO);

    m_screenRenderer.initialize();

    m_isInitialized = true;
    m_enableCrepuscular = true;

}

void Realtime::paintGL() {
    // Students: anything requiring OpenGL calls every frame should be done here
    if (!m_camera) {
        return; // don't draw yet if the camera is undefined !
    }

    // Capture whatever framebuffer is currently bound (screen or screenshot FBO)
    GLint targetFboInt = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &targetFboInt);
    GLuint targetFbo = static_cast<GLuint>(targetFboInt);

    // Use current viewport size if it differs from m_screen_width/height
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    int w = (vp[2] > 0) ? vp[2] : static_cast<int>(m_screen_width);
    int h = (vp[3] > 0) ? vp[3] : static_cast<int>(m_screen_height);

    const bool particlesEnabled = settings.extraCredit1;
    const bool bloomEnabled = settings.extraCredit2;

    // Ensure postprocess exists and matches our target size
    if (!m_post.ready()) m_post.init(w, h);
    else m_post.ensureSize(w, h);
    m_post.setBloomEnabled(bloomEnabled);

    // PARTICLE PIPELINE ========
    // If postprocess is ready, render the whole scene into its FBO, then composite to targetFbo
    if (m_post.ready()) {

        m_post.beginScenePass(w, h); // binds internal FBO and clears color+depth

        // Clear screen color and depth before painting
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render the scene from the light's perspective to get shadow map
        m_lightRenderer.render(m_renderData, static_cast<GLuint>(w), static_cast<GLuint>(h));

        //render the scene based on render data !!
        m_sceneRenderer.render(m_renderData, *m_camera, m_shapeRenderer, m_lightRenderer.getShadow());

        // Particles last so they overlay everything (and do not get overwritten by a skybox pass)
        if (particlesEnabled) {
            m_particles.render(m_camera->getViewMatrix(), m_camera->getProjMatrix());
        }

        // Composite the postprocess result to the originally bound framebuffer
        m_post.endToTarget(targetFbo, w, h, bloomEnabled ? 3 : 0);
        return;

    }

    // Fallback path if postprocess is not ready
    // Clear screen color and depth before painting
    int width = size().width() * m_devicePixelRatio;
    int height = size().height() * m_devicePixelRatio;

    // occlusion pre-pass for crepuscular rays
    if (m_enableCrepuscular) {

        m_crepuscularRenderer.renderOcclusion(
            m_camera->getViewMatrix(), m_camera->getProjMatrix(),
            width, height, m_renderData, m_shapeRenderer
        );

        // reset viewport to full size for main scene render (occlusion pass reduced it)
        glViewport(0, 0, width, height);
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

    }

    // clear screen color and depth before painting, disable blend if active
    glDisable(GL_BLEND);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render the scene from the light's perspective to get shadow map
    m_lightRenderer.render(m_renderData, m_screen_width, m_screen_height);

    //render the scene based on render data !!

    // Render scene first
    m_sceneRenderer.render(m_renderData, *m_camera, m_shapeRenderer, m_lightRenderer.getShadow());

    // add terrain into the scene FBO before presenting
    m_sceneRenderer.paintTerrain(*m_camera);

    // copy scene (with skybox/terrain) to screen
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    m_screenRenderer.renderToScreen(m_sceneRenderer.getSceneTexture(), 
                                    width, height);

    if (particlesEnabled) {
        m_particles.render(m_camera->getViewMatrix(), m_camera->getProjMatrix());
    }
    
    // if god rays enabled, blend them on top
    if (m_enableCrepuscular) {

        glm::mat4 view = m_camera->getViewMatrix();
        glm::mat4 proj = m_camera ->getProjMatrix();

        // additive blending for god rays
        glDisable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glEnable(GL_BLEND);

        m_crepuscularRenderer.blendCrepuscular(
            view, proj,
            width, height, m_renderData
        );

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);

    }

}



void Realtime::resizeGL(int w, int h) {
    m_screen_width  = static_cast<GLuint>(w * m_devicePixelRatio);
    m_screen_height = static_cast<GLuint>(h * m_devicePixelRatio);

    glViewport(0, 0, m_screen_width, m_screen_height);

    if (m_post.ready()) {
        m_post.ensureSize(static_cast<int>(m_screen_width), static_cast<int>(m_screen_height));
    }
    // Students: anything requiring OpenGL calls when the program starts should be done here
    m_sceneRenderer.resize(w * m_devicePixelRatio, h * m_devicePixelRatio);
    
}

void Realtime::sceneChanged() {

    m_renderData.lights.clear();

    SceneParser::parse(settings.sceneFilePath, m_renderData);

    int width = size().width() * m_devicePixelRatio;
    int height = size().height() * m_devicePixelRatio;


    m_camera = std::make_unique<Camera>(glm::vec3(m_renderData.cameraData.pos),
                                        glm::vec3(m_renderData.cameraData.look),
                                        glm::vec3(m_renderData.cameraData.up),
                                        m_renderData.cameraData.heightAngle,
                                        width,
                                        height);

    fitEmitterToCamera(true);


    update(); // asks for a PaintGL() call to occur
}


void Realtime::settingsChanged() {
    if (!m_isInitialized || !m_camera){
        return;
    }


    m_shapeRenderer.updateTessellation();
    m_lightRenderer.setShapes(&m_shapeRenderer);
    m_camera->createProjectionMatrix();

    const bool nowParticles = settings.extraCredit1;
    m_particles.setEnabled(nowParticles);

    // Track season + toggle transitions
    static bool wasParticles = false;

    auto seasonId = []() {
        if (settings.particlesSpring) return 1;
        if (settings.particlesSummer) return 2;
        if (settings.particlesAutumn) return 3;
        return 0; // winter default
    };

    static int lastSeason = seasonId();
    int curSeason = seasonId();

    bool turnedOn = nowParticles && !wasParticles;
    bool seasonChanged = (curSeason != lastSeason);

    if (nowParticles && (turnedOn || seasonChanged)) {
        applyParticleEmitterFromSettings(true);
    }

    wasParticles = nowParticles;
    lastSeason = curSeason;

    m_post.setBloomEnabled(settings.extraCredit2);

    update();
}



// ================== Camera Movement!

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
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
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        m_camera->rotate(deltaX, deltaY);

        if (settings.extraCredit1) { // particles toggle
            fitEmitterToCamera(false);
        }

        // Use deltaX and deltaY here to rotate

        update(); // asks for a PaintGL() call to occur
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    bool moved = false;

    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around

    if (m_keyMap[Qt::Key_W]){
        m_camera->translate(Direction::FORWARD, deltaTime);
        moved = true;

    }

    if (m_keyMap[Qt::Key_A]){
        m_camera->translate(Direction::LEFT, deltaTime);
        moved = true;

    }
    if (m_keyMap[Qt::Key_S]){
        m_camera->translate(Direction::BACKWARD, deltaTime);
        moved = true;

    }
    if (m_keyMap[Qt::Key_D]){
        m_camera->translate(Direction::RIGHT, deltaTime);
        moved = true;

    }
    if (m_keyMap[Qt::Key_Space]){
        m_camera->translate(Direction::UP, deltaTime);
        moved = true;

    }
    if (m_keyMap[Qt::Key_Control]){
        m_camera->translate(Direction::DOWN, deltaTime);
        moved = true;

    }

    if (moved && settings.extraCredit1) {
        fitEmitterToCamera(false);
    }

    const bool particlesOn = settings.extraCredit1;
    const bool bloomOn = settings.extraCredit2;

    if (particlesOn) {
        m_particles.update(deltaTime);
    }

    // Redraw if anything animated is on, or if the camera moved
    if (moved || particlesOn || bloomOn) {
        update();
    }
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    // int fixedWidth = 1024;
    // int fixedHeight = 768;

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
