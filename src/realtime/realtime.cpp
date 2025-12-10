#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"
#include "utils/sceneparser.h"
#include "shapes/shapetesselator.h"
#include "utils/shaderloader.h"

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

    // particles + post processing
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

    m_post.init(m_screen_width, m_screen_height);
    m_particles.initializeGL();

    // start disabled, toggle with extraCredit buttons
    m_post.setBloomEnabled(false);
    m_particles.setEnabled(false);

    m_shapeRenderer.initialize();
    m_sceneRenderer.initialize();
    m_lightRenderer.initialize(m_shapeRenderer);

    m_isInitialized = true;


}

void Realtime::paintGL() {
    // Students: anything requiring OpenGL calls every frame should be done here
    if (!m_camera) {
        return; // don't draw yet if the camera is undefined !
    }

    // framebuffer (screen = 0, screenshot FBO != 0)
    GLint targetFboInt = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &targetFboInt);
    GLuint targetFbo = static_cast<GLuint>(targetFboInt);

    // respect caller viewport size
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    int w = vp[2];
    int h = vp[3];

    const bool bloomEnabled = settings.extraCredit1;
    const bool particlesEnabled = settings.extraCredit2;

    // keep postprocess sized and in sync
    if (!m_post.ready()) m_post.init(w, h);
    else m_post.ensureSize(w, h);

    m_post.setBloomEnabled(bloomEnabled);

    if (m_post.ready()) {
        // scene pass into postprocess FBO
        m_post.beginScenePass(w, h);

        // shadow map + scene render into the postprocess FBO
        m_lightRenderer.render(m_renderData, static_cast<GLuint>(w), static_cast<GLuint>(h));
        m_sceneRenderer.render(m_renderData, *m_camera, m_shapeRenderer, m_lightRenderer.getShadow());

        // particle overlay in the same pass (so bloom and screenshots include it)
        if (particlesEnabled) {
            m_particles.render(m_camera->getViewMatrix(), m_camera->getProjMatrix());
        }

        // output to whatever framebuffer was originally bound
        m_post.endToTarget(targetFbo, w, h, bloomEnabled ? 3 : 0);
        return;
    }

    // fallback (should rarely happen)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_lightRenderer.render(m_renderData, static_cast<GLuint>(w), static_cast<GLuint>(h));
    m_sceneRenderer.render(m_renderData, *m_camera, m_shapeRenderer, m_lightRenderer.getShadow());
    if (particlesEnabled) {
        m_particles.render(m_camera->getViewMatrix(), m_camera->getProjMatrix());
    }
}

void Realtime::resizeGL(int w, int h) {
    // updating stored screen size and kept postprocess sized
    m_screen_width  = static_cast<GLuint>(w * m_devicePixelRatio);
    m_screen_height = static_cast<GLuint>(h * m_devicePixelRatio);

    glViewport(0, 0, m_screen_width, m_screen_height);

    if (m_post.ready()) {
        m_post.ensureSize(static_cast<int>(m_screen_width), static_cast<int>(m_screen_height));
    }
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

    update(); // asks for a PaintGL() call to occur
}


void Realtime::settingsChanged() {
    if (!m_isInitialized || !m_camera){
        return;
    }


    m_shapeRenderer.updateTessellation();
    m_lightRenderer.setShapes(m_shapeRenderer);
    m_camera->createProjectionMatrix();

    // toggles !
    m_post.setBloomEnabled(settings.extraCredit1);
    m_particles.setEnabled(settings.extraCredit2);

    update(); // asks for a PaintGL() call to occur
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

        // Use deltaX and deltaY here to rotate

        update(); // asks for a PaintGL() call to occur
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around

    if (m_keyMap[Qt::Key_W]){
        m_camera->translate(Direction::FORWARD, deltaTime);
    }

    if (m_keyMap[Qt::Key_A]){
        m_camera->translate(Direction::LEFT, deltaTime);
    }
    if (m_keyMap[Qt::Key_S]){
        m_camera->translate(Direction::BACKWARD, deltaTime);
    }
    if (m_keyMap[Qt::Key_D]){
        m_camera->translate(Direction::RIGHT, deltaTime);
    }
    if (m_keyMap[Qt::Key_Space]){
        m_camera->translate(Direction::UP, deltaTime);
    }
    if (m_keyMap[Qt::Key_Control]){
        m_camera->translate(Direction::DOWN, deltaTime);
    }

    // particles
    m_particles.update(deltaTime);

    update(); // asks for a PaintGL() call to occur
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
