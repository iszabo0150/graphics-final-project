#include "Triangle.h"

void Triangle::updateParams() {
    m_vertexData = std::vector<float>();
    setVertexData();
}

void Triangle::setVertexData() {
    // Task 1: update m_vertexData with the vertices and normals
    //         needed to tesselate a triangle
    // Note: you may find the insertVec3 function useful in adding your points into m_vertexData
    m_vertexData.clear();

    // CCW order as viewed from +z so backface culling does not hide it
    const glm::vec3 A(-0.5f,  0.5f, 0.0f);
    const glm::vec3 B(-0.5f, -0.5f, 0.0f);
    const glm::vec3 C( 0.5f, -0.5f, 0.0f);

    // flat-shaded per-vertex normal for this face
    const glm::vec3 N(0.0f, 0.0f, 1.0f);

    // interleave position then normal for each vertex
    insertVec3(m_vertexData, A); insertVec3(m_vertexData, N);
    insertVec3(m_vertexData, B); insertVec3(m_vertexData, N);
    insertVec3(m_vertexData, C); insertVec3(m_vertexData, N);
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Triangle::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
