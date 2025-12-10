#include "shapetesselator.h"
#include "utils/math_utils.h"
#include <numbers>

/**
 * @brief CylinderTessellator::tessellate tesselates the Cylinder based on teh 1st and second parameters. The slicing helper functions
 * have been pilfered from teh lab !!
 * @param p1
 * @param p2
 * @return
 */
std::vector<float> CylinderTessellator::tessellate(int p1, int p2) {
    std::vector<float> data;

    p2 = std::max(3, p2);

    float thetaStep = glm::radians(360.f / float(p2));

    for (int i = 0; i < p2; i++) {
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;

        makeWedge(data, p1, currentTheta, nextTheta);
    }

    return data;
}

void CylinderTessellator::makeWedge(std::vector<float>& data, int param1,  float currentTheta, float nextTheta) {
    // top cap
    // makeCapSlice(data, param1, true, currentTheta, nextTheta);

    // bottom cap
    // makeCapSlice(data, param1, false, currentTheta, nextTheta);

    // side surface
    makeSideSlice(data, param1, currentTheta, nextTheta);
}


void CylinderTessellator::makeCapSlice(std::vector<float>& data, int param1, bool isTop, float currentTheta, float nextTheta) {
    float baseRadius = 0.5f;
    float y = isTop ? 0.5f : -0.5f; //the top / base of the cone !!

    for (int i = 0; i < param1; i++) {
        float innerR = (float) i / param1 * baseRadius;
        float outerR = (float) (i + 1) / param1 * baseRadius;

        glm::vec3 innerLeft(innerR * glm::cos(currentTheta), y, innerR * glm::sin(currentTheta));
        glm::vec3 innerRight(innerR * glm::cos(nextTheta),   y, innerR * glm::sin(nextTheta));
        glm::vec3 outerLeft(outerR * glm::cos(currentTheta), y, outerR * glm::sin(currentTheta));
        glm::vec3 outerRight(outerR * glm::cos(nextTheta),   y, outerR * glm::sin(nextTheta));

        makeCapTile(data, isTop, innerLeft, innerRight, outerLeft, outerRight);
    }
}

void CylinderTessellator::makeCapTile(std::vector<float>& data, bool isTop, glm::vec3 innerLeft, glm::vec3 innerRight, glm::vec3 outerLeft, glm::vec3 outerRight) {

    glm::vec3 normal = isTop ? glm::vec3(0.f, 1.f, 0.f) : glm::vec3(0.0f, -1.0f, 0.0f);

    if (isTop) {

        glm::vec2 uvInnerLeft = calcUV(innerLeft, FaceType::TOP);
        glm::vec2 uvInnerRight = calcUV(innerRight, FaceType::TOP);
        glm::vec2 uvOuterLeft = calcUV(outerLeft, FaceType::TOP);
        glm::vec2 uvOuterRight = calcUV(outerRight, FaceType::TOP);

        TangentBitangent tb1 = Utils::computeTangentBitangent(
            innerLeft, innerRight, outerRight,
            uvInnerLeft, uvInnerRight, uvOuterRight
            );

        // Triangle 2: bottomLeft, bottomRight, topRight
        TangentBitangent tb2 = Utils::computeTangentBitangent(
            innerLeft, outerRight, outerLeft,
            uvInnerLeft, uvOuterRight, uvOuterLeft
            );

        // Top cap: CCW when viewed from above
        // TRIANGLE 1: innerLeft → innerRight → outerRight
        Utils::insertVec3(data, innerLeft);
        Utils::insertVec3(data, normal);
        Utils::insertVec2(data, uvInnerLeft);
        Utils::insertVec3(data, tb1.tangent); //tangent
        Utils::insertVec3(data, tb1.bitangent); //bitangent


        Utils::insertVec3(data, innerRight);
        Utils::insertVec3(data, normal);
        Utils::insertVec2(data, uvInnerRight);
        Utils::insertVec3(data, tb1.tangent); //tangent
        Utils::insertVec3(data, tb1.bitangent); //bitangent


        Utils::insertVec3(data, outerRight);
        Utils::insertVec3(data, normal);
        Utils::insertVec2(data, uvOuterRight);
        Utils::insertVec3(data, tb1.tangent); //tangent
        Utils::insertVec3(data, tb1.bitangent); //bitangent


        // TRIANGLE 2: innerLeft → outerRight → outerLeft
        Utils::insertVec3(data, innerLeft);
        Utils::insertVec3(data, normal);
        Utils::insertVec2(data, uvInnerLeft);
        Utils::insertVec3(data, tb2.tangent); //tangent
        Utils::insertVec3(data, tb2.bitangent); //bitangent


        Utils::insertVec3(data, outerRight);
        Utils::insertVec3(data, normal);
        Utils::insertVec2(data, uvOuterRight);
        Utils::insertVec3(data, tb2.tangent); //tangent
        Utils::insertVec3(data, tb2.bitangent); //bitangent


        Utils::insertVec3(data, outerLeft);
        Utils::insertVec3(data, normal);
        Utils::insertVec2(data, uvOuterLeft);
        Utils::insertVec3(data, tb2.tangent); //tangent
        Utils::insertVec3(data, tb2.bitangent); //bitangent

    } else {
        // Bottom cap: CCW when viewed from below (reversed from top)
        // TRIANGLE 1: innerLeft → outerRight → innerRight

        glm::vec2 uvInnerLeft = calcUV(innerLeft, FaceType::BOTTOM);
        glm::vec2 uvInnerRight = calcUV(innerRight, FaceType::BOTTOM);
        glm::vec2 uvOuterLeft = calcUV(outerLeft, FaceType::BOTTOM);
        glm::vec2 uvOuterRight = calcUV(outerRight, FaceType::BOTTOM);

        TangentBitangent tb1 = Utils::computeTangentBitangent(
            innerLeft, outerRight, innerRight,
            uvInnerLeft, uvOuterRight, uvInnerRight
            );

        // Triangle 2: bottomLeft, bottomRight, topRight
        TangentBitangent tb2 = Utils::computeTangentBitangent(
            innerLeft, outerLeft, outerRight,
            uvInnerLeft, uvOuterLeft, uvOuterRight
            );


        Utils::insertVec3(data, innerLeft);
        Utils::insertVec3(data, normal);
        Utils::insertVec2(data, uvInnerLeft);
        Utils::insertVec3(data, tb1.tangent); //tangent
        Utils::insertVec3(data, tb1.bitangent); //bitangent


        Utils::insertVec3(data, outerRight);
        Utils::insertVec3(data, normal);
        Utils::insertVec2(data, uvOuterRight);
        Utils::insertVec3(data, tb1.tangent); //tangent
        Utils::insertVec3(data, tb1.bitangent); //bitangent


        Utils::insertVec3(data, innerRight);
        Utils::insertVec3(data, normal);
        Utils::insertVec2(data, uvInnerRight);
        Utils::insertVec3(data, tb1.tangent); //tangent
        Utils::insertVec3(data, tb1.bitangent); //bitangent


        // TRIANGLE 2: innerLeft → outerLeft → outerRight
        Utils::insertVec3(data, innerLeft);
        Utils::insertVec3(data, normal);
        Utils::insertVec2(data, uvInnerLeft);
        Utils::insertVec3(data, tb2.tangent); //tangent
        Utils::insertVec3(data, tb2.bitangent); //bitangent


        Utils::insertVec3(data, outerLeft);
        Utils::insertVec3(data, normal);
        Utils::insertVec2(data, uvOuterLeft);
        Utils::insertVec3(data, tb2.tangent); //tangent
        Utils::insertVec3(data, tb2.bitangent); //bitangent


        Utils::insertVec3(data, outerRight);
        Utils::insertVec3(data, normal);
        Utils::insertVec2(data, uvOuterRight);
        Utils::insertVec3(data, tb2.tangent); //tangent
        Utils::insertVec3(data, tb2.bitangent); //bitangent

    }
}

void CylinderTessellator::makeSideSlice(std::vector<float>& data, int param1, float currentTheta, float nextTheta) {

    float radius = 0.5f;
    float totalHeight = 1.f; // from -0.5 to +0.5
    float heightStep = totalHeight / param1;

    for (int row = 0; row < param1; row++) {

        float yBottom = -0.5f + row * heightStep;
        float yTop    = -0.5f + (row + 1) * heightStep;

        glm::vec3 topLeft(radius * glm::cos(currentTheta), yTop, radius * glm::sin(currentTheta));
        glm::vec3 topRight(radius * glm::cos(nextTheta), yTop, radius * glm::sin(nextTheta));

        glm::vec3 bottomLeft(radius * glm::cos(currentTheta), yBottom, radius * glm::sin(currentTheta));
        glm::vec3 bottomRight(radius * glm::cos(nextTheta), yBottom, radius * glm::sin(nextTheta));

        makeSideTile(data, topLeft, topRight, bottomLeft, bottomRight);
    }
}


void CylinderTessellator::makeSideTile(std::vector<float>& data, glm::vec3 topLeftPos, glm::vec3 topRightPos,glm::vec3 bottomLeftPos, glm::vec3 bottomRightPos){
    // normals point straight outward from the vertex points horizontally (ignore y)
    glm::vec3 normalTopLeft     = glm::normalize(glm::vec3(topLeftPos.x,     0.f, topLeftPos.z));
    glm::vec3 normalTopRight    = glm::normalize(glm::vec3(topRightPos.x,    0.f, topRightPos.z));
    glm::vec3 normalBottomLeft  = glm::normalize(glm::vec3(bottomLeftPos.x,  0.f, bottomLeftPos.z));
    glm::vec3 normalBottomRight = glm::normalize(glm::vec3(bottomRightPos.x, 0.f, bottomRightPos.z));

    glm::vec2 uvTopLeft     = calcUV(topLeftPos,     FaceType::BODY);
    glm::vec2 uvTopRight    = calcUV(topRightPos,    FaceType::BODY);
    glm::vec2 uvBottomLeft  = calcUV(bottomLeftPos,  FaceType::BODY);
    glm::vec2 uvBottomRight = calcUV(bottomRightPos, FaceType::BODY);

    TangentBitangent tb1 = Utils::computeTangentBitangent(
        topLeftPos, topRightPos, bottomRightPos,
        uvTopLeft, uvTopRight, uvBottomRight
        );

    // Triangle 2: bottomLeft, bottomRight, topRight
    TangentBitangent tb2 = Utils::computeTangentBitangent(
        topLeftPos, bottomRightPos, bottomLeftPos,
        uvTopLeft, uvBottomRight, uvBottomLeft
        );


    // TRIANGLE 1: topLeft → topRight → bottomRight (CCW from outside)
    Utils::insertVec3(data, topLeftPos);
    Utils::insertVec3(data, normalTopLeft);
    Utils::insertVec2(data, uvTopLeft);
    Utils::insertVec3(data, tb1.tangent); //tangent
    Utils::insertVec3(data, tb1.bitangent); //bitangent


    Utils::insertVec3(data, topRightPos);
    Utils::insertVec3(data, normalTopRight);
    Utils::insertVec2(data, uvTopRight);
    Utils::insertVec3(data, tb1.tangent); //tangent
    Utils::insertVec3(data, tb1.bitangent); //bitangent



    Utils::insertVec3(data, bottomRightPos);
    Utils::insertVec3(data, normalBottomRight);
    Utils::insertVec2(data, uvBottomRight);
    Utils::insertVec3(data, tb1.tangent); //tangent
    Utils::insertVec3(data, tb1.bitangent); //bitangent



    // TRIANGLE 2: topLeft → bottomRight → bottomLeft (CCW from outside)
    Utils::insertVec3(data, topLeftPos);
    Utils::insertVec3(data, normalTopLeft);
    Utils::insertVec2(data, uvTopLeft);
    Utils::insertVec3(data, tb2.tangent); //tangent
    Utils::insertVec3(data, tb2.bitangent); //bitangent



    Utils::insertVec3(data, bottomRightPos);
    Utils::insertVec3(data, normalBottomRight);
    Utils::insertVec2(data, uvBottomRight);
    Utils::insertVec3(data, tb2.tangent); //tangent
    Utils::insertVec3(data, tb2.bitangent); //bitangent


    Utils::insertVec3(data, bottomLeftPos);
    Utils::insertVec3(data, normalBottomLeft);
    Utils::insertVec2(data, uvBottomLeft);
    Utils::insertVec3(data, tb2.tangent); //tangent
    Utils::insertVec3(data, tb2.bitangent); //bitangent

}


glm::vec2 CylinderTessellator::calcUV(const glm::vec3& point, FaceType faceType) {
    if (faceType == FaceType::BODY){

        float v = point.y + 0.5f;
        float theta = std::atan2(point.z, point.x);
        float u = 0.0f;

        if (theta < 0){
            u = -theta / (2 * std::numbers::pi);

        } else if (theta >= 0){
            u = 1 - (theta / (2 * std::numbers::pi));
        }

        return glm::vec2(u,v);

    } else {
        if (faceType == FaceType::TOP){
            float u = (point.x + 0.5f);
            float v = (-point.z + 0.5f);
            return glm::vec2(u,v);
        }
        else { // if hit the bottom !
            float u = (point.x + 0.5f);
            float v = (point.z + 0.5f);
            return glm::vec2(u,v);
        }
    }
}


