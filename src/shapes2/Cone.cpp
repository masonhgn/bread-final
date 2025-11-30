#include "Cone.h"

void Cone::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Cone::makeCapTile(glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight) {
    glm::vec3 normal(0.0f, -1.0f, 0.0f);

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

void Cone::makeCapSlice(float currentTheta, float nextTheta) {
    float baseY = -0.5f;
    float maxRadius = 0.5f;

    for (int i = 0; i < m_param1; i++) {
        float r1 = (float)i / m_param1 * maxRadius;
        float r2 = (float)(i + 1) / m_param1 * maxRadius;

        glm::vec3 topLeft(r1 * glm::cos(currentTheta), baseY, r1 * glm::sin(currentTheta));
        glm::vec3 topRight(r1 * glm::cos(nextTheta), baseY, r1 * glm::sin(nextTheta));
        glm::vec3 bottomLeft(r2 * glm::cos(currentTheta), baseY, r2 * glm::sin(currentTheta));
        glm::vec3 bottomRight(r2 * glm::cos(nextTheta), baseY, r2 * glm::sin(nextTheta));

        makeCapTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

glm::vec3 Cone::calcNorm(glm::vec3 pt) {
    float xNorm = (2.0f * pt.x);
    float yNorm = -(1.0f / 4.0f) * (2.0f * pt.y - 1.0f);
    float zNorm = (2.0f * pt.z);

    return glm::normalize(glm::vec3(xNorm, yNorm, zNorm));
}

void Cone::makeSlopeTile(glm::vec3 topLeft, glm::vec3 topRight, glm::vec3 bottomLeft, glm::vec3 bottomRight) {
    glm::vec3 normalTL, normalTR, normalBL, normalBR;

    // Check if radius is near zero (at the tip), not if position is at origin
    float radiusTL = glm::sqrt(topLeft.x * topLeft.x + topLeft.z * topLeft.z);
    if (radiusTL < 0.0001f) {
        glm::vec3 avgBottom = (bottomLeft + bottomRight) * 0.5f;
        normalTL = calcNorm(avgBottom);
    } else {
        normalTL = calcNorm(topLeft);
    }

    float radiusTR = glm::sqrt(topRight.x * topRight.x + topRight.z * topRight.z);
    if (radiusTR < 0.0001f) {
        glm::vec3 avgBottom = (bottomLeft + bottomRight) * 0.5f;
        normalTR = calcNorm(avgBottom);
    } else {
        normalTR = calcNorm(topRight);
    }

    normalBL = calcNorm(bottomLeft);
    normalBR = calcNorm(bottomRight);

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

void Cone::makeSlopeSlice(float currentTheta, float nextTheta) {
    float tipY = 0.5f;
    float baseY = -0.5f;
    float maxRadius = 0.5f;

    for (int i = 0; i < m_param1; i++) {
        float y1 = tipY + (float)i / m_param1 * (baseY - tipY);
        float y2 = tipY + (float)(i + 1) / m_param1 * (baseY - tipY);

        float r1 = (0.5f - y1) * maxRadius;
        float r2 = (0.5f - y2) * maxRadius;

        glm::vec3 topLeft(r1 * glm::cos(currentTheta), y1, r1 * glm::sin(currentTheta));
        glm::vec3 topRight(r1 * glm::cos(nextTheta), y1, r1 * glm::sin(nextTheta));
        glm::vec3 bottomLeft(r2 * glm::cos(currentTheta), y2, r2 * glm::sin(currentTheta));
        glm::vec3 bottomRight(r2 * glm::cos(nextTheta), y2, r2 * glm::sin(nextTheta));

        makeSlopeTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Cone::makeWedge(float currentTheta, float nextTheta) {
    makeCapSlice(currentTheta, nextTheta);
    makeSlopeSlice(currentTheta, nextTheta);
}

void Cone::setVertexData() {
    for (int i = 0; i < m_param2; i++) {
        float thetaStep = glm::radians(360.0f) / m_param2;
        float currentTheta = i * thetaStep;
        float nextTheta = (i + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
}


// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Cone::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
