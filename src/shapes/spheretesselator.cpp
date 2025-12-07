#include "shapetesselator.h"
#include "utils/math_utils.h"
#include <vector>
#include <glm/glm.hpp>
#include <numbers>


/**
 * @brief SphereTessellator::tessellate tesselates the Sphere based on teh 1st and second parameters. The slicing helper functions
 * have been pilfered from teh lab !!
 * @param p1
 * @param p2
 * @return
 */
std::vector<float> SphereTessellator::tessellate(int p1, int p2) {
    std::vector<float> data;

    //making sure it stays 3D !!
    p1 = std::max(2, p1);
    p2 = std::max(3, p2);

    float thetaStep = glm::radians(360.f / float(p2));

    for (int i = 0; i < p2; i++) {
        float currentTheta = i * thetaStep;
        float nextTheta    = (i + 1) * thetaStep;
        makeWedge(data, p1, currentTheta, nextTheta);
    }


    return data;
}



void SphereTessellator::makeTile(std::vector<float>& data,
                                 glm::vec3 topLeft, glm::vec3 topRight,
                                 glm::vec3 bottomLeft, glm::vec3 bottomRight) {
    // Task 5: Implement the makeTile() function for a Sphere
    // Note: this function is very similar to the makeTile() function for Cube,
    //       but the normals are calculated in a different way!
    glm::vec2 uvTopLeft = calcUV(topLeft);
    glm::vec2 uvTopRight = calcUV(topRight);
    glm::vec2 uvBottomLeft = calcUV(bottomLeft);
    glm::vec2 uvBottomRight = calcUV(bottomRight);

    // Triangle 1: topLeft, bottomLeft, topRight
    TangentBitangent tb1 = Utils::computeTangentBitangent(
        topLeft, bottomLeft, topRight,
        uvTopLeft, uvBottomLeft, uvTopRight
        );

    // Triangle 2: bottomLeft, bottomRight, topRight
    TangentBitangent tb2 = Utils::computeTangentBitangent(
        bottomLeft, bottomRight, topRight,
        uvBottomLeft, uvBottomRight, uvTopRight
        );


    //triangel 1

    Utils::insertVec3(data, topLeft);           // position
    Utils::insertVec3(data, glm::normalize(topLeft)); // normal
    Utils::insertVec2(data, uvTopLeft);  // uv
    Utils::insertVec3(data, tb1.tangent); //tangent
    Utils::insertVec3(data, tb1.bitangent); //bitangent


    Utils::insertVec3(data, bottomLeft);
    Utils::insertVec3(data, glm::normalize(bottomLeft));
    Utils::insertVec2(data, uvBottomLeft);
    Utils::insertVec3(data, tb1.tangent); //tangent
    Utils::insertVec3(data, tb1.bitangent); //bitangent


    Utils::insertVec3(data, topRight);
    Utils::insertVec3(data, glm::normalize(topRight));
    Utils::insertVec2(data, uvTopRight);
    Utils::insertVec3(data, tb1.tangent); //tangent
    Utils::insertVec3(data, tb1.bitangent); //bitangent

    //triangle 2

    Utils::insertVec3(data, bottomLeft);
    Utils::insertVec3(data, glm::normalize(bottomLeft));
    Utils::insertVec2(data, uvBottomLeft);  // uv
    Utils::insertVec3(data, tb2.tangent); //tangent
    Utils::insertVec3(data, tb2.bitangent); //bitangent

    Utils::insertVec3(data, bottomRight);
    Utils::insertVec3(data, glm::normalize(bottomRight));
    Utils::insertVec2(data, uvBottomRight);  // uv
    Utils::insertVec3(data, tb2.tangent); //tangent
    Utils::insertVec3(data, tb2.bitangent); //bitangent

    Utils::insertVec3(data, topRight);
    Utils::insertVec3(data, glm::normalize(topRight));
    Utils::insertVec2(data, uvTopRight);  // uv
    Utils::insertVec3(data, tb2.tangent); //tangent
    Utils::insertVec3(data, tb2.bitangent); //bitangent

}


glm::vec2 SphereTessellator::calcUV(const glm::vec3& point){

    float phi = std::asin(point.y / 0.5f );
    float v = (phi/std::numbers::pi) + 0.5f;
    float u =0.0f;

    if (v== 0 || v== 1){
        u = 0.5f;
    } else {
        float theta = std::atan2(point.z, point.x);
        if (theta < 0){
            u = -theta / (2 * std::numbers::pi);

        } else if (theta >= 0){
            u = 1 - (theta / (2 * std::numbers::pi));
        }

    }
    return glm::vec2(u,v);
}




void SphereTessellator::makeWedge( std::vector<float>& data, int param1, float currentTheta, float nextTheta) {
    // Task 6: create a single wedge of the sphere using the
    //         makeTile() function you implemented in Task 5
    // Note: think about how param 1 comes into play here!


    float radius = 0.5f;

    for (int row = 0; row < param1; row++) {

        float phiTop =  row * (M_PI / param1);
        float phiBottom = (row + 1) * (M_PI / param1);

        glm::vec3 topLeft(
            radius * glm::sin(phiTop) * glm::cos(currentTheta),
            radius * glm::cos(phiTop),
            -radius * glm::sin(phiTop) * glm::sin(currentTheta)
            );

        glm::vec3 topRight(
            radius * glm::sin(phiTop) * glm::cos(nextTheta),
            radius * glm::cos(phiTop),
            -radius * glm::sin(phiTop) * glm::sin(nextTheta)
            );

        glm::vec3 bottomLeft(
            radius * glm::sin(phiBottom) * glm::cos(currentTheta),
            radius * glm::cos(phiBottom),
            -radius * glm::sin(phiBottom) * glm::sin(currentTheta)
            );

        glm::vec3 bottomRight(
            radius * glm::sin(phiBottom) * glm::cos(nextTheta),
            radius * glm::cos(phiBottom),
            -radius * glm::sin(phiBottom) * glm::sin(nextTheta)
            );

        makeTile(data, topLeft, topRight, bottomLeft, bottomRight);
    }


}
