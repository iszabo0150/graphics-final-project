#include "terraingenerator.h"

#include <cmath>
#include "glm/glm.hpp"

TerrainGenerator::TerrainGenerator() {
    m_resolution = 100;
    m_lookupSize = 1024;
    m_randVecLookup.reserve(m_lookupSize);
    std::srand(1230);

    for (int i = 0; i < m_lookupSize; i++) {
        m_randVecLookup.push_back(glm::vec2(std::rand() * 2.0 / RAND_MAX - 1.0,
                                            std::rand() * 2.0 / RAND_MAX - 1.0));
    };
}


TerrainGenerator::~TerrainGenerator()
{
    m_randVecLookup.clear();
}

// generateTerrain helper
void addPointToVector(glm::vec3 point, std::vector<float>& vector) {
    vector.push_back(point.x);
    vector.push_back(point.y);
    vector.push_back(point.z);
}

// Generates the geometry of the output triangle mesh
std::vector<float> TerrainGenerator::generateTerrain() {
    std::vector<float> verts;
    verts.reserve(m_resolution * m_resolution * 6);

    for(int x = 0; x < m_resolution; x++) {
        for(int y = 0; y < m_resolution; y++) {
            int x1 = x;
            int y1 = y;

            int x2 = x + 1;
            int y2 = y + 1;

            glm::vec3 p1 = getPosition(x1,y1);
            glm::vec3 p2 = getPosition(x2,y1);
            glm::vec3 p3 = getPosition(x2,y2);
            glm::vec3 p4 = getPosition(x1,y2);

            glm::vec3 n1 = getNormal(x1,y1);
            glm::vec3 n2 = getNormal(x2,y1);
            glm::vec3 n3 = getNormal(x2,y2);
            glm::vec3 n4 = getNormal(x1,y2);

            // tris 1
            // x1y1z1
            // x2y1z2
            // x2y2z3
            addPointToVector(p1, verts);
            addPointToVector(n1, verts);
            addPointToVector(getColor(n1, p1), verts);

            addPointToVector(p2, verts);
            addPointToVector(n2, verts);
            addPointToVector(getColor(n2, p2), verts);

            addPointToVector(p3, verts);
            addPointToVector(n3, verts);
            addPointToVector(getColor(n3, p3), verts);

            // tris 2
            // x1y1z1
            // x2y2z3
            // x1y2z4
            addPointToVector(p1, verts);
            addPointToVector(n1, verts);
            addPointToVector(getColor(n1, p1), verts);

            addPointToVector(p3, verts);
            addPointToVector(n3, verts);
            addPointToVector(getColor(n3, p3), verts);

            addPointToVector(p4, verts);
            addPointToVector(n4, verts);
            addPointToVector(getColor(n4, p4), verts);
        }
    }
    return verts;
}

// Samples the (infinite) random vector grid at (row, col)
glm::vec2 TerrainGenerator::sampleRandomVector(int row, int col)
{
    std::hash<int> intHash;
    int index = intHash(row * 41 + col * 43) % m_lookupSize;
    return m_randVecLookup.at(index);
}


// Takes a grid coordinate (row, col), [0, m_resolution), which describes a vertex in a plane mesh
// Returns a normalized position (x, y, z); x and y in range from [0, 1), and z is obtained from getHeight()
glm::vec3 TerrainGenerator::getPosition(int row, int col) {
    // Normalizing the planar coordinates to a unit square
    // makes scaling independent of sampling resolution.
    float x = 5.0 * row / m_resolution;
    float y = 5.0 * col / m_resolution;
    float z = getHeight(x, y);
    return glm::vec3(x,y,z);
}


// Helper for computePerlin() and, possibly, getColor()
float interpolate(float A, float B, float alpha) {
    //output = A + ease(alpha)(B - A)
    float eased = 3 * pow(alpha, 2) - 2 * pow(alpha, 3);
    return A + eased * (B - A);
}

// Takes a normalized (x, y) position, in range [0,1)
// Returns a height value, z, by sampling a noise function
float TerrainGenerator::getHeight(float x, float y) {

    // Fractal Brownian Motion (FBM) with multiple octaves for mountain terrain
    // Higher amplitudes = taller peaks, lower frequencies = larger features
    
    float z1 = 0.4 * computePerlin(x * 3, y * 3);      // large mountains
    float z2 = 0.3 * computePerlin(x * 8, y * 8);      // medium ridges
    float z3 = 0.2 * computePerlin(x * 16, y * 16);    // small rocky details
    float z4 = 0.1 * computePerlin(x * 32, y * 32);    // fine texture

    /*
     * multiply inputs by a larger number - frequency greatly increases and peaks
     * start to become indistinguishable from each other
     *
     * multiply inputs by a smaller number - appears to flatten out (depends on amplitude
     * though), seems like one singular curved plane
     */

    // Return combined octaves for mountain-like terrain (all octaves contribute)
    return z1 + z2 + z3 + z4;
}

// Computes the normal of a vertex by averaging neighbors
glm::vec3 TerrainGenerator::getNormal(int row, int col) {
    auto n0 = glm::vec2(row - 1, col - 1);
    auto n1 = glm::vec2(row , col - 1);
    auto n2 = glm::vec2(row + 1, col - 1);
    auto n3 = glm::vec2(row + 1, col);
    auto n4 = glm::vec2(row + 1, col + 1);
    auto n5 = glm::vec2(row, col + 1);
    auto n6 = glm::vec2(row - 1, col + 1);
    auto n7 = glm::vec2(row - 1, col);

    std::vector<glm::vec2> neighbors = { n0, n1, n2, n3, n4, n5, n6, n7 };

    glm::vec3 pos_v = getPosition(row, col);
    glm::vec3 normal_sum = glm::vec3(0.0f);

    for (int i = 0; i < 8; i++) {
        glm::vec2 n_i = neighbors[i];
        glm::vec2 n_i_1 = neighbors[(i + 1) % 8];

        glm::vec3 pos_n = getPosition(n_i[0], n_i[1]);
        glm::vec3 pos_n1 = getPosition(n_i_1[0], n_i_1[1]);

        glm::vec3 normal =glm::cross(pos_n - pos_v, pos_n1 - pos_v);
        normal_sum = normal_sum + normal;
    }

    return glm::normalize(normal_sum);
}

// Computes color of vertex using normal and, optionally, position
glm::vec3 TerrainGenerator::getColor(glm::vec3 normal, glm::vec3 position) {
    // Task 10: compute color as a function of the normal and position

    // Return white as placeholder
    // return glm::vec3(1,1,1);

    // attempt 1
    // if (position[2] > 0.f) {
    //     return glm::vec3(1, 1, 1);
    // }
    // return glm::vec3(0.5, 0.5, 0.5f);

    // attempt 2
    if (1.0 - glm::dot(normal, glm::vec3(0, 0, 1)) < 0.5f && position[2] > 0.f) {
        return glm::vec3(0, 0.7, 0);
    }
    return glm::vec3(0.0, 0.5, 0.0);
}

// Computes the intensity of Perlin noise at some point
float TerrainGenerator::computePerlin(float x, float y) {
    // get grid indices (as ints)
    auto topLeft = glm::vec2((int) x, (int) y);
    auto topRight = glm::vec2((int) x + 1, (int) y);
    auto botLeft = glm::vec2((int) x, (int) y + 1);
    auto botRight = glm::vec2((int) x + 1, (int) y + 1);

    // compute offset vectors
    glm::vec2 offsetTL = glm::vec2(x - (float)topLeft[0], y - (float)topLeft[1]);
    glm::vec2 offsetTR = glm::vec2(x - (float)topRight[0], y - (float)topRight[1]);
    glm::vec2 offsetBL = glm::vec2(x - (float)botLeft[0], y - (float)botLeft[1]);
    glm::vec2 offsetBR = glm::vec2(x - (float)botRight[0], y - (float)botRight[1]);

    // compute the dot product between the grid point direction vectors and its offset vectors
    float A = glm::dot(offsetTL, sampleRandomVector(topLeft[0], topLeft[1])); // dot product between top-left direction and its offset
    float B = glm::dot(offsetTR, sampleRandomVector(topRight[0], topRight[1])); // dot product between top-right direction and its offset
    float C = glm::dot(offsetBR, sampleRandomVector(botRight[0], botRight[1])); // dot product between bottom-right direction and its offset
    float D = glm::dot(offsetBL, sampleRandomVector(botLeft[0], botLeft[1])); // dot product between bottom-left direction and its offset

    float dx = (x - (float)topLeft[0]) / ( (float)topRight[0] - (float)topLeft[0]);
    float dy = (y - (float)topLeft[1]) / ( (float)botLeft[1] - (float)topLeft[1]);
    return interpolate(interpolate(A, B, dx), interpolate(D, C, dx), dy);
}
