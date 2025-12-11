#include "preset_utils.h"

using namespace PresetUtils;

PlantPreset createBushPreset() {
    PlantPreset preset;
    preset.name = "bush";
    
    // grammar
    preset.axiom = "A(0.5,0.7)";
    preset.iterations = 7;
    preset.angle = 22.5f;
    preset.step = 1.0f;
    
    // primitives
    preset.stemPrimitive = PrimitiveType::PRIMITIVE_CYLINDER;
    preset.leafPrimitive = PrimitiveType::PRIMITIVE_CONE;
    
    // stem material (brown woody)
    preset.stemMaterial = makeMaterial(
        glm::vec3(0.4f, 0.28f, 0.18f),
        glm::vec3(0.16f, 0.11f, 0.07f),
        glm::vec3(0.1f, 0.1f, 0.1f),
        5.0f
    );
    
    // rulessssss
    
    // rule A with thick > 0.05
    addRule(preset, "A",
        "[&F(len,thick)L(1)A(len*0.9,thick*0.7)]/////[^+F(len*0.9,thick*0.7)L(1)A(len*0.81,thick*0.49)]/////[&-F(len*0.81,thick*0.49)L(1)A(len*0.73,thick*0.34)]",
        1.0f, {"len", "thick"}, "thick > 0.05");
    
    // terminal A WITH flowers - 30%
    addRule(preset, "A",
        "[^^L(1)]//[&+L(1)]///[^-W(0.5)]",
        0.3f, {"len", "thick"}, "thick <= 0.05");
    
    // terminal A WITHOUT flowers - 70%
    addRule(preset, "A",
        "[^^L(1)]//[&+L(1)]///[^-L(1)]///[&L(1)]",
        0.7f, {"len", "thick"}, "thick <= 0.05");
    
    // f rules
    addRule(preset, "F",
        "S(len,thick)/////F(len*0.9,thick*0.85)",
        1.0f, {"len", "thick"}, "thick > 0.05");
    
    addRule(preset, "F",
        "F(len*0.5,thick)L(1)",
        1.0f, {"len", "thick"}, "thick <= 0.05");
    
    // s rules
    addRule(preset, "S",
        "F(len,thick)L(1)",
        1.0f, {"len", "thick"}, "thick > 0.05");
    
    addRule(preset, "S",
        "L(1)",
        1.0f, {"len", "thick"}, "thick <= 0.05");
    
    // materialsssss
    
    // summer - bright green, white flowers with variations
    {
        SeasonalMaterials summer;
        summer.hasLeaves = true;
        summer.leafMaterials.push_back(makeMaterial({0.557f, 0.678f, 0.224f}, {0.22f, 0.27f, 0.09f}, {0.2f, 0.2f, 0.2f}, 15.0f));
        summer.leafMaterials.push_back(makeMaterial({0.522f, 0.620f, 0.247f}, {0.21f, 0.25f, 0.10f}, {0.2f, 0.2f, 0.2f}, 15.0f));        makeMaterial({0.329f, 0.369f, 0.067f}, {0.13f, 0.15f, 0.03f}, {0.2f, 0.2f, 0.2f}, 15.0f);
        summer.flowerMeshFile = "meshes/Lilly.obj";
        // White
        summer.flowerMaterials.push_back(makeMaterial({0.98f, 0.98f, 0.95f}, {0.5f, 0.5f, 0.48f}, {0.4f, 0.4f, 0.4f}, 8.0f));
        summer.flowerMaterials.push_back(makeMaterial({1.0f, 1.0f, 0.97f}, {0.5f, 0.5f, 0.49f}, {0.4f, 0.4f, 0.4f}, 8.0f));
        // Soft pink
        summer.flowerMaterials.push_back(makeMaterial({1.0f, 0.88f, 0.9f}, {0.5f, 0.44f, 0.45f}, {0.4f, 0.4f, 0.4f}, 8.0f));
        // Light blue
        summer.flowerMaterials.push_back(makeMaterial({0.85f, 0.92f, 1.0f}, {0.43f, 0.46f, 0.5f}, {0.4f, 0.4f, 0.4f}, 8.0f));
        // Pale yellow
        summer.flowerMaterials.push_back(makeMaterial({1.0f, 1.0f, 0.8f}, {0.5f, 0.5f, 0.4f}, {0.4f, 0.4f, 0.4f}, 8.0f));
        preset.seasonalMaterials[Season::SUMMER] = summer;
    }
    
    // spring - bright green, pink flowers with variations
    {
        SeasonalMaterials spring;
        spring.hasLeaves = true;
        spring.leafMaterials.push_back(makeMaterial({0.557f, 0.678f, 0.224f}, {0.22f, 0.27f, 0.09f}, {0.2f, 0.2f, 0.2f}, 15.0f));
        spring.leafMaterials.push_back(makeMaterial({0.522f, 0.620f, 0.247f}, {0.21f, 0.25f, 0.10f}, {0.2f, 0.2f, 0.2f}, 15.0f));        makeMaterial({0.329f, 0.369f, 0.067f}, {0.13f, 0.15f, 0.03f}, {0.2f, 0.2f, 0.2f}, 15.0f);
        spring.flowerMeshFile = "meshes/Lilly.obj";
        // Pink
        spring.flowerMaterials.push_back(makeMaterial({1.0f, 0.85f, 0.9f}, {0.5f, 0.43f, 0.45f}, {0.4f, 0.4f, 0.4f}, 8.0f));
        spring.flowerMaterials.push_back(makeMaterial({0.98f, 0.75f, 0.82f}, {0.49f, 0.38f, 0.41f}, {0.4f, 0.4f, 0.4f}, 8.0f));
        // Hot pink
        spring.flowerMaterials.push_back(makeMaterial({1.0f, 0.55f, 0.7f}, {0.5f, 0.28f, 0.35f}, {0.4f, 0.4f, 0.4f}, 8.0f));
        // Light purple
        spring.flowerMaterials.push_back(makeMaterial({0.9f, 0.75f, 1.0f}, {0.45f, 0.38f, 0.5f}, {0.4f, 0.4f, 0.4f}, 8.0f));
        // Peach
        spring.flowerMaterials.push_back(makeMaterial({1.0f, 0.8f, 0.7f}, {0.5f, 0.4f, 0.35f}, {0.4f, 0.4f, 0.4f}, 8.0f));
        // White
        spring.flowerMaterials.push_back(makeMaterial({1.0f, 0.98f, 0.98f}, {0.5f, 0.49f, 0.49f}, {0.4f, 0.4f, 0.4f}, 8.0f));
        preset.seasonalMaterials[Season::SPRING] = spring;
    }
    
    // fall - muted yellow-green, no flowers
    {
        SeasonalMaterials fall;
        fall.hasLeaves = true;
        fall.leafMaterials.push_back(makeMaterial({0.5f, 0.5f, 0.15f}, {0.2f, 0.2f, 0.06f}, {0.15f, 0.15f, 0.15f}, 10.0f));
        fall.leafMaterials.push_back(makeMaterial({0.55f, 0.45f, 0.12f}, {0.22f, 0.18f, 0.05f}, {0.15f, 0.15f, 0.15f}, 10.0f));
        fall.leafMaterials.push_back(makeMaterial({0.45f, 0.4f, 0.1f}, {0.18f, 0.16f, 0.04f}, {0.15f, 0.15f, 0.15f}, 10.0f));
        fall.flowerMeshFile = "";
        preset.seasonalMaterials[Season::FALL] = fall;
    }
    
    // winter - no leaves, no flowers
    {
        SeasonalMaterials winter;
        winter.hasLeaves = false;
        winter.flowerMeshFile = "";
        preset.seasonalMaterials[Season::WINTER] = winter;
    }
    
    return preset;
}
