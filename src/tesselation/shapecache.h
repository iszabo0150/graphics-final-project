#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>

#include "utils/scenedata.h"              // PrimitiveType, ScenePrimitive
#include "tesselation/shapetesselation.h" // ShapeMeshData, generateShapeMesh

struct ShapeGeometry {
    GLuint vao   = 0;
    GLuint vbo   = 0;
    GLsizei count = 0;  // number of vertices
};

class ShapeCache {
public:
    ShapeCache() = default;

    // Delete all GL objects (call from Realtime::finish)
    void clear();

    // Rebuild all primitive meshes for the given tessellation params
    void rebuildAll(int param1, int param2);

    // Look up geometry by primitive type
    const ShapeGeometry* getGeometry(PrimitiveType type) const;

private:
    ShapeGeometry m_cube;
    ShapeGeometry m_sphere;
    ShapeGeometry m_cone;
    ShapeGeometry m_cylinder;

    void buildGeometry(ShapeGeometry &geom,
                       PrimitiveType type,
                       int param1,
                       int param2);

    static void destroyGeometry(ShapeGeometry &geom);
};
