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
LSymbol LSystem::parseLSymbol(const std::string &tokenString) {
    LSymbol symbol;

    // match letter for rule optionally followed by parameterrs
    std::regex symbolRegex(R"(([A-Za-z+\-\[\]&\^\\\/])(?:\(([^)]*)\))?)");
    std::smatch match;

    if (std::regex_match(tokenString, match, symbolRegex)) {
        symbol.name = match[1].str();

        // parse parameters if present
        if (match[2].matched && !match[2].str().empty()) {
            std::string paramStr = match[2].str();
            std::stringstream ss(paramStr);
            std::string param;

            while (std::getline(ss, param, ',')) {
                // trim whitespace
                param.erase(0, param.find_first_not_of(" \t"));
                param.erase(param.find_last_not_of(" \t") + 1);
                try {
                    symbol.params.push_back(std::stof(param));
                } catch (...) {
                    symbol.params.push_back(0.0f);
                }
            }
        }
    } else {
        symbol.name = tokenString;
    }

    return symbol;
}

/**
 * @brief LSystem::tokenize converts the string from teh json file into a list of LSymbols to be used to create our System !!!
 * @param string
 * @return
 */
std::vector<LSymbol> LSystem::tokenize(const std::string &string) {
    std::vector<LSymbol> tokenizedResult;

    // match symbols (+, -, / etc etc) with optional parameters
    std::regex tokenRegex(R"([A-Za-z+\-\[\]&\^\\\/](?:\([^)]*\))?)");

    for (auto match = std::sregex_iterator(string.begin(), string.end(), tokenRegex);
         match != std::sregex_iterator(); ++match) {
        tokenizedResult.push_back(parseLSymbol(match->str()));
    }

    return tokenizedResult;
}

bool LSystem::evaluateCondition(const std::string &condition,
                                const std::map<std::string, float> &paramValues) {
    if (condition.empty()) return true;

    // replace parameter names with values
    std::string expr = condition;
    for (const auto &[name, value] : paramValues) {
        size_t pos = 0;
        while ((pos = expr.find(name, pos)) != std::string::npos) {
            // make sure it's a whole word (not part of another variable)
            if ((pos == 0 || !isalnum(expr[pos-1])) &&
                (pos + name.length() >= expr.length() || !isalnum(expr[pos + name.length()]))) {
                expr.replace(pos, name.length(), std::to_string(value));
            }
            pos += std::to_string(value).length();
        }
    }


    std::regex compRegex(R"(([\d.]+)\s*(>=|<=|>|<|==|!=)\s*([\d.]+))");
    std::smatch match;

    if (std::regex_search(expr, match, compRegex)) {
        float left = std::stof(match[1].str());
        std::string op = match[2].str();
        float right = std::stof(match[3].str());

        if (op == ">") return left > right;
        if (op == "<") return left < right;
        if (op == ">=") return left >= right;
        if (op == "<=") return left <= right;
        if (op == "==") return std::abs(left - right) < 1e-6f;
        if (op == "!=") return std::abs(left - right) >= 1e-6f;
    }

    return true; // default to true if can't parse
}

std::string LSystem::substituteParams(const std::string &output,
                                      const std::map<std::string, float> &paramValues) {
    std::string result;
    size_t i = 0;

    while (i < output.length()) {
        // check if we're at an opening parenthesis which starts a parameter list
        if (output[i] == '(') {
            result += '(';
            i++;

            // find the matching closing parenthesis
            size_t start = i;
            int depth = 1;
            size_t end = i;

            while (end < output.length() && depth > 0) {
                if (output[end] == '(') depth++;
                if (output[end] == ')') depth--;
                if (depth > 0) end++;
            }

            if (depth == 0) {
                // extract everything inside parentheses
                std::string paramList = output.substr(start, end - start);

                // split by commas (but not commas inside nested parentheses)
                std::vector<std::string> params;
                std::string currentParam;
                int parenDepth = 0;

                for (char c : paramList) {
                    if (c == '(') parenDepth++;
                    else if (c == ')') parenDepth--;
                    else if (c == ',' && parenDepth == 0) {
                        params.push_back(currentParam);
                        currentParam.clear();
                        continue;
                    }
                    currentParam += c;
                }
                if (!currentParam.empty()) {
                    params.push_back(currentParam);
                }

                // proccewss each parameter
                for (size_t p = 0; p < params.size(); p++) {
                    if (p > 0) result += ",";

                    std::string expr = params[p];

                    // replace all parameters in this expression
                    for (const auto &[name, value] : paramValues) {
                        size_t pos = 0;
                        while ((pos = expr.find(name, pos)) != std::string::npos) {
                            bool before = (pos == 0 || !isalnum(expr[pos-1]));
                            bool after = (pos + name.length() >= expr.length() || !isalnum(expr[pos + name.length()]));

                            if (before && after) {
                                expr.replace(pos, name.length(), std::to_string(value));
                            }
                            pos++;
                        }
                    }

                    // evaluate the expression
                    float val = evaluateArithmetic(expr);
                    result += std::to_string(val);
                }

                result += ')';
                i = end + 1;
                continue;
            }
        }

        result += output[i];
        i++;
    }

    return result;
}
// helper function to evaluate arithmetic expressions
float LSystem::evaluateArithmetic(const std::string &expr) {

    // handler multiplication and division first (PEMDAS yasss)
    for (size_t i = 0; i < expr.length(); i++) {
        if (expr[i] == '*' || expr[i] == '/') {

            size_t leftStart = i;
            while (leftStart > 0 && (isdigit(expr[leftStart-1]) || expr[leftStart-1] == '.')) {
                leftStart--;
            }

            size_t rightEnd = i + 1;
            while (rightEnd < expr.length() && (isdigit(expr[rightEnd]) || expr[rightEnd] == '.')) {
                rightEnd++;
            }

            float left = std::stof(expr.substr(leftStart, i - leftStart));
            float right = std::stof(expr.substr(i + 1, rightEnd - i - 1));
            float result = (expr[i] == '*') ? left * right : left / right;

            std::string newExpr = expr.substr(0, leftStart) +
                                  std::to_string(result) +
                                  expr.substr(rightEnd);
            return evaluateArithmetic(newExpr);
        }
    }

    // handle addition and subtraction
    for (size_t i = 1; i < expr.length(); i++) { // start at 1 to handle negative numbers
        if (expr[i] == '+' || expr[i] == '-') {
            float left = evaluateArithmetic(expr.substr(0, i));
            float right = evaluateArithmetic(expr.substr(i + 1));
            return (expr[i] == '+') ? left + right : left - right;
        }
    }

    // base case: just a number
    return std::stof(expr);
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

std::string LSystem::applyRule(const LSymbol &symbol, const LSystemData &data) {
    struct Candidate {
        const LSystemRule* rule;
        std::map<std::string, float> paramValues;
    };

    std::vector<Candidate> candidates;

    // collect all valid rules
    for (const auto &rule : data.rules) {
        if (rule.input != symbol.name) continue;
        if (rule.params.size() != symbol.params.size()) continue;

        // map parameter values
        std::map<std::string, float> paramValues;
        for (size_t i = 0; i < rule.params.size(); i++) {
            paramValues[rule.params[i]] = symbol.params[i];
        }

        // condition check !!
        if (!evaluateCondition(rule.condition, paramValues)) continue;

        candidates.push_back({ &rule, paramValues });
    }

    // norules matched --> return symbol unchanged
    if (candidates.empty()) {
        std::string result = symbol.name;
        if (!symbol.params.empty()) {
            result += "(";
            for (size_t i = 0; i < symbol.params.size(); i++) {
                if (i > 0) result += ",";
                result += std::to_string(symbol.params[i]);
            }
            result += ")";
        }
        return result;
    }

    // compute total probability weight
    float totalWeight = 0.0f;
    for (auto &c : candidates) {
        totalWeight += c.rule->probability;
    }

    // pick random threshold in [0, totalWeight]
    float r = (float(rand()) / RAND_MAX) * totalWeight;

    // select rule by cumulative probability
    float accum = 0.0f;
    for (auto &c : candidates) {
        accum += c.rule->probability;
        if (r <= accum) {
            // apple the rule !!!
            return substituteParams(c.rule->output, c.paramValues);
        }
    }

    // safetly fallback (should never hit)
    return substituteParams(candidates.back().rule->output, candidates.back().paramValues);
}

/**
 * @brief LSystem::interpretLSystem interprets an L system based on turtle grammar !!
 * @param data the configurations / rules for the l system
 * @param symbols the symbosl that make up the rules
 * @param stemCTMs
 * @param leafCTMs
 */
void LSystem::interpretLSystem(const LSystemData &data, const std::vector<LSymbol> &symbols, std::vector<StemData> &stems,
                               std::vector<glm::mat4> &leafCTMs, std::vector<glm::mat4> &flowerCTMs){

    TState turtle;
    turtle.pos = glm::vec3(0, 0, 0);
    turtle.heading = glm::vec3(0, 1, 0);  // forward direction (Y+)
    turtle.left    = glm::vec3(1, 0, 0);  // Changed from -1 to 1
    turtle.up      = glm::vec3(0, 0, 1);  // up direction (Z+)

    std::stack<TState> stateStack;

    for (const auto &sym : symbols) {

        if (sym.name == "F") { // move forward and draw stem
            float segmentLength = sym.params.empty() ? data.step : sym.params[0];
            float thickness = (sym.params.size() > 1) ? sym.params[1] : 0.05f;

            turtle.lastThickness = thickness;

            glm::vec3 center = turtle.pos + turtle.heading * (segmentLength * 0.5f);

            glm::mat4 rot(1.0f);
            rot[0] = glm::vec4(turtle.left,   0);
            rot[1] = glm::vec4(turtle.heading, 0);
            rot[2] = glm::vec4(turtle.up,     0);



            glm::mat4 ctm = glm::translate(glm::mat4(1.0f), center) * rot;
            ctm = glm::scale(ctm, glm::vec3(thickness, segmentLength, thickness));

            stems.push_back({ctm, thickness, segmentLength});
            turtle.pos += turtle.heading * segmentLength;
        }

        else if (sym.name == "L") {
            float scaleVal = sym.params.empty() ? 1.0f : sym.params[0];

            // (leaf stem) length - pushes leaf away from branch
            float petioleLength = 0.3f * scaleVal;

            float radius = turtle.lastThickness * 0.5f;

            // random angle for leaf placement around branch
            float theta = ((rand() % 10000) / 10000.f) * 2.f * M_PI;

            glm::vec3 outward =
                glm::normalize(cos(theta) * turtle.left + sin(theta) * turtle.up);

            // offset outward from branch surface + along heading direction
            glm::vec3 leafPos = turtle.pos
                                + outward * (radius + petioleLength)
                                + turtle.heading * (petioleLength * 0.3f);  // slight forward offset

            // build leaf coordinate frame pointing outward
            glm::vec3 normal = glm::normalize(outward + turtle.heading * 0.3f);
            glm::vec3 tangent = glm::normalize(glm::cross(normal, turtle.heading));
            if (glm::length(tangent) < 0.001f) {
                tangent = turtle.left;
            }
            glm::vec3 bitangent = glm::cross(tangent, normal);

            glm::mat4 rot(1.0f);
            rot[0] = glm::vec4(tangent,   0);
            rot[1] = glm::vec4(normal,    0);
            rot[2] = glm::vec4(bitangent, 0);

            // random twist of the leaf
            float twist = ((rand() % 10000) / 10000.f) * 2.f * M_PI;
            rot = glm::rotate(rot, twist, normal);

            glm::mat4 leaf = glm::translate(glm::mat4(1.0f), leafPos);
            leaf *= rot;

            leaf = glm::scale(leaf, glm::vec3(0.5f * scaleVal, 0.12f * scaleVal, 0.3f * scaleVal));

            leafCTMs.push_back(leaf);
        } else if (sym.name == "W") {  // W for wildflower/flower
            float scaleVal = sym.params.empty() ? 0.15f : sym.params[0];

            // position flower at current turtle position
            glm::vec3 flowerPos = turtle.pos;

            // build flower orientation from turtle state
            glm::mat4 rot(1.0f);
            rot[0] = glm::vec4(turtle.left,    0);
            rot[1] = glm::vec4(turtle.heading, 0);  // flower points along heading
            rot[2] = glm::vec4(turtle.up,      0);

            glm::mat4 flower = glm::translate(glm::mat4(1.0f), flowerPos);
            flower *= rot;
            flower = glm::scale(flower, glm::vec3(scaleVal));

            flowerCTMs.push_back(flower);
        }


        else if (sym.name == "+") { // turn left (counter-clockwise around up axis)
            float angle = sym.params.empty() ? data.angle : glm::radians(sym.params[0]);
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, turtle.up);
            turtle.heading = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.heading, 0)));
            turtle.left    = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.left, 0)));
        }

        else if (sym.name == "-") { // turn right (clockwise around up axis)
            float angle = sym.params.empty() ? data.angle : glm::radians(sym.params[0]);
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), -angle, turtle.up);
            turtle.heading = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.heading, 0)));
            turtle.left    = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.left, 0)));
        }

        else if (sym.name == "&") { // pitch down (rotate heading down around left axis)
            float angle = sym.params.empty() ? data.angle : glm::radians(sym.params[0]);
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, turtle.left);
            turtle.heading = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.heading, 0)));
            turtle.up      = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.up, 0)));
        }

        else if (sym.name == "^") { // pitch up (rotate heading up around left axis)
            float angle = sym.params.empty() ? data.angle : glm::radians(sym.params[0]);
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), -angle, turtle.left);
            turtle.heading = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.heading, 0)));
            turtle.up      = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.up, 0)));
        }

        else if (sym.name == "\\") { // roll left (rotate left around heading axis)
            float angle = sym.params.empty() ? data.angle : glm::radians(sym.params[0]);
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, turtle.heading);
            turtle.left = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.left, 0)));
            turtle.up   = glm::normalize(glm::vec3(rotation * glm::vec4(turtle.up, 0)));
        }

        else if (sym.name == "/") { // roll right (rotate right around heading axis)
            float angle = sym.params.empty() ? data.angle : glm::radians(sym.params[0]);
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), -angle, turtle.heading);
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
