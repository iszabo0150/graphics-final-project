#include "Cylinder.h"
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <algorithm>

void Cylinder::updateParams(int param1, int param2) {
    m_vertexData.clear();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

// Inserts a glm::vec3 into a vector of floats.
void Cylinder::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void Cylinder::setVertexData() {
    m_vertexData.clear();

    int stacks = std::max(1, m_param1);
    int slices = std::max(3, m_param2);

    const float r       = m_radius;   // 0.5
    const float yTop    =  0.5f;
    const float yBottom = -0.5f;
    const float height  = yTop - yBottom;

    float thetaStep = glm::two_pi<float>() / static_cast<float>(slices);

    auto emitVertex = [&](const glm::vec3 &p, const glm::vec3 &n) {
        insertVec3(m_vertexData, p);
        insertVec3(m_vertexData, glm::normalize(n));
    };

    auto circlePos = [&](float radius, float y, float theta) -> glm::vec3 {
        return glm::vec3(radius * std::cos(theta),
                         y,
                         radius * std::sin(theta));
    };

    // ============================================================
    // Top cap. y = +0.5, normal = +Y.
    // Simple triangle fan: center plus ring at radius r.
    // For CCW from above, use (center, next, curr).
    // ============================================================
    {
        const glm::vec3 N(0.f, 1.f, 0.f);
        const glm::vec3 C(0.f, yTop, 0.f);

        for (int i = 0; i < slices; ++i) {
            float curr = i * thetaStep;
            float next = (i + 1) * thetaStep;

            glm::vec3 Pcurr = circlePos(r, yTop, curr);
            glm::vec3 Pnext = circlePos(r, yTop, next);

            // CCW when viewed from +Y
            emitVertex(C,     N);
            emitVertex(Pnext, N);
            emitVertex(Pcurr, N);
        }
    }

    // ============================================================
    // Bottom cap. y = -0.5, normal = -Y.
    // Triangle fan with front faces pointing downward.
    // That means CCW when viewed from below. From above they are CW.
    // Using (center, curr, next) gives a normal of -Y.
    // ============================================================
    {
        const glm::vec3 N(0.f, -1.f, 0.f);
        const glm::vec3 C(0.f, yBottom, 0.f);

        for (int i = 0; i < slices; ++i) {
            float curr = i * thetaStep;
            float next = (i + 1) * thetaStep;

            glm::vec3 Pcurr = circlePos(r, yBottom, curr);
            glm::vec3 Pnext = circlePos(r, yBottom, next);

            // CCW when viewed from -Y
            emitVertex(C,     N);
            emitVertex(Pcurr, N);
            emitVertex(Pnext, N);
        }
    }

    // ============================================================
    // Side surface. Same logic as you had before.
    // ============================================================
    {
        auto sidePos = [&](float y, float theta) -> glm::vec3 {
            return glm::vec3(r * std::cos(theta),
                             y,
                             r * std::sin(theta));
        };

        auto sideNormal = [&](float theta) -> glm::vec3 {
            return glm::vec3(std::cos(theta), 0.f, std::sin(theta));
        };

        for (int i = 0; i < slices; ++i) {
            float curr = i * thetaStep;
            float next = (i + 1) * thetaStep;

            glm::vec3 Ncurr = sideNormal(curr);
            glm::vec3 Nnext = sideNormal(next);

            for (int j = 0; j < stacks; ++j) {
                float t0 = static_cast<float>(j)     / stacks;
                float t1 = static_cast<float>(j + 1) / stacks;

                float y0 = yTop - t0 * height; // top -> bottom
                float y1 = yTop - t1 * height;

                // "next" on left, "current" on right
                glm::vec3 TL = sidePos(y0, next);
                glm::vec3 TR = sidePos(y0, curr);
                glm::vec3 BL = sidePos(y1, next);
                glm::vec3 BR = sidePos(y1, curr);

                // tri 1
                emitVertex(TL, Nnext);
                emitVertex(BL, Nnext);
                emitVertex(BR, Ncurr);

                // tri 2
                emitVertex(TL, Nnext);
                emitVertex(BR, Ncurr);
                emitVertex(TR, Ncurr);
            }
        }
    }
}
