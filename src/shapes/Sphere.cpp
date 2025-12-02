#include "Sphere.h"
#include "utils/uvmapper.h"
#define _USE_MATH_DEFINES
#include <math.h>

glm::vec3 spherePoint(float phi, float theta, float r = 0.5f) {
    // phi -> latitude
    // theta -> longitude
    float x = r * sin(phi) * cos(theta);
    float y = r * cos(phi);
    float z = -r * sin(phi) * sin(theta);
    return glm::vec3(x, y, z);
}

glm::vec3 sphereTangent(float phi, float theta, float r = 0.5f) {
    float x = -r * sin(phi) * sin(theta);
    float y = 0.0f;
    float z = -r * sin(phi) * cos(theta);

    glm::vec3 t(x, y, z);
    if (glm::length(t) < 0.0001f) {
        return glm::vec3(1, 0, 0);
    }
    return glm::normalize(t);
}

glm::vec3 sphereBitangent(float phi, float theta, float r = 0.5f) {
    float x = r * cos(phi) * cos(theta);
    float y = -r * sin(phi);
    float z = -r * cos(phi) * sin(theta);

    glm::vec3 b(x, y, z);
    if (glm::length(b) < 0.0001f) {
        return glm::vec3(0, 1, 0);
    }
    return glm::normalize(b);
}

void Sphere::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Sphere::makeTile(glm::vec3 topLeft, glm::vec3 topRight,
                      glm::vec3 bottomLeft, glm::vec3 bottomRight,
                      float phi1, float phi2, float theta1, float theta2) {
    auto helper = [&](glm::vec3 p, float phi, float theta) {
        glm::vec3 n = glm::normalize(p);
        glm::vec3 tangent = sphereTangent(phi, theta);
        glm::vec3 bitangent = sphereBitangent(phi, theta);

        // recompute tangent at poles to maintain orthogonality
        if (glm::length(glm::vec3(-sin(phi) * sin(theta), 0, -sin(phi) * cos(theta))) < 0.0001f) {
            tangent = glm::normalize(glm::cross(n, bitangent));
        }

        insertVec3(m_vertexData, p);
        insertVec3(m_vertexData, n);
        insertVec2(m_vertexData, getUVCoords(PrimitiveType::PRIMITIVE_SPHERE, p));
        insertVec3(m_vertexData, tangent);
        insertVec3(m_vertexData, bitangent);
    };

    // triangle 1
    helper(topLeft, phi1, theta1);
    helper(bottomLeft, phi2, theta1);
    helper(bottomRight, phi2, theta2);

    // triangle 2
    helper(topLeft, phi1, theta1);
    helper(bottomRight, phi2, theta2);
    helper(topRight, phi1, theta2);
}

void Sphere::makeWedge(float currentTheta, float nextTheta) {
    int latDiv = std::max(2, m_param1);
    float phiStep = M_PI / latDiv;

    for (int i = 0; i < latDiv; i++) {
        float phi1 = i * phiStep;
        float phi2 = (i + 1) * phiStep;

        glm::vec3 topLeft     = spherePoint(phi1, currentTheta);
        glm::vec3 topRight    = spherePoint(phi1, nextTheta);
        glm::vec3 bottomLeft  = spherePoint(phi2, currentTheta);
        glm::vec3 bottomRight = spherePoint(phi2, nextTheta);

        makeTile(topLeft, topRight, bottomLeft, bottomRight, phi1, phi2, currentTheta, nextTheta);
    }
}

void Sphere::makeSphere() {
    int lonDiv = std::max(3, m_param2);
    float thetaStep = glm::radians(360.f / lonDiv);

    for (int j = 0; j < lonDiv; j++) {
        float currentTheta = j * thetaStep;
        float nextTheta = (j + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
}

void Sphere::setVertexData() {
    m_vertexData.clear();
    makeSphere();
}

void Sphere::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void Sphere::insertVec2(std::vector<float> &data, glm::vec2 v) {
    data.push_back(v.x);
    data.push_back(v.y);
}