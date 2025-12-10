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
    
    // ========================================
    // BUSH
    // Based on bush.json
    // ========================================
    PlantPreset bush;
    bush.name = "bush";
    
    // grammar
    bush.axiom = "A(0.5,0.7)";
    bush.iterations = 7;
    bush.angle = 22.5f;
    bush.step = 1.0f;
    
    // primitives
    bush.stemPrimitive = PrimitiveType::PRIMITIVE_CYLINDER;
    bush.leafPrimitive = PrimitiveType::PRIMITIVE_CONE;
    
    // stem material (brown woody stem)
    bush.stemMaterial = makeMaterial(
        glm::vec3(0.4f, 0.28f, 0.18f),
        glm::vec3(0.16f, 0.11f, 0.07f),
        glm::vec3(0.1f, 0.1f, 0.1f),
        5.0f
    );
    
    // Bush rules
    // Rule A with thick > 0.05
    {
        LSystemRule rule;
        rule.input = "A";
        rule.params = {"len", "thick"};
        rule.condition = "thick > 0.05";
        rule.output = "[&F(len,thick)L(1.5)A(len*0.9,thick*0.7)]/////[^+F(len*0.9,thick*0.7)L(1.5)A(len*0.81,thick*0.49)]/////[&-F(len*0.81,thick*0.49)L(1.5)A(len*0.73,thick*0.34)]";
        rule.probability = 1.0f;
        bush.rules.push_back(rule);
    }
    // Terminal A with flowers (30% chance)
    {
        LSystemRule rule;
        rule.input = "A";
        rule.params = {"len", "thick"};
        rule.condition = "thick <= 0.05";
        rule.output = "[^^L(1.5)]//[&+L(1.5)]///[^-W(0.12)]///[&L(1.5)]";
        rule.probability = 0.3f;
        bush.rules.push_back(rule);
    }
    // Terminal A without flowers (70% chance)
    {
        LSystemRule rule;
        rule.input = "A";
        rule.params = {"len", "thick"};
        rule.condition = "thick <= 0.05";
        rule.output = "[^^L(1.5)]//[&+L(1.5)]///[^-L(1.5)]///[&L(1.5)]";
        rule.probability = 0.7f;
        bush.rules.push_back(rule);
    }
    // F rules
    {
        LSystemRule rule;
        rule.input = "F";
        rule.params = {"len", "thick"};
        rule.condition = "thick > 0.05";
        rule.output = "S(len,thick)/////F(len*0.9,thick*0.85)";
        rule.probability = 1.0f;
        bush.rules.push_back(rule);
    }
    {
        LSystemRule rule;
        rule.input = "F";
        rule.params = {"len", "thick"};
        rule.condition = "thick <= 0.05";
        rule.output = "F(len*0.5,thick)L(1.5)";
        rule.probability = 1.0f;
        bush.rules.push_back(rule);
    }
    // S rules
    {
        LSystemRule rule;
        rule.input = "S";
        rule.params = {"len", "thick"};
        rule.condition = "thick > 0.05";
        rule.output = "F(len,thick)L(1.5)";
        rule.probability = 1.0f;
        bush.rules.push_back(rule);
    }
    {
        LSystemRule rule;
        rule.input = "S";
        rule.params = {"len", "thick"};
        rule.condition = "thick <= 0.05";
        rule.output = "L(1.5)";
        rule.probability = 1.0f;
        bush.rules.push_back(rule);
    }
    
    // BUSH SEASONAL MATERIALS
    
    // SUMMER - bright green leaves, white flowers
    {
        SeasonalMaterials summer;
        summer.hasLeaves = true;
        summer.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.3f, 0.65f, 0.2f),    // bright green
            glm::vec3(0.12f, 0.26f, 0.08f),
            glm::vec3(0.15f, 0.15f, 0.15f),
            10.0f
        ));
        summer.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.25f, 0.6f, 0.18f),
            glm::vec3(0.1f, 0.24f, 0.07f),
            glm::vec3(0.15f, 0.15f, 0.15f),
            10.0f
        ));
        summer.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.35f, 0.7f, 0.22f),
            glm::vec3(0.14f, 0.28f, 0.09f),
            glm::vec3(0.15f, 0.15f, 0.15f),
            10.0f
        ));
        // White flowers for summer
        summer.flowerMeshFile = "meshes/Lilly.obj";
        summer.flowerMaterials.push_back(makeMaterial(
            glm::vec3(0.98f, 0.98f, 0.95f),
            glm::vec3(0.5f, 0.5f, 0.48f),
            glm::vec3(0.4f, 0.4f, 0.4f),
            8.0f
        ));
        summer.flowerMaterials.push_back(makeMaterial(
            glm::vec3(1.0f, 1.0f, 0.97f),
            glm::vec3(0.5f, 0.5f, 0.49f),
            glm::vec3(0.4f, 0.4f, 0.4f),
            8.0f
        ));
        bush.seasonalMaterials[Season::SUMMER] = summer;
    }
    
    // SPRING - bright green, pink/white flowers
    {
        SeasonalMaterials spring;
        spring.hasLeaves = true;
        spring.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.28f, 0.62f, 0.18f),
            glm::vec3(0.11f, 0.25f, 0.07f),
            glm::vec3(0.15f, 0.15f, 0.15f),
            10.0f
        ));
        spring.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.32f, 0.67f, 0.2f),
            glm::vec3(0.13f, 0.27f, 0.08f),
            glm::vec3(0.15f, 0.15f, 0.15f),
            10.0f
        ));
        // Pink/white spring flowers
        spring.flowerMeshFile = "meshes/Lilly.obj";
        spring.flowerMaterials.push_back(makeMaterial(
            glm::vec3(1.0f, 0.85f, 0.9f),     // light pink
            glm::vec3(0.5f, 0.43f, 0.45f),
            glm::vec3(0.4f, 0.4f, 0.4f),
            8.0f
        ));
        spring.flowerMaterials.push_back(makeMaterial(
            glm::vec3(0.98f, 0.75f, 0.82f),   // deeper pink
            glm::vec3(0.49f, 0.38f, 0.41f),
            glm::vec3(0.4f, 0.4f, 0.4f),
            8.0f
        ));
        bush.seasonalMaterials[Season::SPRING] = spring;
    }
    
    // FALL - muted yellow-green, no flowers
    {
        SeasonalMaterials fall;
        fall.hasLeaves = true;
        fall.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.5f, 0.5f, 0.15f),     // olive/yellow-green
            glm::vec3(0.2f, 0.2f, 0.06f),
            glm::vec3(0.15f, 0.15f, 0.15f),
            10.0f
        ));
        fall.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.55f, 0.45f, 0.12f),   // brownish yellow
            glm::vec3(0.22f, 0.18f, 0.05f),
            glm::vec3(0.15f, 0.15f, 0.15f),
            10.0f
        ));
        fall.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.45f, 0.4f, 0.1f),
            glm::vec3(0.18f, 0.16f, 0.04f),
            glm::vec3(0.15f, 0.15f, 0.15f),
            10.0f
        ));
        fall.flowerMeshFile = "";  // no flowers in fall
        bush.seasonalMaterials[Season::FALL] = fall;
    }
    
    // WINTER - no leaves, no flowers
    {
        SeasonalMaterials winter;
        winter.hasLeaves = false;
        winter.flowerMeshFile = "";
        bush.seasonalMaterials[Season::WINTER] = winter;
    }
    
    s_presets["bush"] = bush;
    
    // ========================================
    // FLOWER PLANT
    // Based on flower_plant.json
    // ========================================
    PlantPreset flowerPlant;
    flowerPlant.name = "flower_plant";
    
    // grammar
    flowerPlant.axiom = "P";
    flowerPlant.iterations = 5;
    flowerPlant.angle = 18.0f;
    flowerPlant.step = 0.2f;
    
    // primitives
    flowerPlant.stemPrimitive = PrimitiveType::PRIMITIVE_CYLINDER;
    flowerPlant.leafPrimitive = PrimitiveType::PRIMITIVE_CONE;
    
    // stem material (green stem)
    flowerPlant.stemMaterial = makeMaterial(
        glm::vec3(0.2f, 0.35f, 0.1f),
        glm::vec3(0.08f, 0.14f, 0.04f),
        glm::vec3(0.1f, 0.1f, 0.1f),
        5.0f
    );
    
    // Flower plant rules - P rules
    {
        LSystemRule rule;
        rule.input = "P";
        rule.output = "I+[P+O]--//[--K]I[++K]-[PO]++PO";
        rule.probability = 0.4f;
        flowerPlant.rules.push_back(rule);
    }
    {
        LSystemRule rule;
        rule.input = "P";
        rule.output = "I+[P+O]--//[--K]I[++K]-[P]++PO";
        rule.probability = 0.3f;
        flowerPlant.rules.push_back(rule);
    }
    {
        LSystemRule rule;
        rule.input = "P";
        rule.output = "I++[P+O]---//[--K]I[+++K]-[PO]+PO";
        rule.probability = 0.2f;
        flowerPlant.rules.push_back(rule);
    }
    {
        LSystemRule rule;
        rule.input = "P";
        rule.output = "I[++K][-K]+[P]//I-[PO]++P";
        rule.probability = 0.1f;
        flowerPlant.rules.push_back(rule);
    }
    // I rules
    {
        LSystemRule rule;
        rule.input = "I";
        rule.output = "FS[//&&K][//^^K]FS";
        rule.probability = 0.5f;
        flowerPlant.rules.push_back(rule);
    }
    {
        LSystemRule rule;
        rule.input = "I";
        rule.output = "FS[//&K]FS[//^K]";
        rule.probability = 0.3f;
        flowerPlant.rules.push_back(rule);
    }
    {
        LSystemRule rule;
        rule.input = "I";
        rule.output = "FSF[//&&K][//^^K]S";
        rule.probability = 0.2f;
        flowerPlant.rules.push_back(rule);
    }
    // S rules
    {
        LSystemRule rule;
        rule.input = "S";
        rule.output = "SFS";
        rule.probability = 0.5f;
        flowerPlant.rules.push_back(rule);
    }
    {
        LSystemRule rule;
        rule.input = "S";
        rule.output = "SF";
        rule.probability = 0.3f;
        flowerPlant.rules.push_back(rule);
    }
    {
        LSystemRule rule;
        rule.input = "S";
        rule.output = "SFFS";
        rule.probability = 0.2f;
        flowerPlant.rules.push_back(rule);
    }
    // K rules (leaves)
    {
        LSystemRule rule;
        rule.input = "K";
        rule.output = "L(1)";
        rule.probability = 0.5f;
        flowerPlant.rules.push_back(rule);
    }
    {
        LSystemRule rule;
        rule.input = "K";
        rule.output = "L(1.2)";
        rule.probability = 0.25f;
        flowerPlant.rules.push_back(rule);
    }
    {
        LSystemRule rule;
        rule.input = "K";
        rule.output = "L(0.8)";
        rule.probability = 0.25f;
        flowerPlant.rules.push_back(rule);
    }
    // O rules (flower stalks)
    {
        LSystemRule rule;
        rule.input = "O";
        rule.output = "[&&&D/////////////////W(0.15)]";
        rule.probability = 0.6f;
        flowerPlant.rules.push_back(rule);
    }
    {
        LSystemRule rule;
        rule.input = "O";
        rule.output = "[&&D///////////////W(0.12)]";
        rule.probability = 0.25f;
        flowerPlant.rules.push_back(rule);
    }
    {
        LSystemRule rule;
        rule.input = "O";
        rule.output = "[&&&&D///////////////////W(0.18)]";
        rule.probability = 0.15f;
        flowerPlant.rules.push_back(rule);
    }
    // D rules
    {
        LSystemRule rule;
        rule.input = "D";
        rule.output = "FF";
        rule.probability = 0.6f;
        flowerPlant.rules.push_back(rule);
    }
    {
        LSystemRule rule;
        rule.input = "D";
        rule.output = "FFF";
        rule.probability = 0.25f;
        flowerPlant.rules.push_back(rule);
    }
    {
        LSystemRule rule;
        rule.input = "D";
        rule.output = "F";
        rule.probability = 0.15f;
        flowerPlant.rules.push_back(rule);
    }
    
    // FLOWER PLANT SEASONAL MATERIALS
    
    // SUMMER - bright leaves, white flowers
    {
        SeasonalMaterials summer;
        summer.hasLeaves = true;
        summer.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.2f, 0.55f, 0.15f),    // bright green
            glm::vec3(0.08f, 0.22f, 0.06f),
            glm::vec3(0.15f, 0.15f, 0.15f),
            10.0f
        ));
        summer.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.18f, 0.5f, 0.12f),
            glm::vec3(0.07f, 0.2f, 0.05f),
            glm::vec3(0.15f, 0.15f, 0.15f),
            10.0f
        ));
        // White flowers
        summer.flowerMeshFile = "meshes/Lilly.obj";
        summer.flowerMaterials.push_back(makeMaterial(
            glm::vec3(0.95f, 0.95f, 0.9f),
            glm::vec3(0.5f, 0.5f, 0.45f),
            glm::vec3(0.4f, 0.4f, 0.4f),
            8.0f
        ));
        summer.flowerMaterials.push_back(makeMaterial(
            glm::vec3(1.0f, 0.98f, 0.92f),
            glm::vec3(0.5f, 0.49f, 0.46f),
            glm::vec3(0.4f, 0.4f, 0.4f),
            8.0f
        ));
        flowerPlant.seasonalMaterials[Season::SUMMER] = summer;
    }
    
    // SPRING - bright leaves, yellow flowers
    {
        SeasonalMaterials spring;
        spring.hasLeaves = true;
        spring.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.22f, 0.58f, 0.16f),
            glm::vec3(0.09f, 0.23f, 0.06f),
            glm::vec3(0.15f, 0.15f, 0.15f),
            10.0f
        ));
        spring.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.2f, 0.52f, 0.14f),
            glm::vec3(0.08f, 0.21f, 0.06f),
            glm::vec3(0.15f, 0.15f, 0.15f),
            10.0f
        ));
        // Yellow flowers for spring
        spring.flowerMeshFile = "meshes/Lilly.obj";
        spring.flowerMaterials.push_back(makeMaterial(
            glm::vec3(1.0f, 0.95f, 0.3f),     // bright yellow
            glm::vec3(0.5f, 0.48f, 0.15f),
            glm::vec3(0.4f, 0.4f, 0.3f),
            8.0f
        ));
        spring.flowerMaterials.push_back(makeMaterial(
            glm::vec3(1.0f, 0.88f, 0.2f),     // golden yellow
            glm::vec3(0.5f, 0.44f, 0.1f),
            glm::vec3(0.4f, 0.4f, 0.3f),
            8.0f
        ));
        flowerPlant.seasonalMaterials[Season::SPRING] = spring;
    }
    
    // FALL - muted leaves, orange flowers
    {
        SeasonalMaterials fall;
        fall.hasLeaves = true;
        fall.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.35f, 0.4f, 0.12f),    // muted olive green
            glm::vec3(0.14f, 0.16f, 0.05f),
            glm::vec3(0.15f, 0.15f, 0.15f),
            10.0f
        ));
        fall.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.4f, 0.38f, 0.1f),     // brownish green
            glm::vec3(0.16f, 0.15f, 0.04f),
            glm::vec3(0.15f, 0.15f, 0.15f),
            10.0f
        ));
        // Orange flowers for fall
        fall.flowerMeshFile = "meshes/Lilly.obj";
        fall.flowerMaterials.push_back(makeMaterial(
            glm::vec3(1.0f, 0.55f, 0.15f),    // bright orange
            glm::vec3(0.5f, 0.28f, 0.08f),
            glm::vec3(0.4f, 0.35f, 0.3f),
            8.0f
        ));
        fall.flowerMaterials.push_back(makeMaterial(
            glm::vec3(0.95f, 0.45f, 0.1f),    // deep orange
            glm::vec3(0.48f, 0.23f, 0.05f),
            glm::vec3(0.4f, 0.35f, 0.3f),
            8.0f
        ));
        flowerPlant.seasonalMaterials[Season::FALL] = fall;
    }
    
    // WINTER - muted leaves, no flowers
    {
        SeasonalMaterials winter;
        winter.hasLeaves = true;  // flower plant keeps some leaves in winter
        winter.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.25f, 0.3f, 0.1f),     // dark muted green
            glm::vec3(0.1f, 0.12f, 0.04f),
            glm::vec3(0.1f, 0.1f, 0.1f),
            10.0f
        ));
        winter.leafMaterials.push_back(makeMaterial(
            glm::vec3(0.22f, 0.28f, 0.08f),
            glm::vec3(0.09f, 0.11f, 0.03f),
            glm::vec3(0.1f, 0.1f, 0.1f),
            10.0f
        ));
        winter.flowerMeshFile = "";  // no flowers in winter
        flowerPlant.seasonalMaterials[Season::WINTER] = winter;
    }
    
    s_presets["flower_plant"] = flowerPlant;
    
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
