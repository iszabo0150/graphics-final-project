#pragma once

#include "scenedata.h"
// #include "imagereader.h"
#include <vector>
#include <string>

// Struct which contains data for a single primitive, to be used for rendering
struct RenderShapeData {
    ScenePrimitive primitive;
    SceneMaterial material;
    glm::mat4 ctm; // the cumulative transformation matrix

};

// Struct which contains all the data needed to render a scene
struct RenderData {
    SceneGlobalData globalData;
    SceneCameraData cameraData;

    std::vector<SceneLightData> lights;
    std::vector<RenderShapeData> shapes;
};

class SceneParser {
public:
    // Parse the scene and store the results in renderData.
    // @param filepath    The path of the scene file to load.
    // @param renderData  On return, this will contain the metadata of the loaded scene.
    // @return            A boolean value indicating whether the parse was successful.
    static bool parse(std::string filepath, RenderData &renderData);

private:
    // Recursive DFS helper to traverse the scene graph and fill renderData.
    static void dfsGetRenderData(RenderData &renderData, SceneNode *currNode, glm::mat4 currCTM);

    // Build a transformation matrix from a SceneTransformation.
    static glm::mat4 getTransMatrix(SceneTransformation &trans);

    // Convert a SceneLight into a SceneLightData with CTM applied.
    static SceneLightData getSceneLightData(SceneLight &light, glm::mat4 ctm);


    static SceneMaterial makeDefaultLSystemMaterial();

    static ScenePrimitive makePrimitive(PrimitiveType type, const SceneMaterial &mat);

};
