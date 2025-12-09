#include "scenerenderer.h"
#include "utils/shaderloader.h"
#include "utils/textureutils.h"
#include "renderers/lightrenderer.h"

#include <QImage>
#include <QString>

void SceneRenderer::initialize() {
    m_shader = ShaderLoader::createShaderProgram(":/resources/shaders/default.vert", ":/resources/shaders/default.frag");
}

void SceneRenderer::cleanup() {
    glDeleteProgram(m_shader);

    for (auto& pair : m_textureCache) {
        glDeleteTextures(1, &pair.second);
    }
    m_textureCache.clear();
}

/**
 * @brief SceneRenderer::render actually renders what's on screen ! called within the render loop. sets up uniforms
 * and draws each shape
 * @param renderData
 * @param camera
 * @param shapeRenderer
 */
void SceneRenderer::render(const RenderData& renderData, const Camera& camera, ShapeRenderer& shapeRenderer, const Shadow &shadow) {

    glm::vec3 cameraPos = camera.getPos();

    glUseProgram(m_shader);
    setupCameraUniforms(camera, cameraPos);
    setupLightUniforms(renderData.lights, renderData.globalData);
    // sends over shadow map (2D texture)
    setupShadowUniform(shadow);

    // for instance rendering, we're grouping all shapes by their type.
    std::unordered_map<PrimitiveType, std::vector<const RenderShapeData*>> groupedShapes;

    for (const auto& shape : renderData.shapes) {
        groupedShapes[shape.primitive.type].push_back(&shape); // this is simply filling the groupedShape list :)
    }

    // rendering each primitive by group instead of individually
    for (const auto& [type, shapes] : groupedShapes) {

        if (shapes.empty()) continue;

        // collecting per-instance data
        std::vector<glm::mat4> modelMatricies;
        std::vector<glm::mat4> normalMatricies;
        std::vector<glm::vec3> ambients, diffuses, speculars;
        std::vector<float> shininesses;

        // assuming all shapes have the same texture, we can use the first shape
        // for the texture info.
        const RenderShapeData* materialInfo = shapes[0];

        for (const auto* shape : shapes) {

            auto info = shape->material;

            modelMatricies.push_back(shape->ctm);
            ambients.push_back(info.cAmbient);
            diffuses.push_back(info.cDiffuse);
            speculars.push_back(info.cSpecular);
            shininesses.push_back(info.shininess);

        }

        GLPrimitiveData primitiveData = shapeRenderer.getPrimitiveData(type);
        glBindVertexArray(primitiveData.vao);

        // now we're uploading the instace data :)
        glBindBuffer(GL_ARRAY_BUFFER, primitiveData.instanceVBO);

        size_t matrixSize = modelMatricies.size() * sizeof(glm::mat4);
        size_t vec3Size = ambients.size() * sizeof(glm::vec3);
        size_t floatSize = shininesses.size() * sizeof(float);
        size_t totalSize = matrixSize + (vec3Size * 3) + floatSize;

        glBufferData(GL_ARRAY_BUFFER, totalSize, nullptr, GL_DYNAMIC_DRAW);

        size_t offset = 0;
        glBufferSubData(GL_ARRAY_BUFFER, offset, matrixSize, modelMatricies.data());
        offset += matrixSize;

        glBufferSubData(GL_ARRAY_BUFFER, offset, vec3Size, ambients.data());
        offset += vec3Size;

        glBufferSubData(GL_ARRAY_BUFFER, offset, vec3Size, diffuses.data());
        offset += vec3Size;

        glBufferSubData(GL_ARRAY_BUFFER, offset, vec3Size, speculars.data());
        offset += vec3Size;

        glBufferSubData(GL_ARRAY_BUFFER, offset, floatSize, shininesses.data());

        offset = 0;

        // passing model matrix
        for (int i = 0; i < 4; i++) {
            
            glEnableVertexAttribArray(5 + i);
            glVertexAttribPointer(5 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                                 (void*)(offset + i * sizeof(glm::vec4)));
            glVertexAttribDivisor(5 + i, 1);

        }

        offset += matrixSize;
        
        // ambient
        glEnableVertexAttribArray(9);
        glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)offset);
        glVertexAttribDivisor(9, 1);
        offset += vec3Size;

        // diffuse
        glEnableVertexAttribArray(10);
        glVertexAttribPointer(10, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)offset);
        glVertexAttribDivisor(10, 1);
        offset += vec3Size;

        // specular
        glEnableVertexAttribArray(11);
        glVertexAttribPointer(11, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)offset);
        glVertexAttribDivisor(11, 1);
        offset += vec3Size;

        // shininess
        glEnableVertexAttribArray(12);
        glVertexAttribPointer(12, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)offset);
        glVertexAttribDivisor(12, 1);
        offset += vec3Size;

        // setting up textures fo this batch
        setupTextureUniforms(materialInfo->material);

        glDrawArraysInstanced(GL_TRIANGLES, 0, primitiveData.vertexCount, shapes.size());
        glBindVertexArray(0);

    }

    
    // OLD RENDER FUNCTION
    // for (const auto& shape : renderData.shapes) {
    //     glUseProgram(m_shader);

    //     // get the shape's vao from shaperenderer
    //     GLPrimitiveData primitiveData = shapeRenderer.getPrimitiveData(shape.primitive.type);
    //     glBindVertexArray(primitiveData.vao);

    //     // set up all uniforms :P
    //     // setupCameraUniforms(camera, cameraPos);
    //     setupShapeUniforms(shape, shape.material);
    //     // setupLightUniforms(renderData.lights, renderData.globalData);
    //     setupTextureUniforms(shape.material);

    //     // drawwwwwww and unbind after it done !
    //     glDrawArrays(GL_TRIANGLES, 0, primitiveData.vertexCount);

    //     glBindVertexArray(0);
    // }

    glUseProgram(0);
}

//the felow functions are helper functions for all of the uniforms in the shaders !

void SceneRenderer::setupShadowUniform(const Shadow& shadow) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadow.depth_map);

    GLuint locMap = glGetUniformLocation(m_shader, "shadowTexture");
    glUniform1i(locMap, 0);
}

void SceneRenderer::setupCameraUniforms(const Camera& camera, glm::vec3 cameraPos) {
    //passing matrices from camera !

    glUniformMatrix4fv(glGetUniformLocation(m_shader, "viewMatrix"), 1, GL_FALSE, &camera.getViewMatrix()[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "projMatrix"), 1, GL_FALSE, &camera.getProjMatrix()[0][0]);
    glUniform3f(glGetUniformLocation(m_shader, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

}

void SceneRenderer::setupShapeUniforms(const RenderShapeData& shape, const SceneMaterial& material) {
    //filling out the material strcut !
    glUniform3fv(glGetUniformLocation(m_shader, "material.cAmbient"), 1, &material.cAmbient[0]);
    glUniform3fv(glGetUniformLocation(m_shader, "material.cDiffuse"), 1, &material.cDiffuse[0]);
    glUniform3fv(glGetUniformLocation(m_shader, "material.cSpecular"), 1, &material.cSpecular[0]);
    glUniform1f(glGetUniformLocation(m_shader, "material.shininess"), material.shininess);

    glm::mat4 modInvTrans = glm::transpose(glm::inverse(shape.ctm));
    //and the model matrix

    glUniformMatrix4fv(glGetUniformLocation(m_shader, "modelMatrix"), 1, GL_FALSE, &shape.ctm[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "modelInverseTrans"), 1, GL_FALSE, &modInvTrans[0][0]);

}

void SceneRenderer::setupLightUniforms(const std::vector<SceneLightData>& lights, SceneGlobalData globalData) {
   // glUniform1i(glGetUniformLocation(m_shader, "lightCount"), lights.size());
    glUniform1i(glGetUniformLocation(m_shader, "lightCount"), 1);
    // std::cout << "Light count: " << lights.size() << std::endl;

    for (int i = 0; i < lights.size(); i++) {
        const SceneLightData& light = lights[i];
        std::string base = "lights[" + std::to_string(i) + "].";

        //filling out an array of light strcuts
        glUniform1i(glGetUniformLocation(m_shader, (base + "type").c_str()), static_cast<int>(light.type));
        glUniform3fv(glGetUniformLocation(m_shader, (base + "color").c_str()), 1, &light.color[0]);
        glUniform3fv(glGetUniformLocation(m_shader, (base + "function").c_str()), 1, &light.function[0]);
        glUniform3fv(glGetUniformLocation(m_shader, (base + "pos").c_str()), 1, &light.pos[0]);
        glUniform3fv(glGetUniformLocation(m_shader, (base + "dir").c_str()), 1, &light.dir[0]);
        glUniform1f(glGetUniformLocation(m_shader, (base + "penumbra").c_str()), light.penumbra);
        glUniform1f(glGetUniformLocation(m_shader, (base + "angle").c_str()), light.angle);
    }

    // send light matrix (for light 0/the sun) for shadow mapping
    GLuint lightMatLoc = glGetUniformLocation(m_shader, "lightMatrix");
    glUniformMatrix4fv(lightMatLoc, 1, GL_FALSE, &lights[0].matrix[0][0]);

    //passing the gloabl coefecients for light calculations
    glUniform1f(glGetUniformLocation(m_shader, "ka"), globalData.ka);
    glUniform1f(glGetUniformLocation(m_shader, "kd"), globalData.kd);
    glUniform1f(glGetUniformLocation(m_shader, "ks"), globalData.ks);
}

void SceneRenderer::setupTextureUniforms(const SceneMaterial& material) {
    glUniform1i(glGetUniformLocation(m_shader, "textureInfo.isUsed"), material.textureMap.isUsed);
    glUniform1f(glGetUniformLocation(m_shader, "textureInfo.repeatU"), material.textureMap.repeatU);
    glUniform1f(glGetUniformLocation(m_shader, "textureInfo.repeatV"), material.textureMap.repeatV);
    glUniform1f(glGetUniformLocation(m_shader, "textureInfo.blend"), material.blend);

    glUniform1i(glGetUniformLocation(m_shader, "bumpMapInfo.isUsed"), material.bumpMap.isUsed);
    glUniform1f(glGetUniformLocation(m_shader, "bumpMapInfo.repeatU"), material.bumpMap.repeatU);
    glUniform1f(glGetUniformLocation(m_shader, "bumpMapInfo.repeatV"), material.bumpMap.repeatV);
    glUniform1f(glGetUniformLocation(m_shader, "bumpMapInfo.strength"), material.bumpMap.strength);

    // Normal map info
    glUniform1i(glGetUniformLocation(m_shader, "normalMapInfo.isUsed"), material.normalMap.isUsed);
    glUniform1f(glGetUniformLocation(m_shader, "normalMapInfo.repeatU"), material.normalMap.repeatU);
    glUniform1f(glGetUniformLocation(m_shader, "normalMapInfo.repeatV"), material.normalMap.repeatV);
    glUniform1f(glGetUniformLocation(m_shader, "normalMapInfo.strength"), material.normalMap.strength);

    // always set the sampler uniforms, even if not used so opengl doesnt get mad at me
    glUniform1i(glGetUniformLocation(m_shader, "textureSampler"), 1);
    glUniform1i(glGetUniformLocation(m_shader, "normTextureSampler"), 2);
    glUniform1i(glGetUniformLocation(m_shader, "bumpTextureSampler"), 3);

    if (material.textureMap.isUsed) {
        // load texture if not already cached and bindd
        GLuint textureID = loadTexture(material.textureMap.filename, false, 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureID);
    }


    if (material.normalMap.isUsed) {
        GLuint normTextureID = loadTexture(material.normalMap.filename, false, 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, normTextureID);
    }

    if (material.bumpMap.isUsed) {
        GLuint bumpTextureID = loadTexture(material.bumpMap.filename, true, 3);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, bumpTextureID);
    }




}

GLuint SceneRenderer::loadTexture(const std::string& filename, bool isBump, GLuint slot) {
    // check if texture is already loaded
    if (m_textureCache.find(filename) != m_textureCache.end()) {
        return m_textureCache[filename];
    }

    QImage image(QString::fromStdString(filename));
    if (image.isNull()) {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }

    if (isBump){ //if it's a bump map we have to figure out the tangent space normals and store that as a new image ot be used within our shader !
        image = TextureUtils::parseBumpMap(image);
    }

    image = image.convertToFormat(QImage::Format_RGBA8888).mirrored();

    // generate and configure texture !!!
    GLuint textureID;
    glGenTextures(1, &textureID);
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    // cacheeee
    m_textureCache[filename] = textureID;

    return textureID;
}
