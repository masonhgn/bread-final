#include "Sphere.h"

void Sphere::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight) {
    glm::vec3 normalTL = glm::normalize(topLeft);
    glm::vec3 normalTR = glm::normalize(topRight);
    glm::vec3 normalBL = glm::normalize(bottomLeft);
    glm::vec3 normalBR = glm::normalize(bottomRight);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normalTL);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, normalBL);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normalBR);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normalTL);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normalBR);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, normalTR);
}

void Sphere::makeWedge(float currentTheta, float nextTheta) {
    float radius = 0.5f;

    for (int i = 0; i < m_param1; i++) {
        float phi1 = i * glm::radians(180.f) / m_param1;
        float phi2 = (i + 1) * glm::radians(180.f) / m_param1;

        glm::vec3 topLeft(
            radius * glm::sin(phi1) * glm::cos(currentTheta),
            radius * glm::cos(phi1),
            -radius * glm::sin(phi1) * glm::sin(currentTheta)
        );

        glm::vec3 topRight(
            radius * glm::sin(phi1) * glm::cos(nextTheta),
            radius * glm::cos(phi1),
            -radius * glm::sin(phi1) * glm::sin(nextTheta)
        );

        glm::vec3 bottomLeft(
            radius * glm::sin(phi2) * glm::cos(currentTheta),
            radius * glm::cos(phi2),
            -radius * glm::sin(phi2) * glm::sin(currentTheta)
        );

        glm::vec3 bottomRight(
            radius * glm::sin(phi2) * glm::cos(nextTheta),
            radius * glm::cos(phi2),
            -radius * glm::sin(phi2) * glm::sin(nextTheta)
        );

        makeTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Sphere::makeSphere() {
    for (int i = 0; i < m_param2; i++) {
        float thetaStep = glm::radians(360.f) / m_param2;
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
}

void Sphere::setVertexData() {
    makeSphere();
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Sphere::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
