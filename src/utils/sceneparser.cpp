#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>

// Helper to traverse the scene graph recursively and fill RenderData
namespace {

// Build the CTM for this node, then add its primitives and lights,
// then recurse on children.
void traverseSceneGraph(SceneNode *node,
                        const glm::mat4 &parentCTM,
                        RenderData &renderData)
{
    if (!node) return;

    // Start from the parent's CTM
    glm::mat4 ctm = parentCTM;

    // Apply this node's transformations in order (Lab 5 style)
    for (SceneTransformation *t : node->transformations) {
        switch (t->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            ctm *= glm::translate(t->translate);
            break;
        case TransformationType::TRANSFORMATION_SCALE:
            ctm *= glm::scale(t->scale);
            break;
        case TransformationType::TRANSFORMATION_ROTATE:
            // angle is in radians, rotate about given axis
            ctm *= glm::rotate(t->angle, t->rotate);
            break;
        case TransformationType::TRANSFORMATION_MATRIX:
            ctm *= t->matrix;
            break;
        }
    }

    // Add all primitives at this node to the render list
    for (ScenePrimitive *prim : node->primitives) {
        RenderShapeData shapeData;
        shapeData.primitive = *prim;  // copy material, type, meshfile, etc.
        shapeData.ctm       = ctm;    // cumulative transform at this node
        renderData.shapes.push_back(shapeData);
    }

    // Add all lights at this node to the render list
    for (SceneLight *light : node->lights) {
        SceneLightData lightData;

        // Give each light a unique id based on its index in the vector
        lightData.id   = static_cast<int>(renderData.lights.size());
        lightData.type = light->type;

        lightData.color    = light->color;
        lightData.function = light->function;
        lightData.penumbra = light->penumbra;
        lightData.angle    = light->angle;
        lightData.width    = light->width;
        lightData.height   = light->height;

        // World-space position and direction depend on the light type
        glm::vec4 origin(0.f, 0.f, 0.f, 1.f);

        switch (light->type) {
        case LightType::LIGHT_POINT: {
            // Position is the CTM-transformed origin, direction unused
            lightData.pos = ctm * origin;
            lightData.dir = glm::vec4(0.f, 0.f, 0.f, 0.f);
            break;
        }
        case LightType::LIGHT_DIRECTIONAL: {
            // Direction transforms with w = 0 and should be normalized
            glm::vec4 dirOS(light->dir.x, light->dir.y, light->dir.z, 0.f);
            glm::vec3 dirWS3 = glm::vec3(ctm * dirOS);
            dirWS3 = glm::normalize(dirWS3);

            lightData.pos = glm::vec4(0.f, 0.f, 0.f, 0.f); // not used
            lightData.dir = glm::vec4(dirWS3, 0.f);
            break;
        }
        case LightType::LIGHT_SPOT: {
            // Spotlights have both a position and a direction
            lightData.pos = ctm * origin;

            glm::vec4 dirOS(light->dir.x, light->dir.y, light->dir.z, 0.f);
            glm::vec3 dirWS3 = glm::vec3(ctm * dirOS);
            dirWS3 = glm::normalize(dirWS3);

            lightData.dir = glm::vec4(dirWS3, 0.f);
            break;
        }
        }

        renderData.lights.push_back(lightData);
    }

    // Recurse on children with the updated CTM
    for (SceneNode *child : node->children) {
        traverseSceneGraph(child, ctm, renderData);
    }
}

} // end anonymous namespace

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    // 1. Copy global and camera data
    renderData.globalData = fileReader.getGlobalData();
    renderData.cameraData = fileReader.getCameraData();

    // 2. Clear vectors
    renderData.lights.clear();
    renderData.shapes.clear();

    // 3. Traverse the scene graph from the root with identity CTM
    SceneNode* root = fileReader.getRootNode();
    glm::mat4 I(1.0f);

    traverseSceneGraph(root, I, renderData);

    return true;
}
