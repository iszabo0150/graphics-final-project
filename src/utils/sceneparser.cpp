#include "sceneparser.h"
#include "scenefilereader.h"
#include "lsystem/lsystem.h"
#include "renderers/lightrenderer.h"
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

        std::vector<LSymbol> symbols = LSystem::expandLSystem(*currNode->lsystem);

        std::vector<StemData> stems;
        std::vector<glm::mat4> leafCTMs;
        std::vector<glm::mat4> flowerCTMs;
        LSystem::interpretLSystem(*currNode->lsystem, symbols, stems, leafCTMs, flowerCTMs);


        const float refThickness = 1.0f;  // adjust based on the scene file's intended base size
        const float refLength = 1.0f;

        // Only generate flowers if we have flower materials and mesh file
        const auto& flowerMats = currNode->lsystem->flowerMaterials;
        if (!flowerMats.empty() && !currNode->lsystem->flowerMeshFile.empty()) {
            for (const glm::mat4 &localM : flowerCTMs) {
                RenderShapeData r;

                // randomly select a flower material from the available options
                int matIndex = rand() % flowerMats.size();
                const SceneMaterial& chosenMat = flowerMats[matIndex];

                ScenePrimitive p;
                p.type = PrimitiveType::PRIMITIVE_MESH;
                p.meshfile = currNode->lsystem->flowerMeshFile;
                p.material = chosenMat;

                r.primitive = p;
                r.material = chosenMat;
                r.ctm = currCTM * localM;

                renderData.shapes.push_back(r);
            }
        }

        for (const StemData &stem : stems) {
            RenderShapeData r;

            // Make a COPY of the material so we can modify it per-shape
            SceneMaterial mat = currNode->lsystem->stemMaterial;

            // Scale texture repeats based on actual dimensions
            // repeatU controls horizontal wrap (circumference) - scale by thickness ratio
            // repeatV controls vertical wrap (length) - scale by length ratio
            if (mat.textureMap.isUsed) {
                mat.textureMap.repeatU *= (stem.thickness / refThickness);
                mat.textureMap.repeatV *= (stem.length / refLength);
            }
            if (mat.bumpMap.isUsed) {
                mat.bumpMap.repeatU *= (stem.thickness / refThickness);
                mat.bumpMap.repeatV *= (stem.length / refLength);
            }
            if (mat.normalMap.isUsed) {
                mat.normalMap.repeatU *= (stem.thickness / refThickness);
                mat.normalMap.repeatV *= (stem.length / refLength);
            }

            r.primitive = makePrimitive(currNode->lsystem->stemPrimitive, mat);
            r.material = mat;
            r.ctm = currCTM * stem.ctm;

            renderData.shapes.push_back(r);
        }

        // std::cout << renderData.shapes.size() << std::endl;




        // Only generate leaves if we have leaf materials (winter has none)
        const auto& leafMats = currNode->lsystem->leafMaterials;
        if (!leafMats.empty()) {
            for (const glm::mat4 &localM : leafCTMs) {
                RenderShapeData r;

                // Randomly select a leaf material from the available options
                int matIndex = rand() % leafMats.size();
                const SceneMaterial& chosenMat = leafMats[matIndex];

                r.primitive = makePrimitive(currNode->lsystem->leafPrimitive, chosenMat);
                r.material = chosenMat;

                r.ctm = currCTM * localM;

                renderData.shapes.push_back(r);
            }
        }
        // return;
    }

    std::cout << renderData.shapes.size() << std::endl;


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

    glm::mat4 lightSpaceMat = LightRenderer::calculateLightMatrix(&light, glm::vec3(pos), glm::vec3(dir));
    return SceneLightData{light.id, light.type, light.color, light.function, pos, dir, light.penumbra,
                          light.angle, 0.0f, 0.0f, lightSpaceMat};

}
