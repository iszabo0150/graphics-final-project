#include "shapetesselator.h"
#include "utils/math_utils.h"


/**
 * @brief CubeTessellator::tessellate tesselates the Cone based on teh 1st and second parameters. The slicing helper functions
 * have been pilfered from teh lab !!
 * @param p1
 * @param p2
 * @return
 */

std::vector<float> CubeTessellator::tessellate(int p1) {
    std::vector<float> data;

    // Uncomment these lines for Task 3:
    // front (+Z)
    makeFace(data, p1, glm::vec3(-0.5f,  0.5f,  0.5f),   // topLeft
             glm::vec3( 0.5f,  0.5f,  0.5f),   // topRight
             glm::vec3(-0.5f, -0.5f,  0.5f),   // bottomLeft
             glm::vec3( 0.5f, -0.5f,  0.5f),
             FaceType::FRONT);  // bottomRight

    // right (+X)
    makeFace(data, p1,glm::vec3( 0.5f,  0.5f,  0.5f),
             glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f,  0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f),
            FaceType::RIGHT);

    // back (-Z)
    makeFace(data,p1, glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
            FaceType::BACK);

    // left (-X)
    makeFace(data, p1,glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f,  0.5f),
            FaceType::LEFT);

    // top (+Y)
    makeFace(data,p1, glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3( 0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f,  0.5f),
             glm::vec3( 0.5f,  0.5f,  0.5f),
            FaceType::TOP);

    // bottom (-Y)
    makeFace(data, p1,glm::vec3(-0.5f, -0.5f,  0.5f),
             glm::vec3( 0.5f, -0.5f,  0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             glm::vec3( 0.5f, -0.5f, -0.5f),
            FaceType::BOTTOM);


    return data;
}



void CubeTessellator::makeTile(std::vector<float>& data, glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight, FaceType face) {

    // Task 2: create a tile (i.e. 2 triangles) based on 4 given points.
    glm::vec3 edge1 = bottomLeft - topLeft;
    glm::vec3 edge2 = topRight   - topLeft;
    glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

    glm::vec2 uvTopLeft = calculateUV(topLeft, face);
    glm::vec2 uvBottomLeft = calculateUV(bottomLeft, face);
    glm::vec2 uvBottomRight = calculateUV(bottomRight, face);
    glm::vec2 uvTopRight = calculateUV(topRight, face);

    // Triangle 1: topLeft, bottomLeft, topRight
    TangentBitangent tb1 = Utils::computeTangentBitangent(
        topLeft, bottomLeft, bottomRight,
        uvTopLeft, uvBottomLeft, uvBottomRight
        );

    // Triangle 2: bottomLeft, bottomRight, topRight
    TangentBitangent tb2 = Utils::computeTangentBitangent(
        topRight, topLeft, bottomRight,
        uvTopRight, uvTopLeft, uvBottomRight
        );

    Utils::insertVec3(data, topLeft); //position
    Utils::insertVec3(data, faceNormal); //normal
    Utils::insertVec2(data, uvTopLeft);
    Utils::insertVec3(data, tb1.tangent); //tangent
    Utils::insertVec3(data, tb1.bitangent); //bitangent


    Utils::insertVec3(data, bottomLeft);
    Utils::insertVec3(data, faceNormal);
    Utils::insertVec2(data, uvBottomLeft);
    Utils::insertVec3(data, tb1.tangent); //tangent
    Utils::insertVec3(data, tb1.bitangent); //bitangent


    Utils::insertVec3(data, bottomRight);
    Utils::insertVec3(data, faceNormal);
    Utils::insertVec2(data, uvBottomRight);
    Utils::insertVec3(data, tb1.tangent); //tangent
    Utils::insertVec3(data, tb1.bitangent); //bitangent


    Utils::insertVec3(data, topRight);
    Utils::insertVec3(data, faceNormal);
    Utils::insertVec2(data, uvTopRight);
    Utils::insertVec3(data, tb2.tangent); //tangent
    Utils::insertVec3(data, tb2.bitangent); //bitangent


    Utils::insertVec3(data, topLeft);
    Utils::insertVec3(data, faceNormal);
    Utils::insertVec2(data, uvTopLeft);
    Utils::insertVec3(data, tb2.tangent); //tangent
    Utils::insertVec3(data, tb2.bitangent); //bitangent


    Utils::insertVec3(data, bottomRight);
    Utils::insertVec3(data, faceNormal);
    Utils::insertVec2(data, uvBottomRight);
    Utils::insertVec3(data, tb2.tangent); //tangent
    Utils::insertVec3(data, tb2.bitangent); //bitangent

}

void CubeTessellator::makeFace(std::vector<float>& data, int param1, glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight, FaceType face) {

    for (int row = 0; row < param1; row++) {
        // how far down this row is on the face
        float topAmount = (float)row / param1;
        float bottomAmount = (float)(row + 1) / param1;

        // find points going straight down the left and right sides
        glm::vec3 leftTopEdge = (1 - topAmount) * topLeft + topAmount * bottomLeft;
        glm::vec3 leftBottomEdge = (1 - bottomAmount) * topLeft + bottomAmount * bottomLeft;
        glm::vec3 rightTopEdge = (1 - topAmount) * topRight + topAmount * bottomRight;
        glm::vec3 rightBottomEdge = (1 - bottomAmount) * topRight + bottomAmount * bottomRight;

        for (int col = 0; col < param1; col++) {
            // how far across this column is on the face
            float leftAmount = (float)col / param1;
            float rightAmount = (float)(col + 1) / param1;

            // make the four corners of the small square on the cube face
            glm::vec3 tileTopLeft     = (1 - leftAmount)  * leftTopEdge     + leftAmount  * rightTopEdge;
            glm::vec3 tileTopRight    = (1 - rightAmount) * leftTopEdge     + rightAmount * rightTopEdge;
            glm::vec3 tileBottomLeft  = (1 - leftAmount)  * leftBottomEdge  + leftAmount  * rightBottomEdge;
            glm::vec3 tileBottomRight = (1 - rightAmount) * leftBottomEdge  + rightAmount * rightBottomEdge;

            // send those four corners to make a small tile (2 triangles)
            makeTile(data, tileTopLeft, tileTopRight, tileBottomLeft, tileBottomRight, face);
        }
    }
}


glm::vec2 CubeTessellator::calculateUV(const glm::vec3& point, FaceType face) {
    float u = 0.5f;
    float v = 0.5f;

    switch (face) {
    case FaceType::FRONT:  // +Z
        u = point.x + 0.5f;
        v = point.y + 0.5f;
        break;
    case FaceType::BACK:   // -Z
        u = -point.x + 0.5f;
        v = point.y + 0.5f;
        break;
    case FaceType::RIGHT:  // +X
        u = -point.z + 0.5f;
        v = point.y + 0.5f;
        break;
    case FaceType::LEFT:   // -X
        u = point.z + 0.5f;
        v = point.y + 0.5f;
        break;
    case FaceType::TOP:    // +Y
        u = point.x + 0.5f;
        v = -point.z + 0.5f;
        break;
    case FaceType::BOTTOM: // -Y
        u = point.x + 0.5f;
        v = point.z + 0.5f;
        break;
    }

    return glm::vec2(u, v);
}
