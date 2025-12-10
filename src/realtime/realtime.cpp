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
    m_crepuscularRenderer.cleanup();
    m_screenRenderer.cleanup();

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
    m_sceneRenderer.render(m_renderData, *m_camera, m_shapeRenderer, m_lightRenderer.getShadow());

    // add terrain into the scene FBO before presenting
    m_sceneRenderer.paintTerrain(*m_camera);

    // copy scene (with skybox/terrain) to screen
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    m_screenRenderer.renderToScreen(m_sceneRenderer.getSceneTexture(), 
                                    width, height);

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
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

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

    update(); // asks for a PaintGL() call to occur
}


void Realtime::settingsChanged() {
    if (!m_isInitialized || !m_camera){
        return;
    }


    m_shapeRenderer.updateTessellation();
    m_lightRenderer.setShapes(&m_shapeRenderer);
    m_camera->createProjectionMatrix();

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

    if (moved) {
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
