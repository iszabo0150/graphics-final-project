#include "shaperenderer.h"
#include "utils/scenedata.h"
#include "settings.h"
#include "shapes/shapetesselator.h"
#include <iostream>
#include <QCoreApplication>
#include <QImage>


/**
 * @brief ShapeRenderer::initializes the GL data for our primitives. the vaos, vbos, and vertex count for each shape
 */
void ShapeRenderer::initialize() {

    m_shapeMap[PrimitiveType::PRIMITIVE_SPHERE] = createPrimitiveGLData(PrimitiveType::PRIMITIVE_SPHERE);

    m_shapeMap[PrimitiveType::PRIMITIVE_CUBE] = createPrimitiveGLData(PrimitiveType::PRIMITIVE_CUBE);

    m_shapeMap[PrimitiveType::PRIMITIVE_CONE] = createPrimitiveGLData(PrimitiveType::PRIMITIVE_CONE);

    m_shapeMap[PrimitiveType::PRIMITIVE_CYLINDER] = createPrimitiveGLData(PrimitiveType::PRIMITIVE_CYLINDER);

    m_currentParam1 = settings.shapeParameter1;
    m_currentParam2 = settings.shapeParameter2;
}

/**
 * @brief ShapeRenderer::cleanup deletes the vaos and vbos
 */
void ShapeRenderer::cleanup() {
    for (auto& pair : m_shapeMap) {
        glDeleteVertexArrays(1, &pair.second.vao);
        glDeleteBuffers(1, &pair.second.vbo);
    }
    m_shapeMap.clear();
}

/**
 * @brief ShapeRenderer::updateTessellation deletes the old vaos/vbos and recreates the maps using updated parameters
 */
void ShapeRenderer::updateTessellation(){

    if (m_currentParam1 == settings.shapeParameter1 &&
        m_currentParam2 == settings.shapeParameter2) {
        return; // no change, skip update !
    }

    cleanup();
    initialize();
}

/**
 * @brief ShapeRenderer::createPrimitiveGLData tesselates each shape and adds the vertex data and creates vao/vbos to the shape map!
 * @param type
 * @return
 */
GLPrimitiveData ShapeRenderer::createPrimitiveGLData(PrimitiveType type){

    std::vector<float> shapeData;

    switch(type){
    case PrimitiveType::PRIMITIVE_SPHERE:
        shapeData = SphereTessellator::tessellate(settings.shapeParameter1, settings.shapeParameter2);
        break;
    case PrimitiveType::PRIMITIVE_CUBE:
        shapeData = CubeTessellator::tessellate(settings.shapeParameter1);
        break;
    case PrimitiveType::PRIMITIVE_CONE:
        shapeData = ConeTessellator::tessellate(settings.shapeParameter1, settings.shapeParameter2);
        break;
    case PrimitiveType::PRIMITIVE_CYLINDER:
        shapeData = CylinderTessellator::tessellate(settings.shapeParameter1, settings.shapeParameter2);
        break;
    default:
        break;
    }
    std::cout << "jhiiiii" << std::endl;


    GLuint shapeVBO;

    glGenBuffers(1, &shapeVBO);

    // Task 6: Bind the VBO you created here
    std::cout << "buffer genned" << std::endl;


    glBindBuffer(GL_ARRAY_BUFFER, shapeVBO);


    // Task 9: Pass the triangle vector into your VBO here

    glBufferData(GL_ARRAY_BUFFER, shapeData.size() * sizeof(GLfloat), shapeData.data(), GL_STATIC_DRAW);


    // ================== Vertex Array Objects

    // Task 11: Generate a VAO here and store it in m_vao

    GLuint shapeVAO;

    glGenVertexArrays(1,&shapeVAO);

    // Task 12: Bind the VAO you created here
    glBindVertexArray(shapeVAO);


    // Task 13: Add position and color attributes to your VAO here

    glEnableVertexAttribArray(0); //position
    glEnableVertexAttribArray(1); //normal
    glEnableVertexAttribArray(2); // uv coordinates
    glEnableVertexAttribArray(3); // tangent
    glEnableVertexAttribArray(4); // bitangent


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), reinterpret_cast<void*>(0)); //position
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat))); //normal
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), reinterpret_cast<void*>(6 * sizeof(GLfloat))); //uvs
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), reinterpret_cast<void*>(8 * sizeof(GLfloat))); //tangent
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), reinterpret_cast<void*>(11 * sizeof(GLfloat))); //bitanget


    // ================== Returning to Default State

    // Task 14: Unbind your VBO and VAO here

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    int vertexCount = shapeData.size() / 14;


    return GLPrimitiveData{shapeVAO, shapeVBO, vertexCount};
}
