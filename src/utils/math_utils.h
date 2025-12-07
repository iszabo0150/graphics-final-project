#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <glm/glm.hpp>
#include <vector>

struct TangentBitangent {
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

namespace Utils {


inline void insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

inline void insertVec2(std::vector<float> &data, glm::vec2 v) {
    data.push_back(v.x);
    data.push_back(v.y);
}
/**
 * @brief getRodMatrix computes the rotation matrix about the passed in axis about the angle theta
 * @param theta
 * @param axis
 * @return
 */
inline glm::mat3 getRodMatrix(float theta, glm::vec3 axis){

    glm::vec3 u = glm::normalize(axis);
    float x = u.x, y = u.y, z = u.z;

    float c = cos(theta);
    float s = sin(theta);
    float one_c = 1.0f - c;

    // outer product uuᵀ
    glm::mat3 outer(
        x*x, x*y, x*z,
        y*x, y*y, y*z,
        z*x, z*y, z*z
        );

    // matrix that, when multiplied by a vector, performs a cross product
    glm::mat3 K(
        0, -z,  y,
        z,  0, -x,
        -y,  x,  0
        );

    // rodrigues !!!!
    glm::mat3 R = c * glm::mat3(1.0f) + one_c * outer + s * K;


    return R;
}

/**
 * @brief computeTangentBitangent creates the tangent bitangents used in norma;/bump map calculations !
 * @param v0
 * @param v1
 * @param v2
 * @param uv0
 * @param uv1
 * @param uv2
 * @return
 */
inline TangentBitangent computeTangentBitangent( const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                                                       const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2) {
    glm::vec3 deltaPos1 = v1 - v0;
    glm::vec3 deltaPos2 = v2 - v0;

    glm::vec2 deltaUV1 = uv1 - uv0;
    glm::vec2 deltaUV2 = uv2 - uv0;

    float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
    glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
    glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

    return {tangent, bitangent};
}

}






#endif // MATH_UTILS_H
