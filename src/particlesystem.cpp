#include "particlesystem.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

#include "utils/shaderloader.h"

ParticleSystem::ParticleSystem()
    : m_dist(0.f, 1.f)
{
    std::random_device rd;
    m_rng.seed(rd());
}

void ParticleSystem::initializeGL() {
    // Compile shader
    try {
        m_program = ShaderLoader::createShaderProgram(
            ":/resources/shaders/particle.vert",
            ":/resources/shaders/particle.frag"
            );
    } catch (const std::runtime_error &e) {
        std::cerr << "[ParticleSystem] Shader error: " << e.what() << std::endl;
        m_program = 0;
        return;
    }

    // Create VAO/VBO
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // Start with empty buffer; we stream data each frame
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STREAM_DRAW);

    const GLsizei stride = 8 * sizeof(float);

    // layout(location=0) vec3 aPos;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

    // layout(location=1) vec4 aColor;
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

    // layout(location=2) float aSize;
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Cache uniforms
    glUseProgram(m_program);
    m_uView     = glGetUniformLocation(m_program, "u_View");
    m_uProj     = glGetUniformLocation(m_program, "u_Proj");
    m_uSoftness = glGetUniformLocation(m_program, "u_Softness");

    if (m_uSoftness >= 0) {
        glUniform1f(m_uSoftness, m_softness);
    }
    glUseProgram(0);

    // Needed for gl_PointSize from vertex shader in core profile
    glEnable(GL_PROGRAM_POINT_SIZE);
}

void ParticleSystem::destroyGL() {
    if (m_vbo) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_program) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
}

void ParticleSystem::setEnabled(bool enabled) { m_enabled = enabled; }
bool ParticleSystem::isEnabled() const { return m_enabled; }

void ParticleSystem::setEmitter(const Emitter& e) {
    m_emitter = e;
    m_particles.reserve(static_cast<size_t>(std::max(0, m_emitter.maxParticles)));
}

ParticleSystem::Emitter& ParticleSystem::emitter() { return m_emitter; }
const ParticleSystem::Emitter& ParticleSystem::emitter() const { return m_emitter; }

void ParticleSystem::setSeed(uint32_t seed) {
    m_rng.seed(seed);
}

void ParticleSystem::reset() {
    m_particles.clear();
    m_spawnAccumulator = 0.f;
    m_time = 0.f;
}

int ParticleSystem::aliveCount() const {
    return static_cast<int>(m_particles.size());
}

float ParticleSystem::rand01() {
    return m_dist(m_rng);
}

float ParticleSystem::randRange(float a, float b) {
    return a + (b - a) * rand01();
}

glm::vec3 ParticleSystem::randVec3(const glm::vec3& a, const glm::vec3& b) {
    return glm::vec3(
        randRange(a.x, b.x),
        randRange(a.y, b.y),
        randRange(a.z, b.z)
        );
}

void ParticleSystem::spawnOne() {
    if (static_cast<int>(m_particles.size()) >= m_emitter.maxParticles) return;

    Particle p;
    p.pos  = randVec3(m_emitter.spawnMin, m_emitter.spawnMax);

    glm::vec3 jitter = randVec3(-m_emitter.velocityJitter, m_emitter.velocityJitter);
    p.vel  = m_emitter.baseVelocity + jitter;

    p.age  = 0.f;
    p.life = std::max(0.01f, randRange(m_emitter.lifeMin, m_emitter.lifeMax));

    p.size = std::max(0.5f, randRange(m_emitter.sizeMin, m_emitter.sizeMax));
    p.seed = rand01();

    m_particles.push_back(p);
}

void ParticleSystem::update(float dt) {
    if (!m_enabled || !m_emitter.enabled) return;
    if (dt <= 0.f) return;

    m_time += dt;

    // Spawn new particles based on spawnRate
    m_spawnAccumulator += m_emitter.spawnRate * dt;
    int toSpawn = static_cast<int>(std::floor(m_spawnAccumulator));
    if (toSpawn > 0) {
        m_spawnAccumulator -= static_cast<float>(toSpawn);

        int room = m_emitter.maxParticles - static_cast<int>(m_particles.size());
        toSpawn = std::min(toSpawn, std::max(0, room));

        for (int i = 0; i < toSpawn; ++i) {
            spawnOne();
        }
    }

    // Update particles, swap-remove dead ones
    for (size_t i = 0; i < m_particles.size(); ) {
        Particle &p = m_particles[i];

        // Wind varies per-particle via its seed
        glm::vec3 wind(0.f);
        if (m_emitter.windStrength != 0.f) {
            float phase = m_time * m_emitter.windFrequency + p.seed * 10.f;
            float w = std::sin(phase) * m_emitter.windStrength;
            wind = m_emitter.windDir * w;
        }

        glm::vec3 accel = m_emitter.acceleration + wind;

        p.vel += accel * dt;

        if (m_emitter.drag > 0.f) {
            float damp = std::exp(-m_emitter.drag * dt);
            p.vel *= damp;
        }

        p.pos += p.vel * dt;
        p.age += dt;

        if (p.age >= p.life) {
            // kill: swap with last, pop back
            m_particles[i] = m_particles.back();
            m_particles.pop_back();
            continue;
        }

        ++i;
    }
}

void ParticleSystem::render(const glm::mat4& view, const glm::mat4& proj) {
    std::cout << "[ParticleSystem::render] enabled=" << m_enabled
              << " emitter=" << m_emitter.enabled
              << " program=" << m_program
              << " vao=" << m_vao
              << " vbo=" << m_vbo
              << " n=" << m_particles.size()
              << std::endl;

    if (!m_enabled || !m_emitter.enabled) return;
    if (m_program == 0 || m_vao == 0 || m_vbo == 0) return;
    if (m_particles.empty()) return;

    // Pack GPU data: pos(3), color(4), size(1)
    const size_t n = m_particles.size();
    m_gpuData.clear();
    m_gpuData.reserve(n * 8);

    for (const Particle &p : m_particles) {
        float t = p.age / p.life;
        t = std::clamp(t, 0.f, 1.f);

        // Simple color over life
        glm::vec4 c = glm::mix(m_emitter.colorStart, m_emitter.colorEnd, t);

        // Optional extra fade near the end feels nicer
        float fade = 1.f - t;
        c.a *= fade;

        m_gpuData.push_back(p.pos.x);
        m_gpuData.push_back(p.pos.y);
        m_gpuData.push_back(p.pos.z);

        m_gpuData.push_back(c.r);
        m_gpuData.push_back(c.g);
        m_gpuData.push_back(c.b);
        m_gpuData.push_back(c.a);

        m_gpuData.push_back(p.size);
    }

    glUseProgram(m_program);

    if (m_uView >= 0) glUniformMatrix4fv(m_uView, 1, GL_FALSE, glm::value_ptr(view));
    if (m_uProj >= 0) glUniformMatrix4fv(m_uProj, 1, GL_FALSE, glm::value_ptr(proj));
    if (m_uSoftness >= 0) glUniform1f(m_uSoftness, m_softness);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(m_gpuData.size() * sizeof(float)),
                 m_gpuData.data(),
                 GL_STREAM_DRAW);

    // DEBUG: force point size from shader + draw on top of everything
    glEnable(GL_PROGRAM_POINT_SIZE);

    GLboolean depthWasOn = glIsEnabled(GL_DEPTH_TEST);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(m_particles.size()));

    glDisable(GL_BLEND);
    if (depthWasOn) glEnable(GL_DEPTH_TEST);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}
