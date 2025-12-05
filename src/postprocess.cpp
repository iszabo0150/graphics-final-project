#include "postprocess.h"

#include <iostream>
#include <initializer_list>
#include <algorithm> // std::max, std::min

#include "utils/shaderloader.h"

static GLint getUniformAny(GLuint program, std::initializer_list<const char*> names) {
    for (const char* n : names) {
        GLint loc = glGetUniformLocation(program, n);
        if (loc >= 0) return loc;
    }
    return -1;
}

bool PostProcess::ready() const {
    return m_fbo != 0 && m_colorTex != 0 && m_depthRbo != 0 &&
           m_program != 0 && m_quadVao != 0;
}

bool PostProcess::bloomReady() const {
    if (!m_bloomEnabled) return false;

    return ready() &&
           m_bloomFbo != 0 && m_bloomTex != 0 &&
           m_pingpongFbo[0] != 0 && m_pingpongFbo[1] != 0 &&
           m_pingpongTex[0] != 0 && m_pingpongTex[1] != 0 &&
           m_brightProgram != 0 && m_blurProgram != 0 && m_combineProgram != 0;
}

void PostProcess::setBloomEnabled(bool enabled) {
    m_bloomEnabled = enabled;
}

void PostProcess::setBloomStrength(float s) {
    // Clamp to a sane range. Your UI can map 0..300 to 0..3, etc.
    m_bloomStrength = std::min(std::max(s, 0.f), 10.f);
}

void PostProcess::init(int w, int h) {
    destroy(); // safe re-init

    m_w = w;
    m_h = h;

    // ---------------- Main postprocess FBO ----------------
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_colorTex);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTex, 0);

    glGenRenderbuffers(1, &m_depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: PostProcess framebuffer incomplete.\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Compile post shader program (passthrough/invert/grayscale)
    try {
        m_program = ShaderLoader::createShaderProgram(
            ":/resources/shaders/fullscreen.vert",
            ":/resources/shaders/post.frag"
            );
    } catch (const std::runtime_error &e) {
        std::cerr << "PostProcess shader error: " << e.what() << "\n";
        m_program = 0;
    }

    if (m_program != 0) {
        glUseProgram(m_program);
        m_uScreenTex = getUniformAny(m_program, {"u_ScreenTex", "u_SceneTex", "u_Texture"});
        m_uMode      = getUniformAny(m_program, {"u_Mode", "u_mode"});
        if (m_uScreenTex >= 0) glUniform1i(m_uScreenTex, 0);
        glUseProgram(0);
    }

    // ---------------- Bloom targets ----------------
    glGenFramebuffers(1, &m_bloomFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_bloomFbo);

    glGenTextures(1, &m_bloomTex);
    glBindTexture(GL_TEXTURE_2D, m_bloomTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_bloomTex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Bloom framebuffer incomplete.\n";
    }

    glGenFramebuffers(2, m_pingpongFbo);
    glGenTextures(2, m_pingpongTex);

    for (int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFbo[i]);

        glBindTexture(GL_TEXTURE_2D, m_pingpongTex[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pingpongTex[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Error: Pingpong framebuffer " << i << " incomplete.\n";
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Bloom shaders
    try {
        m_brightProgram = ShaderLoader::createShaderProgram(
            ":/resources/shaders/fullscreen.vert",
            ":/resources/shaders/bloom_bright.frag"
            );
        m_blurProgram = ShaderLoader::createShaderProgram(
            ":/resources/shaders/fullscreen.vert",
            ":/resources/shaders/bloom_blur.frag"
            );
        m_combineProgram = ShaderLoader::createShaderProgram(
            ":/resources/shaders/fullscreen.vert",
            ":/resources/shaders/bloom_combine.frag"
            );
    } catch (const std::runtime_error &e) {
        std::cerr << "Bloom shader error: " << e.what() << "\n";
        m_brightProgram = 0;
        m_blurProgram = 0;
        m_combineProgram = 0;
    }

    // Bright uniforms
    if (m_brightProgram != 0) {
        glUseProgram(m_brightProgram);
        m_uBrightSceneTex = getUniformAny(m_brightProgram, {"u_SceneTex", "u_ScreenTex", "u_InputTex", "u_Texture"});
        m_uThreshold      = getUniformAny(m_brightProgram, {"u_Threshold", "u_threshold", "uBrightThreshold"});
        m_uSoftKnee       = getUniformAny(m_brightProgram, {"u_SoftKnee", "u_softKnee", "u_Knee"});
        if (m_uBrightSceneTex >= 0) glUniform1i(m_uBrightSceneTex, 0);
        glUseProgram(0);
    }

    // Blur uniforms
    if (m_blurProgram != 0) {
        glUseProgram(m_blurProgram);
        m_uBlurInputTex = getUniformAny(m_blurProgram, {"u_InputTex", "u_SceneTex", "u_Texture", "u_SrcTex"});
        m_uTexelStep    = getUniformAny(m_blurProgram, {"u_TexelStep", "u_TexelSize", "u_Direction", "u_Step"});
        m_uHorizontal   = getUniformAny(m_blurProgram, {"u_Horizontal", "u_horizontal", "u_IsHorizontal"});
        if (m_uBlurInputTex >= 0) glUniform1i(m_uBlurInputTex, 0);
        glUseProgram(0);
    }

    // Combine uniforms
    if (m_combineProgram != 0) {
        glUseProgram(m_combineProgram);
        m_uCombineSceneTex = getUniformAny(m_combineProgram, {"u_SceneTex", "u_ScreenTex", "u_Texture"});
        m_uCombineBloomTex = getUniformAny(m_combineProgram, {"u_BloomTex", "u_BlurTex", "u_Bloom"});
        m_uBloomStrength   = getUniformAny(m_combineProgram, {"u_Strength", "u_BloomStrength", "u_Intensity", "u_BloomIntensity"});
        if (m_uCombineSceneTex >= 0) glUniform1i(m_uCombineSceneTex, 0);
        if (m_uCombineBloomTex >= 0) glUniform1i(m_uCombineBloomTex, 1);
        glUseProgram(0);
    }

    // If any of these are -1, bloom might "do nothing" even though it runs.
    if (m_brightProgram != 0 && (m_uThreshold < 0 || m_uSoftKnee < 0)) {
        std::cerr << "[Bloom] Warning: threshold/knee uniforms not found. Check bloom_bright.frag uniform names.\n";
    }
    if (m_combineProgram != 0 && m_uBloomStrength < 0) {
        std::cerr << "[Bloom] Warning: strength uniform not found. Check bloom_combine.frag uniform name.\n";
    }

    createFullscreenQuad();
}

void PostProcess::destroy() {
    destroyFullscreenQuad();

    // Base post shader
    if (m_program != 0) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
    m_uScreenTex = -1;
    m_uMode = -1;

    // Bloom shaders
    if (m_brightProgram != 0) {
        glDeleteProgram(m_brightProgram);
        m_brightProgram = 0;
    }
    if (m_blurProgram != 0) {
        glDeleteProgram(m_blurProgram);
        m_blurProgram = 0;
    }
    if (m_combineProgram != 0) {
        glDeleteProgram(m_combineProgram);
        m_combineProgram = 0;
    }

    m_uBrightSceneTex = -1;
    m_uThreshold = -1;
    m_uSoftKnee = -1;

    m_uBlurInputTex = -1;
    m_uTexelStep = -1;
    m_uHorizontal = -1;

    m_uCombineSceneTex = -1;
    m_uCombineBloomTex = -1;
    m_uBloomStrength = -1;

    // Base FBO
    if (m_colorTex != 0) {
        glDeleteTextures(1, &m_colorTex);
        m_colorTex = 0;
    }
    if (m_depthRbo != 0) {
        glDeleteRenderbuffers(1, &m_depthRbo);
        m_depthRbo = 0;
    }
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }

    // Bloom FBOs/textures
    if (m_bloomTex != 0) {
        glDeleteTextures(1, &m_bloomTex);
        m_bloomTex = 0;
    }
    if (m_bloomFbo != 0) {
        glDeleteFramebuffers(1, &m_bloomFbo);
        m_bloomFbo = 0;
    }

    if (m_pingpongTex[0] != 0 || m_pingpongTex[1] != 0) {
        glDeleteTextures(2, m_pingpongTex);
    }
    if (m_pingpongFbo[0] != 0 || m_pingpongFbo[1] != 0) {
        glDeleteFramebuffers(2, m_pingpongFbo);
    }
    m_pingpongTex[0] = 0; m_pingpongTex[1] = 0;
    m_pingpongFbo[0] = 0; m_pingpongFbo[1] = 0;

    m_w = 0;
    m_h = 0;
}

void PostProcess::ensureSize(int w, int h) {
    if (!ready()) return;
    if (w == m_w && h == m_h) return;
    resizeAttachments(w, h);
}

void PostProcess::resizeAttachments(int w, int h) {
    m_w = w;
    m_h = h;

    // Base attachments
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);

    // Bloom attachments
    if (m_bloomTex != 0) {
        glBindTexture(GL_TEXTURE_2D, m_bloomTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    for (int i = 0; i < 2; i++) {
        if (m_pingpongTex[i] != 0) {
            glBindTexture(GL_TEXTURE_2D, m_pingpongTex[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void PostProcess::beginScenePass(int w, int h) {
    if (!ready()) return;

    ensureSize(w, h);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, w, h);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcess::endToTarget(GLuint targetFbo, int w, int h, int mode) {
    if (!ready()) return;

    // mode 3 = bloom
    if (mode == 3) {
        static bool warned = false;
        if (!bloomReady()) {
            if (!warned) {
                std::cerr << "[Bloom] Not running. bloomEnabled=" << (m_bloomEnabled ? 1 : 0)
                << " bloomReady=" << (bloomReady() ? 1 : 0) << "\n";
                warned = true;
            }
            mode = 0; // fallback to passthrough
        } else {
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);

            // LDR-friendly defaults (so you actually see something)
            const float threshold = 0.02f;
            const float softKnee  = 0.50f;
            const float strength  = m_bloomStrength; // THIS is now driven by your slider
            const int   blurPasses = 10;

            // 1) Bright pass: m_colorTex -> m_bloomTex
            glBindFramebuffer(GL_FRAMEBUFFER, m_bloomFbo);
            glViewport(0, 0, w, h);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(m_brightProgram);
            if (m_uThreshold >= 0) glUniform1f(m_uThreshold, threshold);
            if (m_uSoftKnee >= 0)  glUniform1f(m_uSoftKnee,  softKnee);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_colorTex);

            glBindVertexArray(m_quadVao);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // 2) Blur ping-pong: m_bloomTex -> pingpong textures
            glUseProgram(m_blurProgram);

            GLuint inputTex = m_bloomTex;
            bool horizontal = true;

            for (int i = 0; i < blurPasses; i++) {
                int dst = horizontal ? 1 : 0;

                glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFbo[dst]);
                glViewport(0, 0, w, h);
                glClear(GL_COLOR_BUFFER_BIT);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, inputTex);

                // If your shader uses a bool horizontal, set it.
                if (m_uHorizontal >= 0) {
                    glUniform1i(m_uHorizontal, horizontal ? 1 : 0);
                }

                // If your shader uses a vec2 step/direction, set it.
                float dx = horizontal ? (1.0f / float(w)) : 0.0f;
                float dy = horizontal ? 0.0f : (1.0f / float(h));
                if (m_uTexelStep >= 0) glUniform2f(m_uTexelStep, dx, dy);

                glBindVertexArray(m_quadVao);
                glDrawArrays(GL_TRIANGLES, 0, 6);

                inputTex = m_pingpongTex[dst];
                horizontal = !horizontal;
            }

            // 3) Combine: m_colorTex + blurred bloom -> targetFbo
            glBindFramebuffer(GL_FRAMEBUFFER, targetFbo);
            glViewport(0, 0, w, h);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(m_combineProgram);
            if (m_uBloomStrength >= 0) glUniform1f(m_uBloomStrength, strength);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_colorTex);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, inputTex);

            glBindVertexArray(m_quadVao);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glBindVertexArray(0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
            glUseProgram(0);
            return;
        }
    }

    // Default post modes (0 passthrough, 1 invert, 2 grayscale)
    glBindFramebuffer(GL_FRAMEBUFFER, targetFbo);
    glViewport(0, 0, w, h);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_program);

    if (m_uMode >= 0) {
        glUniform1i(m_uMode, mode);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);

    glBindVertexArray(m_quadVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void PostProcess::createFullscreenQuad() {
    float quad[] = {
        // pos      // uv
        -1.f, -1.f,  0.f, 0.f,
        1.f, -1.f,  1.f, 0.f,
        1.f,  1.f,  1.f, 1.f,

        -1.f, -1.f,  0.f, 0.f,
        1.f,  1.f,  1.f, 1.f,
        -1.f,  1.f,  0.f, 1.f
    };

    glGenVertexArrays(1, &m_quadVao);
    glGenBuffers(1, &m_quadVbo);

    glBindVertexArray(m_quadVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void PostProcess::destroyFullscreenQuad() {
    if (m_quadVbo != 0) {
        glDeleteBuffers(1, &m_quadVbo);
        m_quadVbo = 0;
    }
    if (m_quadVao != 0) {
        glDeleteVertexArrays(1, &m_quadVao);
        m_quadVao = 0;
    }
}
