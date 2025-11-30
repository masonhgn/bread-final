#include "Camera.h"
#include <cmath>

static glm::mat4 rotateAroundAxis(const glm::vec3& axis, float angle) {
    glm::vec3 a = glm::normalize(axis);
    float c = cos(angle);
    float s = sin(angle);
    float t = 1.0f - c;

    return glm::mat4(
        t*a.x*a.x + c,     t*a.x*a.y - s*a.z, t*a.x*a.z + s*a.y, 0,
        t*a.x*a.y + s*a.z, t*a.y*a.y + c,     t*a.y*a.z - s*a.x, 0,
        t*a.x*a.z - s*a.y, t*a.y*a.z + s*a.x, t*a.z*a.z + c,     0,
        0,                 0,                 0,                 1
    );
}

Camera::Camera(const SceneCameraData& cameraData, float aspectRatio, float nearPlane, float farPlane)
    : m_pos(cameraData.pos)
    , m_look(glm::normalize(cameraData.look))
    , m_up(glm::normalize(cameraData.up))
    , m_heightAngle(cameraData.heightAngle)
    , m_aspectRatio(aspectRatio)
    , m_nearPlane(nearPlane)
    , m_farPlane(farPlane)
    , m_viewMatrixDirty(true)
    , m_projectionMatrixDirty(true)
{
}

void Camera::computeViewMatrix() const {
    glm::vec3 w = -glm::normalize(glm::vec3(m_look));
    glm::vec3 upDir = glm::normalize(glm::vec3(m_up));
    glm::vec3 u = glm::normalize(glm::cross(upDir, w));
    glm::vec3 v = glm::normalize(glm::cross(w, u));


    glm::vec3 pos = glm::vec3(m_pos);

    m_viewMatrix = glm::mat4(
        u.x, v.x, w.x, 0.0f,
        u.y, v.y, w.y, 0.0f,
        u.z, v.z, w.z, 0.0f,
        -glm::dot(u, pos), -glm::dot(v, pos), -glm::dot(w, pos), 1.0f
    );

    m_viewMatrixDirty = false;
}

void Camera::computeProjectionMatrix() const {
    float tanHalfFOV = tan(m_heightAngle / 2.0f);
    float sy = 1.0f / tanHalfFOV;
    float sx = sy / m_aspectRatio;

    float n = m_nearPlane;
    float f = m_farPlane;

    m_projectionMatrix = glm::mat4(
        sx, 0, 0, 0,
        0, sy, 0, 0,
        0, 0, -(f + n) / (f - n), -1,
        0, 0, -2.0f * f * n / (f - n), 0
    );

    m_projectionMatrixDirty = false;
}

glm::mat4 Camera::getViewMatrix() const {
    if (m_viewMatrixDirty) {
        computeViewMatrix();
    }
    return m_viewMatrix;
}

glm::mat4 Camera::getProjectionMatrix() const {
    if (m_projectionMatrixDirty) {
        computeProjectionMatrix();
    }
    return m_projectionMatrix;
}

void Camera::updateAspectRatio(float aspectRatio) {
    m_aspectRatio = aspectRatio;
    m_projectionMatrixDirty = true;
}

void Camera::updateClippingPlanes(float nearPlane, float farPlane) {
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
    m_projectionMatrixDirty = true;
}

void Camera::updateFromSceneData(const SceneCameraData& cameraData) {
    m_pos = cameraData.pos;
    m_look = glm::normalize(cameraData.look);
    m_up = glm::normalize(cameraData.up);
    m_heightAngle = cameraData.heightAngle;
    m_viewMatrixDirty = true;
}

void Camera::translateForward(float distance) {
    glm::vec3 lookDir = glm::normalize(glm::vec3(m_look));
    m_pos += glm::vec4(lookDir * distance, 0.0f);
    m_viewMatrixDirty = true;
}

void Camera::translateBackward(float distance) {
    translateForward(-distance);
}

void Camera::translateLeft(float distance) {
    glm::vec3 lookDir = glm::normalize(glm::vec3(m_look));
    glm::vec3 upDir = glm::normalize(glm::vec3(m_up));
    glm::vec3 rightDir = glm::cross(lookDir, upDir);
    m_pos -= glm::vec4(rightDir * distance, 0.0f);
    m_viewMatrixDirty = true;
}

void Camera::translateRight(float distance) {
    translateLeft(-distance);
}

void Camera::translateUp(float distance) {
    m_pos += glm::vec4(0, distance, 0, 0);
    m_viewMatrixDirty = true;
}

void Camera::translateDown(float distance) {
    translateUp(-distance);
}

void Camera::rotateAroundWorldY(float angle) {
    glm::vec3 worldY(0, 1, 0);
    glm::mat4 rotation = rotateAroundAxis(worldY, angle);

    m_look = rotation * m_look;
    m_up = rotation * m_up;

    m_look = glm::vec4(glm::normalize(glm::vec3(m_look)), 0.0f);
    m_up = glm::vec4(glm::normalize(glm::vec3(m_up)), 0.0f);

    m_viewMatrixDirty = true;
}

void Camera::rotateAroundRightVector(float angle) {
    glm::vec3 lookDir = glm::normalize(glm::vec3(m_look));
    glm::vec3 upDir = glm::normalize(glm::vec3(m_up));
    glm::vec3 rightDir = glm::cross(lookDir, upDir);

    glm::mat4 rotation = rotateAroundAxis(rightDir, angle);

    m_look = rotation * m_look;
    m_up = rotation * m_up;

    m_look = glm::vec4(glm::normalize(glm::vec3(m_look)), 0.0f);
    m_up = glm::vec4(glm::normalize(glm::vec3(m_up)), 0.0f);

    m_viewMatrixDirty = true;
}

glm::vec3 Camera::getPosition() const {
    return glm::vec3(m_pos);
}

glm::vec3 Camera::getLookDirection() const {
    return glm::normalize(glm::vec3(m_look));
}

glm::vec3 Camera::getUpVector() const {
    return glm::normalize(glm::vec3(m_up));
}

float Camera::getAspectRatio() const {
    return m_aspectRatio;
}

float Camera::getHeightAngle() const {
    return m_heightAngle;
}
