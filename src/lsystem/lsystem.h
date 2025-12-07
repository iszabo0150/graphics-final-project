#ifndef LSYSTEM_H
#define LSYSTEM_H

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "utils/scenedata.h"


struct LSymbol {
    std::string name;       // "F", "+", "[", etc.
    std::vector<float> args; // numeric arguments
};

struct TState {
    glm::vec3 pos;
    glm::vec3 heading;
    glm::vec3 left;
    glm::vec3 up;
};


class LSystem {
public:
    // Parsing & tokenizing
    static LSymbol parseLSymbol(const std::string &tokenString);
    static std::vector<LSymbol> tokenize(const std::string &string);

    // L-system expansion (no params)
    static std::string applyRule(const LSymbol &symbol, const LSystemData &data);
    static std::vector<LSymbol> expandLSystem(const LSystemData &data);

    // Interpretation → produce CTMs for stems & leaves
    static void interpretLSystem(const LSystemData &data,
                          const std::vector<LSymbol> &symbols,
                          std::vector<glm::mat4> &stemCTMs,
                          std::vector<glm::mat4> &leafCTMs);
};

#endif // LSYSTEM_H
