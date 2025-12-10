#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <GL/glew.h>

class PostProcess
{
public:
    PostProcess() = default;
    ~PostProcess() = default;

    // Call init/destroy only when an OpenGL context is current.
    void init(int w, int h);
    void destroy();

    void setBloomEnabled(bool enabled);

    // New: bloom intensity multiplier used in the combine pass.
    void setBloomStrength(float s);
    float bloomStrength() const { return m_bloomStrength; }

    // Safe to call every frame. If size differs, resizes attachments.
    void ensureSize(int w, int h);

    bool ready() const;
    bool bloomReady() const;

    // Pass 1: bind internal FBO, set viewport, clear, enable depth/cull.
    void beginScenePass(int w, int h);

    // Pass 2: bind targetFbo (0 for screen or screenshot FBO), draw fullscreen quad.
    // mode: 0 passthrough, 1 invert, 2 grayscale, 3 bloom
    void endToTarget(GLuint targetFbo, int w, int h, int mode);

private:
    void resizeAttachments(int w, int h);
    void createFullscreenQuad();
    void destroyFullscreenQuad();

    GLuint m_fbo = 0;
    GLuint m_colorTex = 0;
    GLuint m_depthRbo = 0;
    int m_w = 0;
    int m_h = 0;

    GLuint m_program = 0;
    GLint  m_uScreenTex = -1;
    GLint  m_uMode = -1;

    GLuint m_quadVao = 0;
    GLuint m_quadVbo = 0;

    // Bloom
    GLuint m_bloomFbo = 0;
    GLuint m_bloomTex = 0;

    GLuint m_pingpongFbo[2] = {0, 0};
    GLuint m_pingpongTex[2] = {0, 0};

    GLuint m_brightProgram  = 0;
    GLuint m_blurProgram    = 0;
    GLuint m_combineProgram = 0;

    GLint m_uBrightSceneTex = -1;
    GLint m_uThreshold      = -1;
    GLint m_uSoftKnee       = -1;

    GLint m_uBlurInputTex   = -1;
    GLint m_uTexelStep      = -1;
    GLint m_uHorizontal     = -1;

    GLint m_uCombineSceneTex = -1;
    GLint m_uCombineBloomTex = -1;
    GLint m_uBloomStrength   = -1;

    bool  m_bloomEnabled = false;
    float m_bloomStrength = 1.0f;
};
