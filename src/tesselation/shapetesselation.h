#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "utils/scenedata.h"  // for ScenePrimitive, PrimitiveType

// holds interleaved position/normal data: px,py,pz,nx,ny,nz,...
struct ShapeMeshData {
    std::vector<float> vertices;
};

// given a primitive and tessellation parameters, fill out vertex data.
ShapeMeshData generateShapeMesh(const ScenePrimitive &prim,
                                int param1,
                                int param2);
