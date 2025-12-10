#include "meshloader.h"
#include "utils/objloader.h"
#include "utils/math_utils.h"

#include <iostream>
#include <glm/glm.hpp>

std::vector<float> MeshLoader::loadOBJ(const std::string& filepath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // load the OBJ file
    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str());

    if (!warn.empty()) {
        std::cout << "TinyOBJ Warning: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "TinyOBJ Error: " << err << std::endl;
    }

    if (!success) {
        std::cerr << "Failed to load OBJ file: " << filepath << std::endl;
        return {};
    }

    std::vector<float> vertices;

    // process each shape in the OBJ file
    for (const auto& shape : shapes) {
        size_t index_offset = 0;

        // process each face (triangle)
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            int fv = shape.mesh.num_face_vertices[f];

            // we only handle triangles (fv == 3) --> if the mesh has quads we just ignore them
            if (fv != 3) {
                std::cerr << "Warning: Non-triangle face found (vertices: " << fv << "). Skipping." << std::endl;
                index_offset += fv;
                continue;
            }

            // extracting the 3 vertices of this triangle
            glm::vec3 pos[3];
            glm::vec3 norm[3];
            glm::vec2 uv[3];
            bool hasNormals = true;
            bool hasUVs = true;

            for (int v = 0; v < 3; v++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                // position (always present)
                pos[v] = glm::vec3(
                    attrib.vertices[3 * idx.vertex_index + 0],
                    attrib.vertices[3 * idx.vertex_index + 1],
                    attrib.vertices[3 * idx.vertex_index + 2]
                    );

                // normal (may not be present)
                if (idx.normal_index >= 0 && !attrib.normals.empty()) {
                    norm[v] = glm::vec3(
                        attrib.normals[3 * idx.normal_index + 0],
                        attrib.normals[3 * idx.normal_index + 1],
                        attrib.normals[3 * idx.normal_index + 2]
                        );
                } else {
                    hasNormals = false;
                }

                // uv / texture coordinates (may not be present)
                if (idx.texcoord_index >= 0 && !attrib.texcoords.empty()) {
                    uv[v] = glm::vec2(
                        attrib.texcoords[2 * idx.texcoord_index + 0],
                        attrib.texcoords[2 * idx.texcoord_index + 1]
                        );
                } else {
                    hasUVs = false;
                    uv[v] = glm::vec2(0.0f, 0.0f);
                }
            }

            // if normals weren't in the file, compute face normal from cross product
            if (!hasNormals) {
                glm::vec3 edge1 = pos[1] - pos[0];
                glm::vec3 edge2 = pos[2] - pos[0];
                glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));
                norm[0] = norm[1] = norm[2] = faceNormal;
            }

            // compute tangent and bitangent for this triangle using the utility function !
            glm::vec3 tangent(1, 0, 0);
            glm::vec3 bitangent(0, 1, 0);

            if (hasUVs) {
                TangentBitangent tb = Utils::computeTangentBitangent(
                    pos[0], pos[1], pos[2],
                    uv[0], uv[1], uv[2]
                    );
                tangent = tb.tangent;
                bitangent = tb.bitangent;

                if (glm::length(tangent) < 0.0001f || std::isnan(tangent.x)) {
                    tangent = glm::vec3(1, 0, 0);
                }
                if (glm::length(bitangent) < 0.0001f || std::isnan(bitangent.x)) {
                    bitangent = glm::vec3(0, 1, 0);
                }
            }

            // push all the info (position, normals, uvs, tangents, bitangents)
            for (int v = 0; v < 3; v++) {
                Utils::insertVec3(vertices, pos[v]);
                Utils::insertVec3(vertices, glm::normalize(norm[v]));
                Utils::insertVec2(vertices, uv[v]);
                Utils::insertVec3(vertices, tangent);
                Utils::insertVec3(vertices, bitangent);
            }

            index_offset += fv;
        }
    }

    std::cout << "Loaded OBJ: " << filepath << " with " << (vertices.size() / 14) << " vertices" << std::endl;

    return vertices;
}

MeshGLData MeshLoader::getMeshData(const std::string& filepath) {

    // check if already cached
    auto it = m_meshCache.find(filepath);
    if (it != m_meshCache.end()) {
        return it->second;
    }

    // if its not cached, create data for it !!
    MeshGLData data = createMeshGLData(filepath);
    m_meshCache[filepath] = data;
    return data;
}

MeshGLData MeshLoader::createMeshGLData(const std::string& filepath) {
    // load vertex data from OBJ file
    std::vector<float> meshData = loadOBJ(filepath);

    if (meshData.empty()) {
        std::cerr << "Failed to create GL data for mesh: " << filepath << std::endl;
        return MeshGLData{0, 0, 0};
    }

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, meshData.size() * sizeof(GLfloat), meshData.data(), GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // set up vertex attributes (same layout as the shapes)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), reinterpret_cast<void*>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), reinterpret_cast<void*>(6 * sizeof(GLfloat)));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), reinterpret_cast<void*>(8 * sizeof(GLfloat)));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), reinterpret_cast<void*>(11 * sizeof(GLfloat)));

    GLuint instanceVBO;
    glGenBuffers(1, &instanceVBO);

    // Unbind
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    int vertexCount = meshData.size() / 14;

    return MeshGLData{vao, vbo, instanceVBO, vertexCount};
}

void MeshLoader::cleanup() {
    for (auto& pair : m_meshCache) {
        glDeleteVertexArrays(1, &pair.second.vao);
        glDeleteBuffers(1, &pair.second.vbo);
        glDeleteBuffers(1, &pair.second.instanceVBO);

    }
    m_meshCache.clear();
}
