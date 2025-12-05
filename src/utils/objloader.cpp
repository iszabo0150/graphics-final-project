#include "utils/objloader.h"

#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>

// Global cache so we do not reparse the same mesh file many times
static std::unordered_map<std::string, ShapeMeshData> g_meshCache;

ShapeMeshData ObjMeshCache::load(const std::string &filepath) {
    auto it = g_meshCache.find(filepath);
    if (it != g_meshCache.end()) {
        return it->second;
    }

    ShapeMeshData data = loadFromFile(filepath);
    g_meshCache[filepath] = data;
    return data;
}

// Helper to split a string by a delimiter
static std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}

ShapeMeshData ObjMeshCache::loadFromFile(const std::string &filepath) {
    ShapeMeshData out;

    std::ifstream in(filepath);
    if (!in.is_open()) {
        std::cerr << "[ObjMeshCache] Could not open OBJ file: " << filepath << std::endl;
        return out;
    }

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;

    struct Idx {
        int v;
        int vn;
    };
    std::vector<Idx> triangles; // expanded triangle indices

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::stringstream ls(line);
        std::string tag;
        ls >> tag;

        if (tag == "v") {
            float x, y, z;
            ls >> x >> y >> z;
            positions.emplace_back(x, y, z);
        } else if (tag == "vn") {
            float x, y, z;
            ls >> x >> y >> z;
            normals.emplace_back(x, y, z);
        } else if (tag == "f") {
            // f can be: v, v/vt, v//vn, v/vt/vn
            std::vector<std::string> verts;
            std::string vtxt;
            while (ls >> vtxt) {
                verts.push_back(vtxt);
            }
            if (verts.size() < 3) {
                continue;
            }

            auto parseIdx = [&](const std::string &token) -> Idx {
                Idx idx{-1, -1};
                auto parts = split(token, '/');
                if (!parts.empty()) {
                    idx.v = std::stoi(parts[0]) - 1; // OBJ is 1 based
                }
                if (parts.size() == 3 && !parts[2].empty()) {
                    idx.vn = std::stoi(parts[2]) - 1;
                } else if (parts.size() == 2 && !parts[1].empty()) {
                    // v//vn case is actually 3 parts, so 2 parts likely v/vt, ignore
                    idx.vn = -1;
                }
                return idx;
            };

            // fan triangulation for quads (or more, but we assume tris / quads in this project)
            Idx i0 = parseIdx(verts[0]);
            for (size_t k = 1; k + 1 < verts.size(); ++k) {
                Idx i1 = parseIdx(verts[k]);
                Idx i2 = parseIdx(verts[k + 1]);
                triangles.push_back(i0);
                triangles.push_back(i1);
                triangles.push_back(i2);
            }
        }
    }

    in.close();

    if (positions.empty()) {
        std::cerr << "[ObjMeshCache] No vertices found in OBJ: " << filepath << std::endl;
        return out;
    }

    out.vertices.reserve(triangles.size() * 6);

    // Build interleaved buffer, computing face normals if needed
    for (size_t i = 0; i < triangles.size(); i += 3) {
        Idx a = triangles[i];
        Idx b = triangles[i + 1];
        Idx c = triangles[i + 2];

        glm::vec3 pa = positions[a.v];
        glm::vec3 pb = positions[b.v];
        glm::vec3 pc = positions[c.v];

        glm::vec3 na, nb, nc;

        bool haveNormals = !normals.empty() &&
                           a.vn >= 0 && b.vn >= 0 && c.vn >= 0 &&
                           a.vn < (int)normals.size() &&
                           b.vn < (int)normals.size() &&
                           c.vn < (int)normals.size();

        if (haveNormals) {
            na = normals[a.vn];
            nb = normals[b.vn];
            nc = normals[c.vn];
        } else {
            // compute flat normal
            glm::vec3 n = glm::normalize(glm::cross(pb - pa, pc - pa));
            na = nb = nc = n;
        }

        auto emit = [&](const glm::vec3 &p, const glm::vec3 &n) {
            out.vertices.push_back(p.x);
            out.vertices.push_back(p.y);
            out.vertices.push_back(p.z);
            out.vertices.push_back(n.x);
            out.vertices.push_back(n.y);
            out.vertices.push_back(n.z);
        };

        emit(pa, na);
        emit(pb, nb);
        emit(pc, nc);
    }

    return out;
}
