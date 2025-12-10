#include "preset_utils.h"

using namespace PresetUtils;

PlantPreset createFlowerPlantPreset() {
    PlantPreset preset;
    preset.name = "flower_plant";
    
    // grammar
    preset.axiom = "P";
    preset.iterations = 5;
    preset.angle = 18.0f;
    preset.step = 0.2f;
    
    // primitives
    preset.stemPrimitive = PrimitiveType::PRIMITIVE_CYLINDER;
    preset.leafPrimitive = PrimitiveType::PRIMITIVE_CONE;
    
    // stem material (green)
    preset.stemMaterial = makeMaterial(
        glm::vec3(0.2f, 0.35f, 0.1f),
        glm::vec3(0.08f, 0.14f, 0.04f),
        glm::vec3(0.1f, 0.1f, 0.1f),
        5.0f
    );
    
    // rulessss
    
    // p rules
    addRule(preset, "P", "I+[P+O]--//[--K]I[++K]-[PO]++PO", 0.4f);
    addRule(preset, "P", "I+[P+O]--//[--K]I[++K]-[P]++PO", 0.3f);
    addRule(preset, "P", "I++[P+O]---//[--K]I[+++K]-[PO]+PO", 0.2f);
    addRule(preset, "P", "I[++K][-K]+[P]//I-[PO]++P", 0.1f);
    
    // i rules
    addRule(preset, "I", "FS[//&&K][//^^K]FS", 0.5f);
    addRule(preset, "I", "FS[//&K]FS[//^K]", 0.3f);
    addRule(preset, "I", "FSF[//&&K][//^^K]S", 0.2f);
    
    // s rules
    addRule(preset, "S", "SFS", 0.5f);
    addRule(preset, "S", "SF", 0.3f);
    addRule(preset, "S", "SFFS", 0.2f);
    
    // k rules (leaves)
    addRule(preset, "K", "L(1)", 0.5f);
    addRule(preset, "K", "L(1.2)", 0.25f);
    addRule(preset, "K", "L(0.8)", 0.25f);
    
    // o rules (flower stalks)
    addRule(preset, "O", "[&&&D/////////////////W(0.15)]", 0.6f);
    addRule(preset, "O", "[&&D///////////////W(0.12)]", 0.25f);
    addRule(preset, "O", "[&&&&D///////////////////W(0.18)]", 0.15f);
    
    // d rules
    addRule(preset, "D", "FF", 0.6f);
    addRule(preset, "D", "FFF", 0.25f);
    addRule(preset, "D", "F", 0.15f);
    
    // materiallllssss
    
    // summer - bright leaves, white flowers
    {
        SeasonalMaterials summer;
        summer.hasLeaves = true;
        summer.leafMaterials.push_back(makeMaterial({0.2f, 0.55f, 0.15f}, {0.08f, 0.22f, 0.06f}, {0.15f, 0.15f, 0.15f}, 10.0f));
        summer.leafMaterials.push_back(makeMaterial({0.18f, 0.5f, 0.12f}, {0.07f, 0.2f, 0.05f}, {0.15f, 0.15f, 0.15f}, 10.0f));
        summer.flowerMeshFile = "meshes/Lilly.obj";
        summer.flowerMaterials.push_back(makeMaterial({0.95f, 0.95f, 0.9f}, {0.5f, 0.5f, 0.45f}, {0.4f, 0.4f, 0.4f}, 8.0f));
        summer.flowerMaterials.push_back(makeMaterial({1.0f, 0.98f, 0.92f}, {0.5f, 0.49f, 0.46f}, {0.4f, 0.4f, 0.4f}, 8.0f));
        preset.seasonalMaterials[Season::SUMMER] = summer;
    }
    
    // spring - bright leaves, yellow flowers
    {
        SeasonalMaterials spring;
        spring.hasLeaves = true;
        spring.leafMaterials.push_back(makeMaterial({0.22f, 0.58f, 0.16f}, {0.09f, 0.23f, 0.06f}, {0.15f, 0.15f, 0.15f}, 10.0f));
        spring.leafMaterials.push_back(makeMaterial({0.2f, 0.52f, 0.14f}, {0.08f, 0.21f, 0.06f}, {0.15f, 0.15f, 0.15f}, 10.0f));
        spring.flowerMeshFile = "meshes/Lilly.obj";
        spring.flowerMaterials.push_back(makeMaterial({1.0f, 0.95f, 0.3f}, {0.5f, 0.48f, 0.15f}, {0.4f, 0.4f, 0.3f}, 8.0f));
        spring.flowerMaterials.push_back(makeMaterial({1.0f, 0.88f, 0.2f}, {0.5f, 0.44f, 0.1f}, {0.4f, 0.4f, 0.3f}, 8.0f));
        preset.seasonalMaterials[Season::SPRING] = spring;
    }
    
    // fall - muted leaves, orange flowers
    {
        SeasonalMaterials fall;
        fall.hasLeaves = true;
        fall.leafMaterials.push_back(makeMaterial({0.35f, 0.4f, 0.12f}, {0.14f, 0.16f, 0.05f}, {0.15f, 0.15f, 0.15f}, 10.0f));
        fall.leafMaterials.push_back(makeMaterial({0.4f, 0.38f, 0.1f}, {0.16f, 0.15f, 0.04f}, {0.15f, 0.15f, 0.15f}, 10.0f));
        fall.flowerMeshFile = "meshes/Lilly.obj";
        fall.flowerMaterials.push_back(makeMaterial({1.0f, 0.55f, 0.15f}, {0.5f, 0.28f, 0.08f}, {0.4f, 0.35f, 0.3f}, 8.0f));
        fall.flowerMaterials.push_back(makeMaterial({0.95f, 0.45f, 0.1f}, {0.48f, 0.23f, 0.05f}, {0.4f, 0.35f, 0.3f}, 8.0f));
        preset.seasonalMaterials[Season::FALL] = fall;
    }
    
    // winter - muted leaves, no flowers
    {
        SeasonalMaterials winter;
        winter.hasLeaves = true;
        winter.leafMaterials.push_back(makeMaterial({0.25f, 0.3f, 0.1f}, {0.1f, 0.12f, 0.04f}, {0.1f, 0.1f, 0.1f}, 10.0f));
        winter.leafMaterials.push_back(makeMaterial({0.22f, 0.28f, 0.08f}, {0.09f, 0.11f, 0.03f}, {0.1f, 0.1f, 0.1f}, 10.0f));
        winter.flowerMeshFile = "";
        preset.seasonalMaterials[Season::WINTER] = winter;
    }
    
    return preset;
}
