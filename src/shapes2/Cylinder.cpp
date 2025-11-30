#include "Cylinder.h"

void Cylinder::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Cylinder::makeSideTile(glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight) {
    glm::vec3 normalTL = glm::normalize(glm::vec3(topLeft.x, 0.0f, topLeft.z));
    glm::vec3 normalTR = glm::normalize(glm::vec3(topRight.x, 0.0f, topRight.z));
    glm::vec3 normalBL = glm::normalize(glm::vec3(bottomLeft.x, 0.0f, bottomLeft.z));
    glm::vec3 normalBR = glm::normalize(glm::vec3(bottomRight.x, 0.0f, bottomRight.z));

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normalTL);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normalBR);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, normalBL);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normalTL);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, normalTR);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normalBR);
}

void Cylinder::makeSideSlice(float currentTheta, float nextTheta) {
    float topY = 0.5f;
    float bottomY = -0.5f;

    for (int i = 0; i < m_param1; i++) {
        float y1 = topY + (float)i / m_param1 * (bottomY - topY);
        float y2 = topY + (float)(i + 1) / m_param1 * (bottomY - topY);

        glm::vec3 topLeft(m_radius * glm::cos(currentTheta), y1, m_radius * glm::sin(currentTheta));
        glm::vec3 topRight(m_radius * glm::cos(nextTheta), y1, m_radius * glm::sin(nextTheta));
        glm::vec3 bottomLeft(m_radius * glm::cos(currentTheta), y2, m_radius * glm::sin(currentTheta));
        glm::vec3 bottomRight(m_radius * glm::cos(nextTheta), y2, m_radius * glm::sin(nextTheta));

        makeSideTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Cylinder::makeCapTile(glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight, glm::vec3 normal) {
    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal);

    insertVec3(m_vertexData, topLeft);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, bottomRight);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, normal);
}

void Cylinder::makeCapSlice(float currentTheta, float nextTheta, float y, glm::vec3 normal) {
    for (int i = 0; i < m_param1; i++) {
        float r1 = (float)i / m_param1 * m_radius;
        float r2 = (float)(i + 1) / m_param1 * m_radius;

        glm::vec3 topLeft(r1 * glm::cos(currentTheta), y, r1 * glm::sin(currentTheta));
        glm::vec3 topRight(r1 * glm::cos(nextTheta), y, r1 * glm::sin(nextTheta));
        glm::vec3 bottomLeft(r2 * glm::cos(currentTheta), y, r2 * glm::sin(currentTheta));
        glm::vec3 bottomRight(r2 * glm::cos(nextTheta), y, r2 * glm::sin(nextTheta));

        makeCapTile(topLeft, topRight, bottomLeft, bottomRight, normal);
    }
}

void Cylinder::makeCylinder() {
    for (int i = 0; i < m_param2; i++) {
        float thetaStep = glm::radians(360.0f) / m_param2;
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;

        makeSideSlice(currentTheta, nextTheta);
        makeCapSlice(currentTheta, nextTheta, 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
        makeCapSlice(currentTheta, nextTheta, -0.5f, glm::vec3(0.0f, -1.0f, 0.0f));
    }
}

void Cylinder::setVertexData() {
    makeCylinder();
}

void Cylinder::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
