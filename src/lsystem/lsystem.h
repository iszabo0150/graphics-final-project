#ifndef LSYSTEM_H
#define LSYSTEM_H
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <map>
#include "utils/scenedata.h"

struct LSymbol {
    std::string name;       // "F", "+", "[", etc.
    std::vector<float> params; // numeric arguments
};

struct StemData {
    glm::mat4 ctm;
    float thickness;
    float length;
};

struct TState {
    glm::vec3 pos;
    glm::vec3 heading;
    glm::vec3 left;
    glm::vec3 up;
    float lastThickness = 0.05f;
};

class LSystem {
public:
    // Parsing & tokenizing
    static LSymbol parseLSymbol(const std::string &tokenString);
    static std::vector<LSymbol> tokenize(const std::string &string);

    // L-system expansion (with params)
    static std::string applyRule(const LSymbol &symbol, const LSystemData &data);
    static std::vector<LSymbol> expandLSystem(const LSystemData &data);

    // Interpretation → produce CTMs for stems & leaves
    static void interpretLSystem(const LSystemData &data, const std::vector<LSymbol> &symbols,
                          std::vector<StemData> &stems, std::vector<glm::mat4> &leafCTMs);

    // Parametric L-system helpers
    static bool evaluateCondition(const std::string &condition,
                                  const std::map<std::string, float> &paramValues);
    static float evaluateExpression(const std::string &expr,
                                    const std::map<std::string, float> &paramValues);
    static std::string substituteParams(const std::string &output,
                                        const std::map<std::string, float> &paramValues);
    static float evaluateArithmetic(const std::string &expr);
};

#endif // LSYSTEM_H
