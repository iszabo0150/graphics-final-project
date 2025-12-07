#ifndef SHAPETESSELATOR_H
#define SHAPETESSELATOR_H

#include <vector>
#include <glm/glm.hpp>

enum class FaceType {
    FRONT,
    BACK,
    RIGHT,
    LEFT,
    TOP,
    BOTTOM,
    BODY
};




class CubeTessellator {
public:
    static std::vector<float> tessellate(int p1);

private:
    static void makeTile(std::vector<float>& data, glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight, FaceType face);
    static void makeFace(std::vector<float>& data, int param1, glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight, FaceType face);
    static glm::vec2 calculateUV(const glm::vec3& point, FaceType face);

};

class SphereTessellator {
public:
    static std::vector<float> tessellate(int p1, int p2);

private:
    static void makeTile(std::vector<float>& data,
             glm::vec3 topLeft, glm::vec3 topRight,
             glm::vec3 bottomLeft, glm::vec3 bottomRight);
    static void makeWedge(std::vector<float>& data, int param1, float currentTheta, float nextTheta);
    static glm::vec2 calcUV(const glm::vec3& point);


};

class ConeTessellator {
public:
    static std::vector<float> tessellate(int p1, int p2);

private:
    static void makeCapSlice(std::vector<float>& data, int param1, float currentTheta, float nextTheta);
    static void makeCapTile(std::vector<float>& data, glm::vec3 innerLeft,
                     glm::vec3 innerRight,
                     glm::vec3 outerLeft,
                     glm::vec3 outerRight);
    static void makeSlopeSlice(std::vector<float>& data, int param1, float currentTheta, float nextTheta);
    static void makeSlopeTile(std::vector<float>& data, glm::vec3 topLeft, glm::vec3 topRight,glm::vec3 bottomLeft, glm::vec3 bottomRight);
    static void makeWedge(std::vector<float>& data, int param1, float currentTheta, float nextTheta);
    static glm::vec3 calcNorm(glm::vec3& pt);
    static glm::vec3 getSlopeNormal(glm::vec3& pt, glm::vec3& edgeLeft, glm::vec3& edgeRight);
    static glm::vec2 calcUV(const glm::vec3& point, FaceType faceType);

};

class CylinderTessellator {
public:
    static std::vector<float> tessellate(int p1, int p2);


private:
    static void makeWedge(std::vector<float>& data, int param1, float currentTheta, float nextTheta);

    static void makeCapSlice(std::vector<float>& data, int param1, bool isTop, float currentTheta, float nextTheta);
    static void makeCapTile(std::vector<float>& data, bool isTop, glm::vec3 innerLeft, glm::vec3 innerRight, glm::vec3 outerLeft, glm::vec3 outerRight);

    static void makeSideSlice(std::vector<float>& data, int param1,float currentTheta, float nextTheta);
    static void makeSideTile(std::vector<float>& data, glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight);
    static glm::vec2 calcUV(const glm::vec3& point, FaceType faceType);


};





#endif // SHAPETESSELATOR_H
