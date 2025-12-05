#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <vector>
#include <random>
#include <cstdint>

class ParticleSystem {
public:
    struct Emitter {
        bool enabled = true;

        // Limits and spawning
        int   maxParticles = 20000;
        float spawnRate    = 2000.f; // particles per second

        // Spawn region (axis-aligned box in world space)
        glm::vec3 spawnMin = glm::vec3(-1.f,  1.f, -1.f);
        glm::vec3 spawnMax = glm::vec3( 1.f,  1.f,  1.f);

        // Motion
        glm::vec3 baseVelocity   = glm::vec3(0.f, -1.2f, 0.f);
        glm::vec3 velocityJitter = glm::vec3(0.4f, 0.2f, 0.4f);

        glm::vec3 acceleration   = glm::vec3(0.f, -0.2f, 0.f); // gravity-ish
        float     drag           = 0.0f; // 1/sec, 0 disables

        // Lifetime
        float lifeMin = 2.0f;
        float lifeMax = 5.0f;

        // Visuals (GL_POINTS)
        float sizeMin = 3.0f; // pixels
        float sizeMax = 8.0f; // pixels

        glm::vec4 colorStart = glm::vec4(1.f, 1.f, 1.f, 0.9f);
        glm::vec4 colorEnd   = glm::vec4(1.f, 1.f, 1.f, 0.0f);

        // Simple oscillating wind term
        glm::vec3 windDir       = glm::vec3(1.f, 0.f, 0.f);
        float     windStrength  = 0.0f;
        float     windFrequency = 1.0f;
    };

    ParticleSystem();

    // Call after an OpenGL context is current (after glewInit in Realtime::initializeGL)
    void initializeGL();
    void destroyGL();

    void setEnabled(bool enabled);
    bool isEnabled() const;

    void setEmitter(const Emitter& e);
    Emitter& emitter();
    const Emitter& emitter() const;

    void setSeed(uint32_t seed);
    void reset();

    void update(float dt);
    void render(const glm::mat4& view, const glm::mat4& proj);

    int aliveCount() const;

private:
    struct Particle {
        glm::vec3 pos = glm::vec3(0.f);
        glm::vec3 vel = glm::vec3(0.f);
        float age  = 0.f;
        float life = 1.f;
        float size = 6.f;   // pixels
        float seed = 0.f;   // random 0..1, used for wind phase variation
    };

    void  spawnOne();
    float rand01();
    float randRange(float a, float b);
    glm::vec3 randVec3(const glm::vec3& a, const glm::vec3& b);

    Emitter m_emitter;
    bool    m_enabled = true;

    std::vector<Particle> m_particles; // alive-only, swap-remove on death

    float m_spawnAccumulator = 0.f; // fractional spawn accumulator
    float m_time             = 0.f;

    std::mt19937 m_rng;
    std::uniform_real_distribution<float> m_dist;

    // GPU side
    GLuint m_vao     = 0;
    GLuint m_vbo     = 0;
    GLuint m_program = 0;

    GLint m_uView     = -1;
    GLint m_uProj     = -1;
    GLint m_uSoftness = -1;

    float m_softness = 0.08f; // edge feather for circular mask

    // Packed per-vertex data: pos(3), color(4), size(1) = 8 floats
    std::vector<float> m_gpuData;
};
