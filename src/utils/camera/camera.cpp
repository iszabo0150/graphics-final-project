#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>   // for glm::lookAt only
#include <cmath>
#include "settings.h"                     // for settings.nearPlane / farPlane

Camera::Camera()
    : m_eye(0.f, 0.f, 5.f),
    m_look(0.f, 0.f, -1.f),
    m_up(0.f, 1.f, 0.f),
    m_heightAngle(glm::radians(45.f)),
    m_aspect(1.f),
    m_near(0.1f),
    m_far(100.f)
{}

// use SceneCameraData for position, look, up, FOV.
// Pull near / far from the global settings object.
void Camera::setFromScene(const SceneCameraData &data, float aspect) {
    m_eye         = glm::vec3(data.pos);   // if pos is vec4, w is dropped
    m_look        = glm::vec3(data.look);
    m_up          = glm::vec3(data.up);
    m_heightAngle = data.heightAngle;
    m_aspect      = aspect;

    m_near        = settings.nearPlane;
    m_far         = settings.farPlane;
}

void Camera::setAspect(float aspect) {
    m_aspect = aspect;
}

void Camera::setClipPlanes(float nearPlane, float farPlane) {
    m_near = nearPlane;
    m_far  = farPlane;
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_eye, m_eye + m_look, m_up);
}

// Build the perspective projection matrix by hand.
// This is equivalent to glm::perspective(m_heightAngle, m_aspect, m_near, m_far)
// but written out explicitly, as required by the handout.
glm::mat4 Camera::getProjMatrix() const {
    float tanHalfFovy = std::tan(m_heightAngle * 0.5f);

    glm::mat4 P(0.f);

    // Scale x and y
    P[0][0] = 1.f / (m_aspect * tanHalfFovy);
    P[1][1] = 1.f / tanHalfFovy;

    // Depth mapping
    P[2][2] = -(m_far + m_near) / (m_far - m_near);
    P[2][3] = -1.f;
    P[3][2] = -(2.f * m_far * m_near) / (m_far - m_near);
    // P[3][3] stays 0

    return P;
}
