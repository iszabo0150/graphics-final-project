#ifndef PLANTPRESETS_H
#define PLANTPRESETS_H

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "utils/scenedata.h"

enum class Season {
    SPRING,
    SUMMER,
    FALL,
    WINTER
};

// materials that change based on season
struct SeasonalMaterials {
    std::vector<SceneMaterial> leafMaterials;
    std::vector<SceneMaterial> flowerMaterials;
    std::string flowerMeshFile;  // empty string if no flowers for this season
    bool hasLeaves = true;       // winter trees might have no leaves
};

struct PlantPreset {
    std::string name;
    
    // grammar definition (same across all seasons)
    std::string axiom;
    std::vector<LSystemRule> rules;
    int iterations;
    float angle;  // in degrees
    float step;
    
    // primitive types
    PrimitiveType stemPrimitive;
    PrimitiveType leafPrimitive;
    
    // stem material (same across all seasons - bark doesn't change)
    SceneMaterial stemMaterial;
    
    // seasonal variants for leaves and flowers
    std::map<Season, SeasonalMaterials> seasonalMaterials;
};

// static class to manage plant presets
class PlantPresets {
public:
    // get a preset by name (returns nullptr if not found)
    static const PlantPreset* getPreset(const std::string& name);
    
    // get all available preset names
    static std::vector<std::string> getAvailablePresets();
    
    // apply seasonal materials to an LSystemData struct
    static void applySeasonToLSystem(LSystemData& lsystem, 
                                      const PlantPreset& preset, 
                                      Season season,
                                      const std::string& basePath = "");
    
    // convrt preset to LSystemData for a given season
    static LSystemData createLSystemData(const PlantPreset& preset, 
                                         Season season,
                                         const std::string& basePath = "");

private:
    // initialize all presets (called once on first access)
    static void initializePresets();
    
    // helper to create materials
    static SceneMaterial makeMaterial(const glm::vec3& diffuse, 
                                      const glm::vec3& ambient,
                                      const glm::vec3& specular, 
                                      float shininess);
    
    static SceneMaterial makeStemMaterial(const glm::vec3& diffuse,
                                          const glm::vec3& ambient,
                                          const glm::vec3& specular,
                                          float shininess,
                                          const std::string& textureFile,
                                          const std::string& normalMapFile,
                                          float blend);
    
    // storage for all presets
    static std::map<std::string, PlantPreset> s_presets;
    static bool s_initialized;
};

#endif // PLANTPRESETS_H
