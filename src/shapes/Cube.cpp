#include "Cube.h"

void Cube::updateParams(int param1) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    setVertexData();
}

void Cube::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    // Task 2: create a tile (i.e. 2 triangles) based on 4 given points.
    // compute flat face normal (unit length) using CCW edges
    // using TL -> BL and TL -> BR so the first triangle (TL, BL, BR) is CCW.
    glm::vec3 e1 = bottomLeft - topLeft;
    glm::vec3 e2 = bottomRight - topLeft;
    glm::vec3 N  = glm::normalize(glm::cross(e1, e2));

    // triangle 1: TL, BL, BR
    insertVec3(m_vertexData, topLeft);     insertVec3(m_vertexData, N);
    insertVec3(m_vertexData, bottomLeft);  insertVec3(m_vertexData, N);
    insertVec3(m_vertexData, bottomRight); insertVec3(m_vertexData, N);

    // triangle 2: TL, BR, TR
    insertVec3(m_vertexData, topLeft);     insertVec3(m_vertexData, N);
    insertVec3(m_vertexData, bottomRight); insertVec3(m_vertexData, N);
    insertVec3(m_vertexData, topRight);    insertVec3(m_vertexData, N);
}

void Cube::makeFace(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    // Task 3: create a single side of the cube out of the 4
    //         given points and makeTile()
    // Note: think about how param 1 affects the number of triangles on
    //       the face of the cube
    const int S = std::max(1, m_param1);

    // helper lambdas for linear interpolation
    auto lerp = [](const glm::vec3& a, const glm::vec3& b, float t) {
        return a * (1.0f - t) + b * t;
    };

    // for each grid cell, compute its four corners by bilinear interpolation
    for (int j = 0; j < S; ++j) {
        float v0 = static_cast<float>(j)     / S;  // 0 at top, 1 at bottom
        float v1 = static_cast<float>(j + 1) / S;

        // interpolate the left and right edges for both v0 and v1 rows
        glm::vec3 leftRow0  = lerp(topLeft,    bottomLeft,  v0);
        glm::vec3 rightRow0 = lerp(topRight,   bottomRight, v0);
        glm::vec3 leftRow1  = lerp(topLeft,    bottomLeft,  v1);
        glm::vec3 rightRow1 = lerp(topRight,   bottomRight, v1);

        for (int i = 0; i < S; ++i) {
            float u0 = static_cast<float>(i)     / S;  // 0 at left, 1 at right
            float u1 = static_cast<float>(i + 1) / S;

            // corners of the sub-quad in CCW order as seen from the front
            glm::vec3 TL = lerp(leftRow0,  rightRow0, u0);
            glm::vec3 TR = lerp(leftRow0,  rightRow0, u1);
            glm::vec3 BL = lerp(leftRow1,  rightRow1, u0);
            glm::vec3 BR = lerp(leftRow1,  rightRow1, u1);

            makeTile(TL, TR, BL, BR);
        }
    }
}

void Cube::setVertexData() {
    // Uncomment these lines for Task 2, then comment them out for Task 3:

    // makeTile(glm::vec3(-0.5f,  0.5f, 0.5f),
    //          glm::vec3( 0.5f,  0.5f, 0.5f),
    //          glm::vec3(-0.5f, -0.5f, 0.5f),
    //          glm::vec3( 0.5f, -0.5f, 0.5f));

    // Uncomment these lines for Task 3:

    makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
             glm::vec3( 0.5f,  0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3( 0.5f, -0.5f, 0.5f));

    // Task 4: Use the makeFace() function to make all 6 sides of the cube
    // Back (-Z)
    makeFace(glm::vec3( 0.5f,  0.5f, -0.5f),  // TL
             glm::vec3(-0.5f,  0.5f, -0.5f),  // TR
             glm::vec3( 0.5f, -0.5f, -0.5f),  // BL
             glm::vec3(-0.5f, -0.5f, -0.5f)); // BR

    // Left (-X)
    makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),  // TL
             glm::vec3(-0.5f,  0.5f,  0.5f),  // TR
             glm::vec3(-0.5f, -0.5f, -0.5f),  // BL
             glm::vec3(-0.5f, -0.5f,  0.5f)); // BR

    // Right (+X)
    makeFace(glm::vec3( 0.5f,  0.5f,  0.5f),  // TL
             glm::vec3( 0.5f,  0.5f, -0.5f),  // TR
             glm::vec3( 0.5f, -0.5f,  0.5f),  // BL
             glm::vec3( 0.5f, -0.5f, -0.5f)); // BR

    // Top (+Y)
    makeFace(glm::vec3(-0.5f,  0.5f, -0.5f),  // TL
             glm::vec3( 0.5f,  0.5f, -0.5f),  // TR
             glm::vec3(-0.5f,  0.5f,  0.5f),  // BL
             glm::vec3( 0.5f,  0.5f,  0.5f)); // BR

    // Bottom (-Y)
    makeFace(glm::vec3(-0.5f, -0.5f,  0.5f),  // TL
             glm::vec3( 0.5f, -0.5f,  0.5f),  // TR
             glm::vec3(-0.5f, -0.5f, -0.5f),  // BL
             glm::vec3( 0.5f, -0.5f, -0.5f)); // BR
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cube::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
