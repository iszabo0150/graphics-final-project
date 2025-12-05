// utils/objloader.h
#pragma once

#include <string>
#include "tesselation/shapetesselation.h"

// Simple cache + loader for OBJ meshes.
// Produces ShapeMeshData with interleaved pos/normal: px,py,pz,nx,ny,nz,...
class ObjMeshCache {
public:
    // Load or fetch from cache
    static ShapeMeshData load(const std::string &filepath);

private:
    static ShapeMeshData loadFromFile(const std::string &filepath);
};
