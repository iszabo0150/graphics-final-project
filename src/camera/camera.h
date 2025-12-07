#pragma once

#include <glm/glm.hpp>

// A class representing a virtual camera !!

enum class Direction {
    LEFT,
    RIGHT,
    UP,
    DOWN,
    FORWARD,
    BACKWARD
};

class Camera {
public:
    Camera(const glm::vec3 &pos, const glm::vec3 &look, const glm::vec3 &up, float heightAngle, int width, int height);

    void createProjectionMatrix();

    void translate(Direction dir, float deltaTime);
    void rotate(float deltaX, float deltaY);


    glm::vec3 getPos() const;
    glm::mat4 getProjMatrix() const { return m_projectionMatrix; }
    glm::mat4 getViewMatrix() const { return m_viewMatrix; }




private:
    glm::vec3 m_pos;       // Camera position in world space
    glm::vec3 m_look;      // Forward/look direction
    glm::vec3 m_up;        // Up direction
    int m_width;
    int m_height;
    float m_aspectRatio;
    float m_heightAngle;
    float m_widthAngle;
    glm::mat4 m_viewMatrix;
    glm::mat4 m_inverseView;
    glm::mat4 m_projectionMatrix;

    float m_vpWidth;
    float m_vpHeight;

    float m_tiltAngle;

    void createViewMatrices(const glm::vec3 &pos, const glm::vec3 &look, const glm::vec3 &up);

    glm::vec3 getCoordsCameraSpace(float worldX, float worldY) const;




};
