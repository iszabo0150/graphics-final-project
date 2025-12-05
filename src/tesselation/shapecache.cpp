#include "tesselation/shapecache.h"
#include <vector>

void ShapeCache::destroyGeometry(ShapeGeometry &geom) {
    if (geom.vbo != 0) {
        glDeleteBuffers(1, &geom.vbo);
        geom.vbo = 0;
    }
    if (geom.vao != 0) {
        glDeleteVertexArrays(1, &geom.vao);
        geom.vao = 0;
    }
    geom.count = 0;
}

void ShapeCache::clear() {
    destroyGeometry(m_cube);
    destroyGeometry(m_sphere);
    destroyGeometry(m_cone);
    destroyGeometry(m_cylinder);
}

void ShapeCache::rebuildAll(int param1, int param2) {
    buildGeometry(m_cube,     PrimitiveType::PRIMITIVE_CUBE,     param1, param2);
    buildGeometry(m_sphere,   PrimitiveType::PRIMITIVE_SPHERE,   param1, param2);
    buildGeometry(m_cone,     PrimitiveType::PRIMITIVE_CONE,     param1, param2);
    buildGeometry(m_cylinder, PrimitiveType::PRIMITIVE_CYLINDER, param1, param2);
}

void ShapeCache::buildGeometry(ShapeGeometry &geom,
                               PrimitiveType type,
                               int param1,
                               int param2)
{
    // 1. Tessellate on CPU
    ScenePrimitive prim;
    prim.type = type;

    ShapeMeshData mesh = generateShapeMesh(prim, param1, param2);

    // 2. Drop old buffers
    destroyGeometry(geom);

    if (mesh.vertices.empty()) return;

    // 3. Allocate VAO/VBO and upload data
    glGenVertexArrays(1, &geom.vao);
    glGenBuffers(1, &geom.vbo);

    glBindVertexArray(geom.vao);
    glBindBuffer(GL_ARRAY_BUFFER, geom.vbo);

    glBufferData(GL_ARRAY_BUFFER,
                 mesh.vertices.size() * sizeof(float),
                 mesh.vertices.data(),
                 GL_STATIC_DRAW);

    // Our vertex layout: [pos.xyz, normal.xyz] = 6 floats per vertex
    GLsizei stride = 6 * sizeof(float);

    // location 0: position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        stride,
        reinterpret_cast<void*>(0)
        );

    // location 1: normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        stride,
        reinterpret_cast<void*>(3 * sizeof(float))
        );

    glBindVertexArray(0);

    geom.count = static_cast<GLsizei>(mesh.vertices.size() / 6);
}

const ShapeGeometry* ShapeCache::getGeometry(PrimitiveType type) const {
    switch (type) {
    case PrimitiveType::PRIMITIVE_CUBE:     return &m_cube;
    case PrimitiveType::PRIMITIVE_SPHERE:   return &m_sphere;
    case PrimitiveType::PRIMITIVE_CONE:     return &m_cone;
    case PrimitiveType::PRIMITIVE_CYLINDER: return &m_cylinder;
    default:                                return nullptr;
    }
}
