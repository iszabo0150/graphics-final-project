// textureutils.h
#pragma once
#include <QImage>
#include <glm/glm.hpp>

class TextureUtils {
public:
    static QImage parseBumpMap(const QImage& bumpMap);

private:
    static float getHeight(const QImage& image, int x, int y);
};
