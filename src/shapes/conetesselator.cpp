#include "shapetesselator.h"
#include "utils/math_utils.h"
#include <numbers>

/**
 * @brief ConeTessellator::tessellate tesselates the Cone based on teh 1st and second parameters. The slicing helper functions
 * have been pilfered from teh lab !!
 * @param p1
 * @param p2
 * @return
 */
std::vector<float> ConeTessellator::tessellate(int p1, int p2) {

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

void ConeTessellator::makeCapSlice(std::vector<float>& data, int param1, float currentTheta, float nextTheta) {
    float baseRadius = 0.5f;
    float y = -0.5f; //the base of the cone !!

    for (int i = 0; i < param1; i++) {
        float innerR = (float) i / param1 * baseRadius;
        float outerR = (float) (i + 1) / param1 * baseRadius;

        glm::vec3 innerLeft(innerR * glm::cos(currentTheta), y, innerR * glm::sin(currentTheta));
        glm::vec3 innerRight(innerR * glm::cos(nextTheta),   y, innerR * glm::sin(nextTheta));
        glm::vec3 outerLeft(outerR * glm::cos(currentTheta), y, outerR * glm::sin(currentTheta));
        glm::vec3 outerRight(outerR * glm::cos(nextTheta),   y, outerR * glm::sin(nextTheta));

        makeCapTile(data, innerLeft, innerRight, outerLeft, outerRight);
    }
}


void ConeTessellator::makeCapTile(std::vector<float>& data, glm::vec3 innerLeft, glm::vec3 innerRight, glm::vec3 outerLeft, glm::vec3 outerRight) {

    glm::vec3 normal(0.f, -1.f, 0.f);
    glm::vec2 uvInnerLeft = calcUV(innerLeft, FaceType::BOTTOM);
    glm::vec2 uvInnerRight = calcUV(innerRight, FaceType::BOTTOM);
    glm::vec2 uvOuterLeft = calcUV(outerLeft, FaceType::BOTTOM);
    glm::vec2 uvOuterRight = calcUV(outerRight, FaceType::BOTTOM);

    TangentBitangent tb1 = Utils::computeTangentBitangent(
        outerLeft, innerRight, innerLeft,
        uvOuterLeft, uvInnerRight, uvInnerLeft
        );

    // Triangle 2: bottomLeft, bottomRight, topRight
    TangentBitangent tb2 = Utils::computeTangentBitangent(
        outerLeft, outerRight, innerRight,
        uvOuterLeft, uvOuterRight, uvInnerRight
        );

    // triangle 1: outerLeft → innerRight → innerLeft
    Utils::insertVec3(data, outerLeft);
    Utils::insertVec3(data, normal);
    Utils::insertVec2(data, uvOuterLeft);
    Utils::insertVec3(data, tb1.tangent); //tangent
    Utils::insertVec3(data, tb1.bitangent); //bitangent

    Utils::insertVec3(data, innerRight);
    Utils::insertVec3(data, normal);
    Utils::insertVec2(data, uvInnerRight);
    Utils::insertVec3(data, tb1.tangent); //tangent
    Utils::insertVec3(data, tb1.bitangent); //bitangent

    Utils::insertVec3(data, innerLeft);
    Utils::insertVec3(data, normal);
    Utils::insertVec2(data, uvInnerLeft);
    Utils::insertVec3(data, tb1.tangent); //tangent
    Utils::insertVec3(data, tb1.bitangent); //bitangent

    // triangle 2: outerLeft → outerRight → innerRight
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

    Utils::insertVec3(data, innerRight);
    Utils::insertVec3(data, normal);
    Utils::insertVec2(data, uvInnerRight);
    Utils::insertVec3(data, tb2.tangent); //tangent
    Utils::insertVec3(data, tb2.bitangent); //bitangent

}

void ConeTessellator::makeSlopeTile(std::vector<float>& data, glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight) {

    glm::vec3 nTL = getSlopeNormal(topLeft, bottomLeft, bottomRight);
    glm::vec3 nTR = getSlopeNormal(topRight, bottomLeft, bottomRight);
    glm::vec3 nBL = calcNorm(bottomLeft);
    glm::vec3 nBR = calcNorm(bottomRight);

    glm::vec2 uvTopLeft     = calcUV(topLeft,     FaceType::BODY);
    glm::vec2 uvTopRight    = calcUV(topRight,    FaceType::BODY);
    glm::vec2 uvBottomLeft  = calcUV(bottomLeft,  FaceType::BODY);
    glm::vec2 uvBottomRight = calcUV(bottomRight, FaceType::BODY);

    TangentBitangent tb1 = Utils::computeTangentBitangent(
        bottomLeft, topLeft, topRight,
        uvBottomLeft, uvTopLeft, uvTopRight
        );

    // Triangle 2: bottomLeft, bottomRight, topRight
    TangentBitangent tb2 = Utils::computeTangentBitangent(
        bottomLeft, topRight, bottomRight,
        uvBottomLeft, uvTopRight, uvBottomRight
        );

    // triangle 1: bottomLeft → topLeft → topRight
    Utils::insertVec3(data, bottomLeft);
    Utils::insertVec3(data, nBL);
    Utils::insertVec2(data, uvBottomLeft);
    Utils::insertVec3(data, tb1.tangent); //tangent
    Utils::insertVec3(data, tb1.bitangent); //bitangent

    Utils::insertVec3(data, topLeft);
    Utils::insertVec3(data, nTL);
    Utils::insertVec2(data, uvTopLeft);
    Utils::insertVec3(data, tb1.tangent); //tangent
    Utils::insertVec3(data, tb1.bitangent); //bitangent


    Utils::insertVec3(data, topRight);
    Utils::insertVec3(data, nTR);
    Utils::insertVec2(data, uvTopRight);
    Utils::insertVec3(data, tb1.tangent); //tangent
    Utils::insertVec3(data, tb1.bitangent); //bitangent


    // triangle 2: bottomLeft → topRight → bottomRight
    Utils::insertVec3(data, bottomLeft);
    Utils::insertVec3(data, nBL);
    Utils::insertVec2(data, uvBottomLeft);
    Utils::insertVec3(data, tb2.tangent); //tangent
    Utils::insertVec3(data, tb2.bitangent); //bitangent


    Utils::insertVec3(data, topRight);
    Utils::insertVec3(data, nTR);
    Utils::insertVec2(data, uvTopRight);
    Utils::insertVec3(data, tb2.tangent); //tangent
    Utils::insertVec3(data, tb2.bitangent); //bitangent


    Utils::insertVec3(data, bottomRight);
    Utils::insertVec3(data, nBR);
    Utils::insertVec2(data, uvBottomRight);
    Utils::insertVec3(data, tb2.tangent); //tangent
    Utils::insertVec3(data, tb2.bitangent); //bitangent

}

void ConeTessellator::makeSlopeSlice(std::vector<float>& data, int param1, float currentTheta, float nextTheta) {
    float height = 1.f;
    float halfHeight = 0.5f;
    float baseRadius = 0.5f;

    for (int row = 0; row < param1; row++) {

        // top and bottom of this vertical band
        float yTop = -halfHeight + (row + 1) * (height / param1);
        float yBottom = -halfHeight + row * (height / param1);

        float rTop = baseRadius * (1.f - (float)(row + 1) / param1);
        float rBottom = baseRadius * (1.f - (float)row / param1);

        glm::vec3 topLeft(rTop * glm::cos(currentTheta), yTop, rTop * glm::sin(currentTheta));
        glm::vec3 topRight(rTop * glm::cos(nextTheta),   yTop, rTop * glm::sin(nextTheta));
        glm::vec3 bottomLeft(rBottom * glm::cos(currentTheta), yBottom, rBottom * glm::sin(currentTheta));
        glm::vec3 bottomRight(rBottom * glm::cos(nextTheta),   yBottom, rBottom * glm::sin(nextTheta));

        makeSlopeTile(data, topLeft, topRight, bottomLeft, bottomRight);
    }
}

void ConeTessellator::makeWedge(std::vector<float>& data, int param1, float currentTheta, float nextTheta) {

    makeCapSlice(data, param1, currentTheta, nextTheta);
    makeSlopeSlice(data, param1, currentTheta, nextTheta);
}


glm::vec3 ConeTessellator::calcNorm(glm::vec3& pt) {
    float xNorm = (2.f * pt.x);
    float yNorm = -(1.f / 4.f) * (2.f * pt.y - 1.f);
    float zNorm = (2.f * pt.z);

    glm::vec3 normal = glm::vec3(xNorm, yNorm, zNorm);

    return glm::normalize(normal);
}

glm::vec3 ConeTessellator::getSlopeNormal(glm::vec3& pt,
                               glm::vec3& edgeLeft,
                               glm::vec3& edgeRight) {
    float r = glm::length(glm::vec2(pt.x, pt.z));

    // handle the tip case --> average normals of both slope faces
    if (r < 1e-5f) {
        glm::vec3 n1 = calcNorm(edgeLeft);
        glm::vec3 n2 = calcNorm(edgeRight);
        return glm::normalize(n1 + n2);
    }

    // otherwise just use the implicit cone normal :P
    return calcNorm(pt);
}


glm::vec2 ConeTessellator::calcUV(const glm::vec3& point, FaceType faceType) {
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
        // if hit the bottom !
        float u = (point.x + 0.5f);
        float v = (point.z + 0.5f);
        return glm::vec2(u,v);
   }
}
