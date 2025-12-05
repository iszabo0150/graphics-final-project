#include "Cone.h"

void Cone::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

// Task 8: create function(s) to make tiles which you can call later on
// Note: Consider your makeTile() functions from Sphere and Cube

void Cone::makeCapSlice(float currentTheta, float nextTheta){
    // Task 8: create a slice of the cap face using your
    //         make tile function(s)
    // Note: think about how param 1 comes into play here!
    const int R = std::max(1, m_param1);   // radial subdivisions
    const float y = -0.5f;                 // cap plane (cone points up, base at y = -0.5)
    const glm::vec3 Ncap(0.f, -1.f, 0.f);  // outward normal for the cap

    auto pos = [&](float radius, float theta) -> glm::vec3 {
        return glm::vec3(radius * std::cos(theta), y, radius * std::sin(theta));
    };

    // emit a flat tile (two triangles) with a constant normal
    auto emitFlatTile = [&](const glm::vec3& TL, const glm::vec3& TR,
                            const glm::vec3& BL, const glm::vec3& BR) {
        // tri 1: TL, BL, BR
        insertVec3(m_vertexData, TL); insertVec3(m_vertexData, Ncap);
        insertVec3(m_vertexData, BL); insertVec3(m_vertexData, Ncap);
        insertVec3(m_vertexData, BR); insertVec3(m_vertexData, Ncap);
        // tri 2: TL, BR, TR
        insertVec3(m_vertexData, TL); insertVec3(m_vertexData, Ncap);
        insertVec3(m_vertexData, BR); insertVec3(m_vertexData, Ncap);
        insertVec3(m_vertexData, TR); insertVec3(m_vertexData, Ncap);
    };

    // radial rings from center (0) to rim (0.5). Use CCW as seen from below (outside).
    for (int j = 0; j < R; ++j) {
        float r0 = (static_cast<float>(j)     / R) * 0.5f; // inner
        float r1 = (static_cast<float>(j + 1) / R) * 0.5f; // outer

        // CCW from outside (below): put "nextTheta" on the left, "currentTheta" on the right
        glm::vec3 TL = pos(r0, currentTheta);
        glm::vec3 TR = pos(r0, nextTheta);
        glm::vec3 BL = pos(r1, currentTheta);
        glm::vec3 BR = pos(r1, nextTheta);

        emitFlatTile(TL, TR, BL, BR);
    }
}

void Cone::makeSlopeSlice(float currentTheta, float nextTheta){
    // Task 9: create a single sloped face using your make
    //         tile function(s)
    // Note: think about how param 1 comes into play here!
    const int S = std::max(1, m_param1);
    const float H = 1.0f;
    const float R = 0.5f;

    auto pos = [&](float t, float theta) -> glm::vec3 {
        float y = 0.5f - t * H; // apex +0.5 -> base -0.5
        float r = R * t;
        return glm::vec3(r * std::cos(theta), y, r * std::sin(theta));
    };

    auto coneNormal = [](const glm::vec3& p) -> glm::vec3 {
        float xNorm = 2.f * p.x;
        float yNorm = -(1.f/4.f) * (2.f * p.y - 1.f);
        float zNorm = 2.f * p.z;
        return glm::vec3{xNorm, yNorm, zNorm}; // not normalized yet
    };

    auto fallbackNormal = [&](float theta) -> glm::vec3 {
        // side normal for a right cone (limit as we approach the apex)
        return glm::normalize(glm::vec3(std::cos(theta), R / H, std::sin(theta)));
    };

    auto safeNormal = [&](const glm::vec3& p, float theta) -> glm::vec3 {
        glm::vec3 n = coneNormal(p);
        float len2 = glm::dot(n, n);
        if (len2 < 1e-12f) return fallbackNormal(theta);  // only hits at apex
        return n / std::sqrt(len2);
    };

    auto emit = [&](const glm::vec3& p, const glm::vec3& n){
        insertVec3(m_vertexData, p);
        insertVec3(m_vertexData, n);
    };

    for (int j = 0; j < S; ++j) {
        float t0 = static_cast<float>(j)     / S;  // upper ring
        float t1 = static_cast<float>(j + 1) / S;  // lower ring

        // CCW from outside: next on left, current on right
        glm::vec3 TL = pos(t0, nextTheta);
        glm::vec3 TR = pos(t0, currentTheta);
        glm::vec3 BL = pos(t1, nextTheta);
        glm::vec3 BR = pos(t1, currentTheta);

        glm::vec3 NTL = safeNormal(TL, nextTheta);
        glm::vec3 NTR = safeNormal(TR, currentTheta);
        glm::vec3 NBL = safeNormal(BL, nextTheta);
        glm::vec3 NBR = safeNormal(BR, currentTheta);

        // tri 1
        emit(TL, NTL); emit(BL, NBL); emit(BR, NBR);
        // tri 2
        emit(TL, NTL); emit(BR, NBR); emit(TR, NTR);
    }
}

void Cone::makeWedge(float currentTheta, float nextTheta) {
    // Task 10: create a single wedge of the Cone using the
    //          makeCapSlice() and makeSlopeSlice() functions you
    //          implemented in Task 5
    makeCapSlice(currentTheta, nextTheta);
    makeSlopeSlice(currentTheta, nextTheta);
}

void Cone::setVertexData() {
    // Task 10: create a full cone using the makeWedge() function you
    //          just implemented
    // Note: think about how param 2 comes into play here!
    const int wedges = std::max(3, m_param2); // need at least a few slices
    const float thetaStep = glm::radians(360.f / wedges);

    for (int i = 0; i < wedges; ++i) {
        float currentTheta = i * thetaStep;
        float nextTheta    = (i + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cone::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
