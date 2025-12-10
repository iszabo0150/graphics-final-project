#include "plantpresets.h"
#include "presets/preset_utils.h"
#include <iostream>

// static member initialization
std::map<std::string, PlantPreset> PlantPresets::s_presets;
bool PlantPresets::s_initialized = false;

void PlantPresets::initializePresets() {
    if (s_initialized) return;
    
    // register all presets
    s_presets["oak_tree"] = createOakTreePreset();
    s_presets["bush"] = createBushPreset();
    s_presets["flower_plant"] = createFlowerPlantPreset();
    
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

void PlantPresets::applySeasonToLSystem(LSystemData& lsystem, const PlantPreset& preset, 
                                         Season season, const std::string& basePath) {
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

LSystemData PlantPresets::createLSystemData(const PlantPreset& preset, Season season, 
                                             const std::string& basePath) {
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
