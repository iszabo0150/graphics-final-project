#include "preset_utils.h"

using namespace PresetUtils;

PlantPreset createOakTreePreset() {
    PlantPreset preset;
    preset.name = "oak_tree";
    
    // grammar
    preset.axiom = "F(10,3.0)A(4.5,3.0,40)";
    preset.iterations = 8;
    preset.angle = 35.0f;
    preset.step = 1.0f;
    
    // primitives
    preset.stemPrimitive = PrimitiveType::PRIMITIVE_CYLINDER;
    preset.leafPrimitive = PrimitiveType::PRIMITIVE_CONE;
    
    // stem material (bark)
    preset.stemMaterial = makeStemMaterial(
        glm::vec3(0.35f, 0.2f, 0.12f),
        glm::vec3(0.15f, 0.08f, 0.04f),
        glm::vec3(0.1f, 0.1f, 0.1f),
        2.0f,
        "textures/pls.jpg",
        "textures/plsN.jpg",
        0.5f
    );
    
    // ============ RULES ============
    
    // Rule A with thick > 0.05
    addRule(preset, "A",
        "F(len*0.62,thick)[+(angle*0.95)^(6)B(len*1.5,thick*0.4,angle*1.12)]F(len*0.15,thick)L(1)/(117)[+(angle*1.3)^(12)B(len*1.5,thick*0.4,angle*0.92)]F(len*0.09,thick)L(1)/(122)[+(angle*0.85)^(3)B(len*1.5,thick*0.4,angle*0.88)]F(len*0.11,thick)L(1)A(len*0.93,thick*0.55,angle*1.05)",
        0.3f, {"len", "thick", "angle"}, "thick > 0.05");
    
    // Rule B variations with thick > 0.05
    addRule(preset, "B",
        "F(len*0.6,thick)L(1)^(6)[+(angle*0.78)B(len*0.88,thick*0.55,angle*0.82)]F(len*0.1,thick)L(1)/(142)[+(angle*0.62)B(len*0.88,thick*0.55,angle*0.9)]F(len*0.08,thick)L(1)/(127)[+(angle*0.95)B(len*0.88,thick*0.55,angle*0.88)]F(len*0.07,thick)L(1)",
        0.3f, {"len", "thick", "angle"}, "thick > 0.05");
    
    addRule(preset, "B",
        "F(len*0.65,thick)L(1)^(10)[+(angle*1.12)B(len*0.88,thick*0.55,angle*0.98)]F(len*0.12,thick)L(1)/(135)[+(angle*0.52)B(len*0.88,thick*0.55,angle*0.85)]F(len*0.09,thick)L(1)/(148)[+(angle*1.18)B(len*0.88,thick*0.55,angle*0.92)]F(len*0.06,thick)L(1)",
        0.3f, {"len", "thick", "angle"}, "thick > 0.05");
    
    addRule(preset, "B",
        "F(len*0.45,thick)L(1)^(4)[+(angle*0.92)B(len*0.88,thick*0.55,angle*0.95)]F(len*0.11,thick)L(1)/(118)[+(angle*0.55)B(len*0.88,thick*0.55,angle*0.92)]F(len*0.07,thick)L(1)/(155)[+(angle*0.85)B(len*0.88,thick*0.55,angle*0.9)]F(len*0.05,thick)L(1)",
        0.4f, {"len", "thick", "angle"}, "thick > 0.05");
    
    // Terminal rules (thick <= 0.05)
    addRule(preset, "A",
        "F(len*0.2,thick)[+(angle*0.8)L(1)]F(len*0.15,thick)[/(90)L(1)]F(len*0.12,thick)[/(180)L(1)]F(len*0.1,thick)[/(270)L(1)]F(len*0.08,thick)[^(20)L(1)][&(20)]",
        1.0f, {"len", "thick", "angle"}, "thick <= 0.05");
    
    // Terminal B WITH flowers - 30%
    addRule(preset, "B",
        "F(len*0.15,thick)[+(angle*0.6)L(1)]F(len*0.12,thick)[/(72)L(1)]F(len*0.1,thick)[/(144)L(1)]F(len*0.08,thick)[/(216)L(1)]F(len*0.06,thick)[/(288)L(1)]F(len*0.05,thick)[^(15)L(1)][&(15)W(0.5)]",
        0.3f, {"len", "thick", "angle"}, "thick <= 0.05");
    
    // Terminal B WITHOUT flowers - 70%
    addRule(preset, "B",
        "F(len*0.15,thick)[+(angle*0.6)L(1)]F(len*0.12,thick)[/(72)L(1)]F(len*0.1,thick)[/(144)L(1)]F(len*0.08,thick)[/(216)L(1)]F(len*0.06,thick)[/(288)L(1)]F(len*0.05,thick)[^(15)L(1)][&(15)L(1)]",
        0.7f, {"len", "thick", "angle"}, "thick <= 0.05");
    
    // ============ SEASONAL MATERIALS ============
    
    // SUMMER - lush green
    {
        SeasonalMaterials summer;
        summer.hasLeaves = true;
        summer.leafMaterials.push_back(makeMaterial({0.557f, 0.678f, 0.224f}, {0.22f, 0.27f, 0.09f}, {0.2f, 0.2f, 0.2f}, 15.0f));
        summer.leafMaterials.push_back(makeMaterial({0.522f, 0.620f, 0.247f}, {0.21f, 0.25f, 0.10f}, {0.2f, 0.2f, 0.2f}, 15.0f));
        summer.flowerMeshFile = "";
        preset.seasonalMaterials[Season::SUMMER] = summer;
    }
    
    // SPRING - green with flowers
    {
        SeasonalMaterials spring;
        spring.hasLeaves = true;
        spring.leafMaterials.push_back(makeMaterial({0.557f, 0.678f, 0.224f}, {0.22f, 0.27f, 0.09f}, {0.2f, 0.2f, 0.2f}, 15.0f));
        spring.leafMaterials.push_back(makeMaterial({0.522f, 0.620f, 0.247f}, {0.21f, 0.25f, 0.10f}, {0.2f, 0.2f, 0.2f}, 15.0f));
        spring.flowerMeshFile = "meshes/Lilly.obj";
        spring.flowerMaterials.push_back(makeMaterial({0.988f, 0.976f, 0.910f}, {0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 0.5f}, 3.0f));
        spring.flowerMaterials.push_back(makeMaterial({1.0f, 0.8f, 0.9f}, {0.5f, 0.4f, 0.45f}, {0.5f, 0.5f, 0.5f}, 3.0f));
        spring.flowerMaterials.push_back(makeMaterial({0.95f, 0.7f, 0.75f}, {0.47f, 0.35f, 0.37f}, {0.5f, 0.5f, 0.5f}, 3.0f));
        preset.seasonalMaterials[Season::SPRING] = spring;
    }
    
    // FALL - orange/red/yellow
    {
        SeasonalMaterials fall;
        fall.hasLeaves = true;
        fall.leafMaterials.push_back(makeMaterial({0.922f, 0.647f, 0.294f}, {0.37f, 0.26f, 0.12f}, {0.2f, 0.2f, 0.2f}, 15.0f));
        fall.leafMaterials.push_back(makeMaterial({0.949f, 0.780f, 0.161f}, {0.38f, 0.31f, 0.06f}, {0.2f, 0.2f, 0.2f}, 15.0f));
        fall.leafMaterials.push_back(makeMaterial({0.969f, 0.596f, 0.224f}, {0.39f, 0.24f, 0.09f}, {0.2f, 0.2f, 0.2f}, 15.0f));
        fall.leafMaterials.push_back(makeMaterial({0.969f, 0.384f, 0.224f}, {0.39f, 0.15f, 0.09f}, {0.2f, 0.2f, 0.2f}, 15.0f));
        fall.flowerMeshFile = "";
        preset.seasonalMaterials[Season::FALL] = fall;
    }
    
    // WINTER - no leaves
    {
        SeasonalMaterials winter;
        winter.hasLeaves = false;
        winter.flowerMeshFile = "";
        preset.seasonalMaterials[Season::WINTER] = winter;
    }
    
    return preset;
}
