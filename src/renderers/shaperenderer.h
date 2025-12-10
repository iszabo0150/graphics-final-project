#ifndef SHAPERENDERER_H
#define SHAPERENDERER_H

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <map>

#include "utils/scenedata.h"
#include "utils/math_utils.h"

struct GLPrimitiveData {
    GLuint vao;
    GLuint vbo;
    int vertexCount;
};

class ShapeRenderer {
public:
    void initialize();
    void cleanup();
    GLPrimitiveData getPrimitiveData(PrimitiveType type) {return m_shapeMap.at(type);}
    void updateTessellation();
    void loadSkybox();

private:
    std::map<PrimitiveType, GLPrimitiveData> m_shapeMap;
    GLPrimitiveData createPrimitiveGLData(PrimitiveType type);

    int m_currentParam1 = -1;
    int m_currentParam2 = -1;
};

#endif // SHAPERENDERER_H
