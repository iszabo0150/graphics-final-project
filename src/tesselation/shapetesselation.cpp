#include "tesselation/shapetesselation.h"
#include "shapes/cube.h"
#include "shapes/sphere.h"
#include "shapes/cone.h"
#include "shapes/Cylinder.h"
#include "utils/objloader.h"
// Optional: lab-only debug shapes, not used by scene JSON
#include "shapes/Triangle.h"
#include "shapes/Tet.h"

#include <algorithm>

ShapeMeshData generateShapeMesh(const ScenePrimitive &primitive,
                                int param1,
                                int param2)
{
    ShapeMeshData mesh;

    // Clamp parameters to safe minimums
    int p1 = std::max(param1, 1);
    int p2 = std::max(param2, 1);

    switch (primitive.type) {
    case PrimitiveType::PRIMITIVE_CUBE: {
        Cube cube;
        cube.updateParams(p1);
        mesh.vertices = cube.generateShape();
        break;
    }

    case PrimitiveType::PRIMITIVE_SPHERE: {
        Sphere sphere;
        sphere.updateParams(p1, p2);
        mesh.vertices = sphere.generateShape();
        break;
    }

    case PrimitiveType::PRIMITIVE_CONE: {
        Cone cone;
        cone.updateParams(p1, p2);
        mesh.vertices = cone.generateShape();
        break;
    }

    case PrimitiveType::PRIMITIVE_CYLINDER: {
        Cylinder cyl;
        cyl.updateParams(p1, std::max(p2, 3));
        mesh.vertices = cyl.generateShape();
        break;
    }

    case PrimitiveType::PRIMITIVE_MESH: {
        // Use the meshfile field from ScenePrimitive
        // Adjust primitive.meshfile if your field is named differently.
        mesh = ObjMeshCache::load(primitive.meshfile);
        break;
    }

    default:
        // Unknown primitive, leave mesh empty
        break;
    }

    return mesh;
}
