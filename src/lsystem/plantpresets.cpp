#include "plantpresets.h"
#include <iostream>

// Static member initialization
std::map<std::string, PlantPreset> PlantPresets::s_presets;
bool PlantPresets::s_initialized = false;

SceneMaterial PlantPresets::makeMaterial(const glm::vec3& diffuse, 
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

SceneMaterial PlantPresets::makeStemMaterial(const glm::vec3& diffuse,
                                              const glm::vec3& ambient,
                                              const glm::vec3& specular,
                                              float shininess,
                                              const std::string& textureFile,
                                              const std::string& normalMapFile,
                                              float blend) {
    SceneMaterial mat = makeMaterial(diffuse, ambient, specular, shininess);
    
    if (!textureFile.empty()) {
        mat.textureMap.isUsed = true;
        mat.textureMap.filename = textureFile;  // Will be resolved with basePath later
        mat.textureMap.repeatU = 1.0f;
        mat.textureMap.repeatV = 1.0f;
        mat.blend = blend;
    }
    
    if (!normalMapFile.empty()) {
        mat.normalMap.isUsed = true;
        mat.normalMap.filename = normalMapFile;  // Will be resolved with basePath later
        mat.normalMap.repeatU = 1.0f;
        mat.normalMap.repeatV = 1.0f;
        mat.normalMap.strength = 1.0f;
    }
    
    return mat;
}

void PlantPresets::initializePresets() {
    if (s_initialized) return;
    
    // oak tree !!
    PlantPreset oakTree;
    oakTree.name = "oak_tree";
    
    // grammar
    oakTree.axiom = "F(10,3.0)A(4.5,3.0,40)";
    oakTree.iterations = 8;
    oakTree.angle = 35.0f;
    oakTree.step = 1.0f;
    
    // primitives
    oakTree.stemPrimitive = PrimitiveType::PRIMITIVE_CYLINDER;
    oakTree.leafPrimitive = PrimitiveType::PRIMITIVE_CONE;
    
    // stem material (bark - same for all seasons)
    oakTree.stemMaterial = makeStemMaterial(
        glm::vec3(0.35f, 0.2f, 0.12f),   // diffuse
        glm::vec3(0.15f, 0.08f, 0.04f),  // ambient
        glm::vec3(0.1f, 0.1f, 0.1f),     // specular
        2.0f,                             // shininess
        "textures/pls.jpg",               // texture
        "textures/plsN.jpg",              // normal map
        0.5f                              // blend
    );
    
    // Rules (same for all seasons - the branching structure doesn't change)

    // Rule A with thick > 0.05 (probability 0.3)
    {
        LSystemRule rule;
        rule.input = "A";
        rule.params = {"len", "thick", "angle"};
        rule.condition = "thick > 0.05";
        rule.output = "F(len*0.62,thick)[+(angle*0.95)^(6)B(len*1.5,thick*0.4,angle*1.12)]F(len*0.15,thick)L(1)/(117)[+(angle*1.3)^(12)B(len*1.5,thick*0.4,angle*0.92)]F(len*0.09,thick)L(1)/(122)[+(angle*0.85)^(3)B(len*1.5,thick*0.4,angle*0.88)]F(len*0.11,thick)L(1)A(len*0.93,thick*0.55,angle*1.05)";
        rule.probability = 0.3f;
        oakTree.rules.push_back(rule);
    }
    
    // Rule B variations with thick > 0.05
    {
        LSystemRule rule;
        rule.input = "B";
        rule.params = {"len", "thick", "angle"};
        rule.condition = "thick > 0.05";
        rule.output = "F(len*0.6,thick)L(1)^(6)[+(angle*0.78)B(len*0.88,thick*0.55,angle*0.82)]F(len*0.1,thick)L(1)/(142)[+(angle*0.62)B(len*0.88,thick*0.55,angle*0.9)]F(len*0.08,thick)L(1)/(127)[+(angle*0.95)B(len*0.88,thick*0.55,angle*0.88)]F(len*0.07,thick)L(1)";
        rule.probability = 0.3f;
        oakTree.rules.push_back(rule);
    }
    {
        LSystemRule rule;
        rule.input = "B";
        rule.params = {"len", "thick", "angle"};
        rule.condition = "thick > 0.05";
        rule.output = "F(len*0.65,thick)L(1)^(10)[+(angle*1.12)B(len*0.88,thick*0.55,angle*0.98)]F(len*0.12,thick)L(1)/(135)[+(angle*0.52)B(len*0.88,thick*0.55,angle*0.85)]F(len*0.09,thick)L(1)/(148)[+(angle*1.18)B(len*0.88,thick*0.55,angle*0.92)]F(len*0.06,thick)L(1)";
        rule.probability = 0.3f;
        oakTree.rules.push_back(rule);
    }
    {
        LSystemRule rule;
        rule.input = "B";
        rule.params = {"len", "thick", "angle"};
        rule.condition = "thick > 0.05";
        rule.output = "F(len*0.45,thick)L(1)^(4)[+(angle*0.92)B(len*0.88,thick*0.55,angle*0.95)]F(len*0.11,thick)L(1)/(118)[+(angle*0.55)B(len*0.88,thick*0.55,angle*0.92)]F(len*0.07,thick)L(1)/(155)[+(angle*0.85)B(len*0.88,thick*0.55,angle*0.9)]F(len*0.05,thick)L(1)";
        rule.probability = 0.4f;
        oakTree.rules.push_back(rule);
    }
    
    // Terminal rules (thick <= 0.05)
    {
        LSystemRule rule;
        rule.input = "A";
        rule.params = {"len", "thick", "angle"};
        rule.condition = "thick <= 0.05";
        rule.output = "F(len*0.2,thick)[+(angle*0.8)L(1)]F(len*0.15,thick)[/(90)L(1)]F(len*0.12,thick)[/(180)L(1)]F(len*0.1,thick)[/(270)L(1)]F(len*0.08,thick)[^(20)L(1)][&(20)]";
        rule.probability = 1.0f;
        oakTree.rules.push_back(rule);
    }
    // Terminal B rule WITH flowers (W symbol) - 30% chance
    {
        LSystemRule rule;
        rule.input = "B";
        rule.params = {"len", "thick", "angle"};
        rule.condition = "thick <= 0.05";
        rule.output = "F(len*0.15,thick)[+(angle*0.6)L(1)]F(len*0.12,thick)[/(72)L(1)]F(len*0.1,thick)[/(144)L(1)]F(len*0.08,thick)[/(216)L(1)]F(len*0.06,thick)[/(288)L(1)]F(len*0.05,thick)[^(15)L(1)][&(15)W(0.5)]";
        rule.probability = 0.3f;
        oakTree.rules.push_back(rule);
    }
    // Terminal B rule WITHOUT flowers - 70% chance
    {
        LSystemRule rule;
        rule.input = "B";
        rule.params = {"len", "thick", "angle"};
        rule.condition = "thick <= 0.05";
        rule.output = "F(len*0.15,thick)[+(angle*0.6)L(1)]F(len*0.12,thick)[/(72)L(1)]F(len*0.1,thick)[/(144)L(1)]F(len*0.08,thick)[/(216)L(1)]F(len*0.06,thick)[/(288)L(1)]F(len*0.05,thick)[^(15)L(1)][&(15)L(1)]";
        rule.probability = 0.7f;
        oakTree.rules.push_back(rule);
    }
    
    // ========================================
    // SEASONAL MATERIALS
    // ========================================
    
    // SUMMER - Lush green leaves
    {
        SeasonalMaterials summer;
        summer.hasLeaves = true;
        summer.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.557f, 0.678f, 0.224f),
            glm::vec3(0.22f, 0.27f, 0.09f),
            glm::vec3(0.2f, 0.2f, 0.2f),
            15.0f
        ));
        summer.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.522f, 0.620f, 0.247f),
            glm::vec3(0.21f, 0.25f, 0.10f),
            glm::vec3(0.2f, 0.2f, 0.2f),
            15.0f
        ));
        // No flowers in summer
        summer.flowerMeshFile = "";
        oakTree.seasonalMaterials[Season::SUMMER] = summer;
    }
    
    // SPRING - Green leaves with flowers
    {
        SeasonalMaterials spring;
        spring.hasLeaves = true;
        spring.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.557f, 0.678f, 0.224f),
            glm::vec3(0.22f, 0.27f, 0.09f),
            glm::vec3(0.2f, 0.2f, 0.2f),
            15.0f
        ));
        spring.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.522f, 0.620f, 0.247f),
            glm::vec3(0.21f, 0.25f, 0.10f),
            glm::vec3(0.2f, 0.2f, 0.2f),
            15.0f
        ));
        
        // Flowers for spring!
        spring.flowerMeshFile = "meshes/Lilly.obj";
        spring.flowerMaterials.push_back(makeMaterial(
            glm::vec3(0.988f, 0.976f, 0.910f),   // White/cream
            glm::vec3(0.5f, 0.5f, 0.5f),
            glm::vec3(0.5f, 0.5f, 0.5f),
            3.0f
        ));
        spring.flowerMaterials.push_back(makeMaterial(
            glm::vec3(1.0f, 0.8f, 0.9f),         // Light pink
            glm::vec3(0.5f, 0.4f, 0.45f),
            glm::vec3(0.5f, 0.5f, 0.5f),
            3.0f
        ));
        spring.flowerMaterials.push_back(makeMaterial(
            glm::vec3(0.95f, 0.7f, 0.75f),       // Deeper pink
            glm::vec3(0.47f, 0.35f, 0.37f),
            glm::vec3(0.5f, 0.5f, 0.5f),
            3.0f
        ));
        oakTree.seasonalMaterials[Season::SPRING] = spring;
    }
    
    // FALL - Orange, red, yellow leaves
    {
        SeasonalMaterials fall;
        fall.hasLeaves = true;
        fall.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.922f, 0.647f, 0.294f),   // Orange
            glm::vec3(0.37f, 0.26f, 0.12f),
            glm::vec3(0.2f, 0.2f, 0.2f),
            15.0f
        ));
        fall.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.949f, 0.780f, 0.161f),   // Yellow-orange
            glm::vec3(0.38f, 0.31f, 0.06f),
            glm::vec3(0.2f, 0.2f, 0.2f),
            15.0f
        ));
        fall.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.969f, 0.596f, 0.224f),   // Bright orange
            glm::vec3(0.39f, 0.24f, 0.09f),
            glm::vec3(0.2f, 0.2f, 0.2f),
            15.0f
        ));
        fall.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.969f, 0.384f, 0.224f),   // Red-orange
            glm::vec3(0.39f, 0.15f, 0.09f),
            glm::vec3(0.2f, 0.2f, 0.2f),
            15.0f
        ));
        // No flowers in fall
        fall.flowerMeshFile = "";
        oakTree.seasonalMaterials[Season::FALL] = fall;
    }
    
    // WINTER - No leaves (bare branches)
    {
        SeasonalMaterials winter;
        winter.hasLeaves = false;
        // Empty leaf materials - the tree won't generate leaves
        // No flowers in winter
        winter.flowerMeshFile = "";
        oakTree.seasonalMaterials[Season::WINTER] = winter;
    }
    
    // Register the preset
    s_presets["oak_tree"] = oakTree;
    
    s_initialized = true;
    std::cout << "PlantPresets initialized with " << s_presets.size() << " presets" << std::endl;
}

const PlantPreset* PlantPresets::getPreset(const std::string& name) {
    initializePresets();
    
    auto it = s_presets.find(name);
    if (it != s_presets.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<std::string> PlantPresets::getAvailablePresets() {
    initializePresets();
    
    std::vector<std::string> names;
    for (const auto& [name, preset] : s_presets) {
        names.push_back(name);
    }
    return names;
}

void PlantPresets::applySeasonToLSystem(LSystemData& lsystem, const PlantPreset& preset, Season season,
                                         const std::string& basePath) {

    auto it = preset.seasonalMaterials.find(season);

    if (it == preset.seasonalMaterials.end()) {
        std::cout << "Warning: Season not found in preset, using SUMMER" << std::endl;
        it = preset.seasonalMaterials.find(Season::SUMMER);
        if (it == preset.seasonalMaterials.end()) {
            std::cout << "Error: No seasonal materials found in preset" << std::endl;
            return;
        }
    }
    
    const SeasonalMaterials& seasonal = it->second;
    
    // apply leaf materials
    lsystem.leafMaterials = seasonal.leafMaterials;
    
    // apply flower materials and mesh
    lsystem.flowerMaterials = seasonal.flowerMaterials;

    if (!seasonal.flowerMeshFile.empty() && !basePath.empty()) {
        lsystem.flowerMeshFile = basePath + "/" + seasonal.flowerMeshFile;
    } else {
        lsystem.flowerMeshFile = seasonal.flowerMeshFile;
    }
}

LSystemData PlantPresets::createLSystemData(const PlantPreset& preset, Season season, const std::string& basePath) {

    LSystemData lsystem;
    lsystem.valid = true;
    
    // copy grammar
    lsystem.axiom = preset.axiom;
    lsystem.iterations = preset.iterations;
    lsystem.angle = glm::radians(preset.angle);
    lsystem.step = preset.step;
    
    // copy primitives
    lsystem.stemPrimitive = preset.stemPrimitive;
    lsystem.leafPrimitive = preset.leafPrimitive;
    
    // copy stem material
    lsystem.stemMaterial = preset.stemMaterial;
    
    // resolve texture paths if basePath provided
    if (!basePath.empty()) {
        if (lsystem.stemMaterial.textureMap.isUsed) {
            lsystem.stemMaterial.textureMap.filename = basePath + "/" + lsystem.stemMaterial.textureMap.filename;
        }
        if (lsystem.stemMaterial.normalMap.isUsed) {
            lsystem.stemMaterial.normalMap.filename = basePath + "/" + lsystem.stemMaterial.normalMap.filename;
        }
    }
    
    // copy rules
    lsystem.rules = preset.rules;
    
    // apply seasonal materials
    applySeasonToLSystem(lsystem, preset, season, basePath);
    
    return lsystem;
}
