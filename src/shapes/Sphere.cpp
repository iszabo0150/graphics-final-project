#include "Sphere.h"
#include <glm/gtc/constants.hpp>  // for glm::pi<float>(), glm::two_pi<float>()
#include <cmath>
#include <algorithm>

void Sphere::updateParams(int param1, int param2) {
    // Clear old data
    m_vertexData.clear();

    // Clamp to project-appropriate minimums:
    // - at least 2 stacks (vertical slices)
    // - at least 3 wedges (around the sphere)
    m_param1 = std::max(param1, 2);
    m_param2 = std::max(param2, 3);

    setVertexData();
}

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    // Sphere tile: position + normal = normalized position
    auto emit = [&](const glm::vec3 &p) {
        insertVec3(m_vertexData, p);
        insertVec3(m_vertexData, glm::normalize(p));
    };

    // Triangle 1: TL, BL, BR
    emit(topLeft);
    emit(bottomLeft);
    emit(bottomRight);

    // Triangle 2: TL, BR, TR
    emit(topLeft);
    emit(bottomRight);
    emit(topRight);
}

void Sphere::makeWedge(float currentTheta, float nextTheta) {
    // Build one longitudinal wedge between currentTheta and nextTheta,
    // split into m_param1 stacks from north to south.
    const int   S = m_param1;
    const float r = m_radius;

    auto sph = [&](float phi, float theta) -> glm::vec3 {
        float sinPhi   = std::sin(phi);
        float cosPhi   = std::cos(phi);
        float cosTheta = std::cos(theta);
        float sinTheta = std::sin(theta);

        return glm::vec3(r * sinPhi * cosTheta,
                         r * cosPhi,
                         r * sinPhi * sinTheta);
    };

    const float phiStep = glm::pi<float>() / float(S);  // 0..pi

    for (int j = 0; j < S; ++j) {
        float phi0 = j * phiStep;
        float phi1 = (j + 1) * phiStep;

        // CCW from outside: nextTheta on the left, currentTheta on the right
        glm::vec3 TL = sph(phi0, nextTheta);
        glm::vec3 TR = sph(phi0, currentTheta);
        glm::vec3 BL = sph(phi1, nextTheta);
        glm::vec3 BR = sph(phi1, currentTheta);

        makeTile(TL, TR, BL, BR);
    }
}

void Sphere::makeSphere() {
    // Sweep wedges all the way around [0, 2*pi)
    const int   wedges    = m_param2;
    const float thetaStep = glm::two_pi<float>() / float(wedges);

    for (int i = 0; i < wedges; ++i) {
        float currentTheta = i * thetaStep;
        float nextTheta    = (i + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
}

void Sphere::setVertexData() {
    // Full sphere using wedges
    makeSphere();
}

// Inserts a glm::vec3 into a vector of floats.
void Sphere::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
