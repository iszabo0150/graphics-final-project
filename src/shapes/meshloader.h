// src/utils/meshloader.h
#ifndef MESHLOADER_H
#define MESHLOADER_H

#include <string>
#include <vector>
#include <map>
#include <GL/glew.h>
#include "utils/objloader.h"

struct MeshGLData {
    GLuint vao;
    GLuint vbo;
    GLuint instanceVBO;
    int vertexCount;
};

class MeshLoader {
public:
    // load mesh and return vertex data
    static std::vector<float> loadOBJ(const std::string& filepath);

    // get or create GL data for a mesh file (caches loaded meshes)
    MeshGLData getMeshData(const std::string& filepath);

    void cleanup();

private:
    std::map<std::string, MeshGLData> m_meshCache;

    MeshGLData createMeshGLData(const std::string& filepath);
};

#endif
