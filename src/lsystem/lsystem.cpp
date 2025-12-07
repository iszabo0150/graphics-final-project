#include "lsystem.h"
#include "utils/scenedata.h"

#include <stack>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <regex>

/**
 * @brief LSystem::parseLSymbol turns a string into an LSymbol ! Kind of overkill right now but will eb helpful once things get fancier for the
 * final !!
 * @param tokenString
 * @return
 */
LSymbol LSystem::parseLSymbol(const std::string &tokenString){
    LSymbol symbol;
    symbol.name = tokenString;
    return symbol;
}

/**
 * @brief LSystem::tokenize converts the string from teh json file into a list of LSymbols to be used to create our System !!!
 * @param string
 * @return
 */
std::vector<LSymbol> LSystem::tokenize(const std::string &string){
    std::vector<LSymbol> tokenizedResult;

    // extracts letters and operands from the string: eg  A, F, L, +, -, [, ], &, ^, \, /
    std::regex tokenRegex(R"([A-Za-z]|[\+\-\[\]\&\^\\\/])");

    //looping thorugh all the matches
    for (auto match = std::sregex_iterator(string.begin(), string.end(), tokenRegex); match != std::sregex_iterator(); match++){
        tokenizedResult.push_back(parseLSymbol(match->str()));
    }

    return tokenizedResult;
}

/**
 * @brief LSystem::applyRule expands the string based on teh symbol's rule !
 * @param symbol
 * @param data
 * @return
 */
std::string LSystem::applyRule(const LSymbol &symbol, const LSystemData &data){
    for (const auto &rule : data.rules) {
        if (rule.input == symbol.name) {
            return rule.output;
        }
    }
    return symbol.name; //if there's no rule for a symbol it stays as is !
}

/**
 * @brief LSystem::expandLSystem expands an L system based on the current nodes rules. iterated through the string depending on the amount
 * of iterations, and for each iteration expands based on teh data's rules
 * @param data
 * @return
 */
std::vector<LSymbol> LSystem::expandLSystem(const LSystemData &data){
    std::string current = data.axiom;

    for (int i = 0; i < data.iterations; i++) {
        auto symbols = tokenize(current);
        std::string next;

        for (auto &s : symbols) {
            next += applyRule(s, data);
        }

        current = next;
    }

    return tokenize(current);
}

/**
 * @brief LSystem::interpretLSystem interprets an L system based on turtle grammar !!
 * @param data the configurations / rules for the l system
 * @param symbols the symbosl that make up the rules
 * @param stemCTMs
 * @param leafCTMs
 */
void LSystem::interpretLSystem(const LSystemData &data, const std::vector<LSymbol> &symbols, std::vector<glm::mat4> &stemCTMs,
                               std::vector<glm::mat4> &leafCTMs){

    TState turtle;
    turtle.pos = glm::vec3(0, 0, 0);
    turtle.heading = glm::vec3(0, 1, 0);  // forward direction (Y+)
    turtle.left    = glm::vec3(-1, 0, 0); // left direction (X-)
    turtle.up      = glm::vec3(0, 0, 1);  // up direction (Z+)

    std::stack<TState> stateStack;

    for (const auto &sym : symbols) {

        if (sym.name == "F") { // move forward and draw stem
            float segmentLength = data.step;

            glm::vec3 center = turtle.pos + turtle.heading * (segmentLength * 0.5f);

            // make rotation matrix from turtle's orientation vectors !
            glm::mat4 rot(1.0f);
            rot[0] = glm::vec4(turtle.left,   0);
            rot[1] = glm::vec4(turtle.heading, 0);
            rot[2] = glm::vec4(turtle.up,     0);

            glm::mat4 ctm = glm::translate(glm::mat4(1.0f), center) * rot;
            ctm = glm::scale(ctm, glm::vec3(0.05f, segmentLength, 0.05f));

            stemCTMs.push_back(ctm);
            turtle.pos += turtle.heading * segmentLength;
        }

        else if (sym.name == "L") { // create a leaf
            glm::mat4 leaf = glm::translate(glm::mat4(1.0f), turtle.pos);
            leaf = glm::scale(leaf, glm::vec3(0.1f));
            leafCTMs.push_back(leaf);
        }

        else if (sym.name == "+") { // turn left (counter-clockwise around up axis)
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), data.angle, turtle.up);
            turtle.heading = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.heading, 0)));
            turtle.left    = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.left, 0)));
        }

        else if (sym.name == "-") { // turn right (clockwise around up axis)
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), -data.angle, turtle.up);
            turtle.heading = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.heading, 0)));
            turtle.left    = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.left, 0)));
        }

        else if (sym.name == "&") { // pitch down (rotate heading down around left axis)
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), data.angle, turtle.left);
            turtle.heading = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.heading, 0)));
            turtle.up      = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.up, 0)));
        }

        else if (sym.name == "^") { // pitch up (rotate heading up around left axis)
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), -data.angle, turtle.left);
            turtle.heading = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.heading, 0)));
            turtle.up      = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.up, 0)));
        }

        else if (sym.name == "\\") { // roll left (rotate left around heading axis)
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), data.angle, turtle.heading);
            turtle.left = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.left, 0)));
            turtle.up   = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.up, 0)));
        }

        else if (sym.name == "/") { // roll right (rotate right around heading axis)
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), -data.angle, turtle.heading);
            turtle.left = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.left, 0)));
            turtle.up   = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.up, 0)));
        }

        else if (sym.name == "[") { // save the current state to return to
            stateStack.push(turtle);
        }

        else if (sym.name == "]") { // return back to the last saved state
            if (!stateStack.empty()) {
                turtle = stateStack.top();
                stateStack.pop();
            }
        }
    }
}
