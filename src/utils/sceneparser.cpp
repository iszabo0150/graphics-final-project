#include "sceneparser.h"
#include "scenefilereader.h"
#include "lsystem/lsystem.h"
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <iostream>


/**
 * @brief SceneParser::parse parses the Scene Graph !! calses dfsRenderData to recursively populate renderData
 * @param filepath
 * @param renderData
 * @return
 */

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader = ScenefileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    renderData.cameraData = fileReader.getCameraData();
    renderData.globalData = fileReader.getGlobalData();

    renderData.shapes.clear();

    SceneNode* root = fileReader.getRootNode();
    glm::mat4 baseCTM = glm::mat4(1.0f); //identity matrix --> leaves things unchanged

    dfsGetRenderData(renderData, root, baseCTM);


    return true;
}

SceneMaterial SceneParser::makeDefaultLSystemMaterial(){
    SceneMaterial m;
    m.clear(); // zero-out everything first

    // Ambient is usually diffuse * 0.2 or similar — safe default:
    m.cAmbient  = glm::vec4(0.2f, 0.0f, 0.0f, 1.0f);

    // User-specified L-system defaults:
    m.cDiffuse  = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    m.cSpecular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    m.shininess = 25.0f;

    return m;
}

ScenePrimitive SceneParser::makePrimitive(PrimitiveType type, const SceneMaterial &mat){
    ScenePrimitive p;
    p.type = type;
    p.material = mat;

    // Only used for triangle meshes—safe empty:
    p.meshfile = "";

    return p;
}

/**
 * @brief SceneParser::dfsGetRenderData recursivelly fills out the scene graph !! for each node checks for transformations and
 * builds the transformation matrix, adds shapes and lights to the list
 * @param renderData
 * @param currNode
 * @param currCTM
 */
void SceneParser::dfsGetRenderData(RenderData& renderData, SceneNode* currNode, glm::mat4 currCTM) {
    //passing the **value** of currCTM so muttating it doesnt affect siblings :)

    if (!currNode->transformations.empty()) {
        for (SceneTransformation* trans : currNode->transformations) {
            currCTM = currCTM * getTransMatrix(*trans);
        }
    }


    if (!currNode->primitives.empty()){
        for (ScenePrimitive* p : currNode->primitives) {

            RenderShapeData r = RenderShapeData{*p,p->material,currCTM};
            renderData.shapes.push_back(r);
        }

    }

    if (!currNode->lights.empty()) {
        for (SceneLight* light : currNode->lights) {
            SceneLightData l = getSceneLightData(*light, currCTM);
            renderData.lights.push_back(l);
        }
    }

    if (currNode->lsystem && currNode->lsystem->valid){

        std::vector<LSymbol> symbols =
        LSystem::expandLSystem(*currNode->lsystem);

        std::vector<glm::mat4> stemCTMs;
        std::vector<glm::mat4> leafCTMs;

        LSystem::interpretLSystem(*currNode->lsystem, symbols, stemCTMs, leafCTMs);


        for (const glm::mat4 &localM : stemCTMs) {
            RenderShapeData r;

            r.primitive = makePrimitive(currNode->lsystem->stemPrimitive, currNode->lsystem->stemMaterial);
            r.material = currNode->lsystem->stemMaterial;

            r.ctm = currCTM * localM;

            renderData.shapes.push_back(r);
        }



        for (const glm::mat4 &localM : leafCTMs) {
            RenderShapeData r;

            r.primitive = makePrimitive(currNode->lsystem->leafPrimitive, currNode->lsystem->leafMaterial);
            r.material = currNode->lsystem->leafMaterial;

            r.ctm = currCTM * localM;

            renderData.shapes.push_back(r);
        }
        return;
    }

    if (!currNode->children.empty()){
        for (SceneNode* children : currNode->children){
            dfsGetRenderData(renderData, children, currCTM);
        }
    }

    else {
        return;
    }
}

/**
 * @brief SceneParser::getTransMatrix returns the transformation matrix depending on the SceneTransformation type
 * @param trans
 * @return
 */
glm::mat4 SceneParser::getTransMatrix(SceneTransformation &trans) {
    switch (trans.type) {
    case TransformationType::TRANSFORMATION_TRANSLATE:
        return glm::translate(glm::mat4(1.0f), trans.translate);

    case TransformationType::TRANSFORMATION_SCALE:
        return glm::scale(glm::mat4(1.0f), trans.scale);

    case TransformationType::TRANSFORMATION_ROTATE:
        return glm::rotate(glm::mat4(1.0f), trans.angle, trans.rotate);

    case TransformationType::TRANSFORMATION_MATRIX:
        return trans.matrix;

    default:
        return glm::mat4(1.0f); //identity matrix ! leaves things unchanged
    }
}

/**
 * @brief SceneParser::getSceneLightData returns the the light data depnding on teh ctm and the passed in SceneLight
 * @param light
 * @param ctm
 * @return
 */
SceneLightData SceneParser::getSceneLightData(SceneLight &light, glm::mat4 ctm){

    glm::vec4 pos(0,0,0,1);
    glm::vec4 dir(0,0,-1,0); // default light-space direction

    if (light.type != LightType::LIGHT_DIRECTIONAL){ //ISNT a directional light
        pos = ctm * glm::vec4(0, 0, 0, 1);  // origin transformed into world space with the ctm
    }


    if (light.type != LightType::LIGHT_POINT){ //ISNT a point light
        dir = glm::normalize(ctm * glm::vec4(light.dir));
    }

    return SceneLightData{light.id, light.type, light.color, light.function, pos, dir, light.penumbra, light.angle};

}
