#ifndef PRESET_UTILS_H
#define PRESET_UTILS_H

#include "lsystem/plantpresets.h"

// helper functions for creating materials - shared by all presets
namespace PresetUtils {

inline SceneMaterial makeMaterial(const glm::vec3& diffuse, 
                                   const glm::vec3& ambient,
                                   const glm::vec3& specular, 
                                   float shininess) {
    SceneMaterial mat;
    mat.clear();
    mat.cDiffuse = glm::vec4(diffuse, 1.0f);
    mat.cAmbient = glm::vec4(ambient, 1.0f);
    mat.cSpecular = glm::vec4(specular, 1.0f);
    mat.shininess = shininess;
    return mat;
}

inline SceneMaterial makeStemMaterial(const glm::vec3& diffuse,
                                       const glm::vec3& ambient,
                                       const glm::vec3& specular,
                                       float shininess,
                                       const std::string& textureFile = "",
                                       const std::string& normalMapFile = "",
                                       float blend = 0.5f) {
    SceneMaterial mat = makeMaterial(diffuse, ambient, specular, shininess);
    
    if (!textureFile.empty()) {
        mat.textureMap.isUsed = true;
        mat.textureMap.filename = textureFile;
        mat.textureMap.repeatU = 1.0f;
        mat.textureMap.repeatV = 1.0f;
        mat.blend = blend;
    }
    
    if (!normalMapFile.empty()) {
        mat.normalMap.isUsed = true;
        mat.normalMap.filename = normalMapFile;
        mat.normalMap.repeatU = 1.0f;
        mat.normalMap.repeatV = 1.0f;
        mat.normalMap.strength = 1.0f;
    }
    
    return mat;
}

// helper to add a rule
inline void addRule(PlantPreset& preset, 
                    const std::string& input,
                    const std::string& output,
                    float probability = 1.0f,
                    const std::vector<std::string>& params = {},
                    const std::string& condition = "") {
    LSystemRule rule;
    rule.input = input;
    rule.output = output;
    rule.probability = probability;
    rule.params = params;
    rule.condition = condition;
    preset.rules.push_back(rule);
}

} // namespace PresetUtils

// preset creation functions - each defined in their own file
PlantPreset createOakTreePreset();
PlantPreset createBushPreset();
PlantPreset createFlowerPlantPreset();

#endif // PRESET_UTILS_H
