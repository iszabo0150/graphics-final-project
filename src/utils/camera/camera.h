#pragma once

#include <glm/glm.hpp>
#include "utils/scenedata.h"

class Camera
{
public:
    Camera();

    // initialize from scene camera + current aspect ratio
    void setFromScene(const SceneCameraData &data, float aspect);

    // update aspect ratio on window resize
    void setAspect(float aspect);

    // update near / far planes when settings change
    void setClipPlanes(float nearPlane, float farPlane);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjMatrix() const;

    glm::vec3 getEye() const { return m_eye; }

private:
    glm::vec3 m_eye;
    glm::vec3 m_look;
    glm::vec3 m_up;

    float m_heightAngle;
    float m_aspect;
    float m_near;
    float m_far;
};
