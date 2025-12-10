#ifndef TERRAINGENERATOR_H
#define TERRAINGENERATOR_H

#include <vector>
#include "glm/glm.hpp"

class TerrainGenerator
{
    public:
        TerrainGenerator();
        ~TerrainGenerator();
        int getResolution() { return m_resolution; };
        std::vector<float> generateTerrain();

    private:
        std::vector<glm::vec2> m_randVecLookup;
        int m_resolution;
        int m_lookupSize;

        glm::vec2 sampleRandomVector(int row, int col);

        glm::vec3 getPosition(int row, int col);

        float getHeight(float x, float y);

        glm::vec3 getNormal(int row, int col);

        glm::vec3 getColor(glm::vec3 normal, glm::vec3 position);

        // Computes the intensity of Perlin noise at some point
        float computePerlin(float x, float y);
};
#endif // TERRAINGENERATOR_H
