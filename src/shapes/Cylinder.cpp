#include "Cylinder.h"
#include "utils/uvmapper.h"
#include <algorithm>

void Cylinder::makeCapTile(glm::vec3 center, glm::vec3 p1, glm::vec3 p2, bool isTop) {
    glm::vec3 n = isTop ? glm::vec3(0, 1, 0) : glm::vec3(0, -1, 0);
    auto addV = [&](glm::vec3 pos) {
        insertVec3(m_vertexData, pos);
        insertVec3(m_vertexData, n);
        insertVec2(m_vertexData, getUVCoords(PrimitiveType::PRIMITIVE_CYLINDER, pos));
    };
    if (isTop) {
        addV(center); addV(p1); addV(p2);
    } else {
        addV(center); addV(p2); addV(p1);
    }
}

void Cylinder::makeBodyTile(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4) {
    auto addV = [&](glm::vec3 p) {
        glm::vec3 n = glm::normalize(glm::vec3(p.x, 0, p.z));
        insertVec3(m_vertexData, p);
        insertVec3(m_vertexData, n);
        insertVec2(m_vertexData, getUVCoords(PrimitiveType::PRIMITIVE_CYLINDER, p));
    };
    addV(p1); addV(p3); addV(p4);
    addV(p1); addV(p4); addV(p2);
}

void Cylinder::makeCapSlice(float theta1, float theta2, bool isTop) {
    int radialDiv = std::max(1, m_param1);
    float rMax = 0.5f;
    float y = isTop ? 0.5f : -0.5f;
    glm::vec3 center(0, y, 0);
    
    for (int i = 0; i < radialDiv; i++) {
        float r1 = (rMax / radialDiv) * i;
        float r2 = (rMax / radialDiv) * (i + 1);
        
        glm::vec3 p1(r1 * cos(theta1), y, r1 * sin(theta1));
        glm::vec3 p2(r1 * cos(theta2), y, r1 * sin(theta2));
        glm::vec3 p3(r2 * cos(theta2), y, r2 * sin(theta2));
        glm::vec3 p4(r2 * cos(theta1), y, r2 * sin(theta1));
        
        makeCapTile(center, p4, p3, isTop);
        makeCapTile(center, p1, p2, isTop);
    }
}

void Cylinder::makeBodySlice(float theta1, float theta2) {
    int div = std::max(1, m_param1);
    float yStep = 1.0f / div;
    float r = 0.5f;
    
    for (int i = 0; i < div; i++) {
        float y1 = -0.5f + i * yStep;
        float y2 = -0.5f + (i + 1) * yStep;
        
        glm::vec3 p1(r * cos(theta1), y1, r * sin(theta1));
        glm::vec3 p2(r * cos(theta2), y1, r * sin(theta2));
        glm::vec3 p3(r * cos(theta1), y2, r * sin(theta1));
        glm::vec3 p4(r * cos(theta2), y2, r * sin(theta2));
        
        makeBodyTile(p1, p2, p3, p4);
    }
}

void Cylinder::makeWedge(float theta1, float theta2) {
    makeCapSlice(theta1, theta2, false);
    makeBodySlice(theta1, theta2);
    makeCapSlice(theta1, theta2, true);
}

void Cylinder::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Cylinder::setVertexData() {
    m_vertexData.clear();
    int wedges = std::max(3, m_param2);
    float thetaStep = glm::radians(360.f / wedges);
    
    for (int i = 0; i < wedges; i++) {
        float theta1 = i * thetaStep;
        float theta2 = (i + 1) * thetaStep;
        makeWedge(theta1, theta2);
    }
}

void Cylinder::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void Cylinder::insertVec2(std::vector<float> &data, glm::vec2 v) {
    data.push_back(v.x);
    data.push_back(v.y);
}