#include <stdexcept>
#include "camera.h"
#include "utils/math_utils.h"
#include "settings.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <cmath>
#include <iostream>



/**
 * @brief The constructor !! takes in the data from the Scene Parser and stores the neccesary values and matrices as instance variables
 */
Camera::Camera(const glm::vec3 &pos, const glm::vec3 &look, const glm::vec3 &up, float heightAngle, int width, int height)
    : m_pos(pos), m_look(look), m_up(up), m_heightAngle(heightAngle), m_width(width), m_height(height),
    m_aspectRatio(static_cast<float>(width) / height){

    m_widthAngle = 2 * atan(m_aspectRatio * tan(heightAngle / 2));
    m_tiltAngle = asin(glm::normalize(m_look).y);


    m_vpWidth = 2*tan(m_widthAngle /2);
    m_vpHeight = 2 * tan(m_heightAngle /2);
    createViewMatrices(pos,look,up);
    createProjectionMatrix();
}


/**
 * @brief Camera::getCoordsCameraSpace transforms a row and col value into coordinates withint he Camera's space
 * @param row
 * @param col
 * @return vector representing a position !!
 */

glm::vec3 Camera::getCoordsCameraSpace(float worldX, float worldY) const{

    float x = m_vpWidth * (((worldX + 0.5)/m_width) - 0.5);
    float y =  m_vpHeight * (((m_height - 1 - worldY + 0.5)/m_height) - 0.5);

    return glm::vec3(x,y,-1);
}

/**
 * @brief Camera::getPos
 * @return the position of the Camera
 */
glm::vec3 Camera::getPos() const{
    return m_pos;
}

/**
 * @brief Camera::translate translates the camera in teh specieifed direction based on the deltaTime value
 * @param dir
 * @param deltaTime
 */
void Camera::translate(Direction dir, float deltaTime){

    float speed = 7.0f;
    glm::vec3 rightDir = glm::normalize(glm::cross(m_look, m_up));


    //figure out what the new camera position is!
    switch(dir){
        case Direction::UP :
            m_pos = m_pos + glm::vec3(0.0f, 1.0f, 0.0f) * speed * deltaTime;
            break;
        case Direction::DOWN:
            m_pos = m_pos + glm::vec3(0.0f, -1.0f, 0.0f) * speed * deltaTime;
            break;
        case Direction::FORWARD:
            m_pos = m_pos + m_look * speed * deltaTime;
            break;
        case Direction::BACKWARD:
            m_pos = m_pos - m_look * speed * deltaTime;
            break;
        case Direction::RIGHT:
            m_pos = m_pos + rightDir * speed * deltaTime;
            break;
        case Direction::LEFT:
            m_pos = m_pos -rightDir * speed * deltaTime;
            break;
        default:
            break;
    }

// std::cout << "Camera pos: (" << m_pos.x << ", " << m_pos.y << ", " << m_pos.z << ") " << std::endl;
    //update the view matrix :P
    createViewMatrices(m_pos, m_look, m_up);
}


/**
 * @brief Camera::rotates the camera depending on the change in x and y values of the mouse
 * @param deltaX
 * @param deltaY
 */
void Camera::rotate(float deltaX, float deltaY){
    float sensitivity = 0.005;

    // Apply horizontal rotation first (around world up axis)
    float horizontalRotation = deltaX * sensitivity;
    glm::mat3 rotationX = Utils::getRodMatrix(horizontalRotation, glm::vec3(0,1,0));

    // Apply horizontal rotation to look vector only
    m_look = rotationX * m_look;

    // Recalculate tilt angle after horizontal rotation
    m_tiltAngle = asin(glm::clamp(glm::normalize(m_look).y, -1.0f, 1.0f));

    // Now calculate vertical rotation limits
    float verticalRotation = deltaY * sensitivity;
    float newVerticalAngle = m_tiltAngle + verticalRotation;

    // Clamp to prevent flipping
    float minPitch = glm::radians(-89.0f);
    float maxPitch = glm::radians(89.0f);
    newVerticalAngle = glm::clamp(newVerticalAngle, minPitch, maxPitch);

    float actualRotationY = newVerticalAngle - m_tiltAngle;

    // Calculate right vector using world up
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 rightVector = glm::cross(m_look, worldUp);

    // Handle the case where look is parallel to world up (looking straight up/down)
    if (glm::length(rightVector) < 0.001f) {
        // Use world forward as fallback
        rightVector = glm::cross(m_look, glm::vec3(0.0f, 0.0f, 1.0f));
    }

    rightVector = glm::normalize(rightVector);

    // Apply vertical rotation around the right vector
    glm::mat3 rotationY = Utils::getRodMatrix(actualRotationY, rightVector);

    m_look = rotationY * m_look;

    // Recalculate up vector to be perpendicular to look and right
    m_up = glm::normalize(glm::cross(rightVector, m_look));

    // Update tilt angle
    m_tiltAngle = newVerticalAngle;

    // Update the view matrix
    createViewMatrices(m_pos, m_look, m_up);
}

/**
 * @brief Camera::createMatrices creates the CTM and inverse CTM of the camera based on the pos value and look and up vectors
 * @param pos
 * @param look
 * @param up
 */
void Camera::createViewMatrices(const glm::vec3 &pos, const glm::vec3 &look, const glm::vec3 &up){

    glm::vec3 w = -glm::normalize(look);
    glm::vec3 v = glm::normalize(up - (glm::dot(up,w) * w));
    glm::vec3 u = glm::cross(v,w);

    glm::vec4 rot1 = glm::vec4(u.x,
                               v.x,
                               w.x,
                               0.0f);

    glm::vec4 rot2 = glm::vec4(u.y,
                               v.y,
                               w.y,
                               0.0f);

    glm::vec4 rot3 = glm::vec4(u.z,
                               v.z,
                               w.z,
                               0.0f);

    glm::vec4 rot4 = glm::vec4(0.0f,
                               0.0f,
                               0.0f,
                               1.0f);
    glm::mat4 rotation = glm::mat4(rot1,rot2,rot3, rot4);


    glm::vec4 trans1 = glm::vec4(1.0f,
                                 0.0f,
                                 0.0f,
                                 0.0f);

    glm::vec4 trans2 = glm::vec4(0.0f,
                                 1.0f,
                                 0.0f,
                                 0.0f);

    glm::vec4 trans3 = glm::vec4(0.0f,
                                 0.0f,
                                 1.0f,
                                 0.0f);

    glm::vec4 trans4 = glm::vec4(-pos.x,
                                 -pos.y,
                                 -pos.z,
                                 1.0f);

    glm::mat4 trans = glm::mat4(trans1,trans2,trans3, trans4);


    m_viewMatrix = rotation * trans;
    m_inverseView = glm::inverse(m_viewMatrix);
}

/**
 * @brief Camera::createProjectionMatrix create s teh projection matrix for hte camera !!
 */
void Camera::createProjectionMatrix(){

    glm::mat4 remapZ = glm::mat4(1.0f,
                                 0.0f,
                                 0.0f,
                                 0.0f, 0.0f,
                                        1.0f,
                                        0.0f,
                                        0.0f,
                                            0.0f,
                                            0.0f,
                                            -2.0f,
                                            0.0f,
                                                0.0f,
                                                0.0f,
                                                -1.0f,
                                                1.0f);

    float c = -settings.nearPlane / settings.farPlane;

    glm::mat4 mPP = glm::mat4(1.0f,
                              0.0f,
                              0.0f,
                              0.0f,
                                    0.0f,
                                    1.0f,
                                    0.0f,
                                    0.0f,
                                            0.0f,
                                            0.0f,
                                            1.0f / (1.0f + c),
                                            -1.0f,
                                                                0.0f,
                                                                0.0f,
                                                                -c /(1+c),
                                                                0.0f);

    float scaleX = 1 / (settings.farPlane * std::tan(m_widthAngle / 2.0f));
    float scaleY = 1 / (settings.farPlane * std::tan(m_heightAngle / 2.0f));
    float scaleZ = 1 / settings.farPlane;



    glm::mat4 scale = glm::mat4(scaleX,
                                0.0f,
                                0.0f,
                                0.0f,
                                    0.0f,
                                    scaleY,
                                    0.0f,
                                    0.0f,
                                        0.0f,
                                        0.0f,
                                        scaleZ,
                                        0.0f,
                                                0.0f,
                                                0.0f,
                                                0.0f,
                                                1.0f);


    m_projectionMatrix = remapZ * mPP * scale;
}

