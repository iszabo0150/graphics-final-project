#include "textureutils.h"


/**
 * @brief TextureUtils::parseBumpMap into usable normals !!! takes the average of the normals around teh current pixel and stores
 * the normal as a color value!
 * @param bumpMap
 * @return
 */
QImage TextureUtils::parseBumpMap(const QImage& bumpMap) {

    QImage normals(bumpMap.width(), bumpMap.height(), QImage::Format_RGBA8888);

    for (int y = 0; y < bumpMap.height(); y++) {
        for (int x = 0; x < bumpMap.width(); x++) {

            //the vertex we're dealing with atm / pseudo kerneling around
            glm::vec3 centerVertex = glm::vec3(x, y, getHeight(bumpMap, x, y));
            glm::vec3 accumulatedNormal = glm::vec3(0, 0, 0);

            // calculating normals how we did in lab 7 by taking the average of the normals of the surrounding traingles
            std::vector<glm::ivec2> neighborOffsets = {
                {-1, -1}, {0, -1}, {1, -1}, {1, 0},
                {1, 1}, {0, 1}, {-1, 1}, {-1, 0}
            };

            for (int i = 0; i < 8; i++) {
                glm::ivec2 currentOffset = neighborOffsets[i];
                glm::ivec2 nextOffset = neighborOffsets[(i + 1) % 8];

                glm::vec3 currentNeighbor = glm::vec3(x + currentOffset.x,  y + currentOffset.y,
                                                      getHeight(bumpMap, x + currentOffset.x, y + currentOffset.y));

                glm::vec3 nextNeighbor = glm::vec3(x + nextOffset.x, y + nextOffset.y,
                                                   getHeight(bumpMap, x + nextOffset.x, y + nextOffset.y));

                // calc normal from triangle formed by center and two neighbors
                glm::vec3 edge1 = currentNeighbor - centerVertex;
                glm::vec3 edge2 = nextNeighbor - centerVertex;
                accumulatedNormal += glm::cross(edge1, edge2);
            }

            accumulatedNormal = glm::normalize(accumulatedNormal);

            // store as an rgb color value !!!
            QColor color(
                static_cast<int>((accumulatedNormal.x + 1.0f) * 127.5f),
                static_cast<int>((accumulatedNormal.y + 1.0f) * 127.5f),
                static_cast<int>((accumulatedNormal.z + 1.0f) * 127.5f),
                255
                );
            normals.setPixelColor(x, y, color);
        }
    }

    return normals;
}

/**
 * @brief TextureUtils::getHeight
 * @param image
 * @param x
 * @param y
 * @return
 */
float TextureUtils::getHeight(const QImage& image, int x, int y) {
    // clamp coords to image boundaries
    x = std::clamp(x, 0, image.width() - 1);
    y = std::clamp(y, 0, image.height() - 1);

    QColor pixel = image.pixelColor(x, y);

    // convert to grayscale intensity in [0,1] range
    float grayscaleIntensity = (pixel.red() + pixel.green() + pixel.blue()) / (3.0f * 255.0f);
    return grayscaleIntensity;
}
