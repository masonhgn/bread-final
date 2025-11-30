#pragma once

#include <glm/glm.hpp>
#include "utils/scenedata.h"

class Camera {
public:
    Camera(const SceneCameraData& cameraData, float aspectRatio, float nearPlane, float farPlane);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    void updateAspectRatio(float aspectRatio);
    void updateClippingPlanes(float nearPlane, float farPlane);
    void updateFromSceneData(const SceneCameraData& cameraData);

    void translateForward(float distance);
    void translateBackward(float distance);
    void translateLeft(float distance);
    void translateRight(float distance);
    void translateUp(float distance);
    void translateDown(float distance);

    void rotateAroundWorldY(float angle);
    void rotateAroundRightVector(float angle);

    glm::vec3 getPosition() const;
    glm::vec3 getLookDirection() const;
    glm::vec3 getUpVector() const;
    float getAspectRatio() const;
    float getHeightAngle() const;

private:
    glm::vec4 m_pos;
    glm::vec4 m_look;
    glm::vec4 m_up;
    float m_heightAngle;

    float m_aspectRatio;
    float m_nearPlane;
    float m_farPlane;

    mutable glm::mat4 m_viewMatrix;
    mutable glm::mat4 m_projectionMatrix;
    mutable bool m_viewMatrixDirty;
    mutable bool m_projectionMatrixDirty;

    void computeViewMatrix() const;
    void computeProjectionMatrix() const;
};
