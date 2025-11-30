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
    auto helper = [&](glm::vec3 p) {
        glm::vec3 n = glm::normalize(p);
        insertVec3(m_vertexData, p);
        insertVec3(m_vertexData, n);
        insertVec2(m_vertexData, getUVCoords(PrimitiveType::PRIMITIVE_SPHERE, p));
    };

    helper(topLeft);
    helper(bottomLeft);
    helper(bottomRight);

    helper(topLeft);
    helper(bottomRight);
    helper(topRight);
}

void Sphere::makeWedge(float currentTheta, float nextTheta) {
    // create a single wedge of the sphere using the makeTile() function
    int latDiv = std::max(2, m_param1);
    float phiStep = M_PI / latDiv;  // π radians from top to bottom

    for (int i = 0; i < latDiv; i++) {
        float phi1 = i * phiStep;         // current latitude
        float phi2 = (i + 1) * phiStep;   // next latitude

        // compute 4 corners of this tile
        glm::vec3 topLeft     = spherePoint(phi1, currentTheta);
        glm::vec3 topRight    = spherePoint(phi1, nextTheta);
        glm::vec3 bottomLeft  = spherePoint(phi2, currentTheta);
        glm::vec3 bottomRight = spherePoint(phi2, nextTheta);

        makeTile(topLeft, topRight, bottomLeft, bottomRight);
    }
}

void Sphere::makeSphere() {
    // create a full sphere using the makeWedge() function
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