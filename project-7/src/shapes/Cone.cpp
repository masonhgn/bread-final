#include "Cone.h"
#include "utils/uvmapper.h"

glm::vec3 calcNorm(glm::vec3& pt) {
    float r = sqrt(pt.x * pt.x + pt.z * pt.z);
    if (r < 0.0001f && pt.y > 0.49f) {
        return glm::vec3(0, 1, 0);
    }
    float xNorm = 2.f * pt.x;
    float yNorm = 1.f - 2.f * pt.y;
    float zNorm = 2.f * pt.z;
    float len = sqrt(xNorm * xNorm + yNorm * yNorm + zNorm * zNorm);
    if (len < 0.0001f) {
        return glm::vec3(0, 1, 0);
    }
    return glm::vec3(xNorm / len, yNorm / len, zNorm / len);
}

void Cone::makeCapTile(glm::vec3 center, glm::vec3 p1, glm::vec3 p2) {
    glm::vec3 n(0, -1, 0);
    glm::vec3 tangent = glm::vec3(1, 0, 0);
    glm::vec3 bitangent = glm::vec3(0, 0, 1);

    auto addV = [&](glm::vec3 pos) {
        insertVec3(m_vertexData, pos);
        insertVec3(m_vertexData, n);
        insertVec2(m_vertexData, getUVCoords(PrimitiveType::PRIMITIVE_CONE, pos));
        insertVec3(m_vertexData, tangent);
        insertVec3(m_vertexData, bitangent);
    };

    addV(center);
    addV(p1);
    addV(p2);
}

void Cone::makeSlopeTile(glm::vec3 p1, glm::vec3 p2,
                         glm::vec3 p3, glm::vec3 p4)
{
    auto addV = [&](glm::vec3 p) {
        glm::vec3 n = calcNorm(p);
        glm::vec3 tangent = glm::vec3(-p.z, 0, p.x);

        if (glm::length(tangent) < 0.0001f) {
            tangent = glm::vec3(1, 0, 0);
        } else {
            tangent = glm::normalize(tangent);
        }




        glm::vec3 bitangent = glm::cross(n, tangent);

        insertVec3(m_vertexData, p);
        insertVec3(m_vertexData, n);
        insertVec2(m_vertexData, getUVCoords(PrimitiveType::PRIMITIVE_CONE, p));
        insertVec3(m_vertexData, tangent);
        insertVec3(m_vertexData, bitangent);
    };

    addV(p1); addV(p3); addV(p4);
    addV(p1); addV(p4); addV(p2);
}

void Cone::makeCapSlice(float theta1, float theta2)
{
    int radialDiv = std::max(1, m_param1);
    float rMax = 0.5f;
    glm::vec3 center(0, -0.5f, 0);

    for (int i = 0; i < radialDiv; i++) {
        float r1 = (rMax / radialDiv) * i;
        float r2 = (rMax / radialDiv) * (i + 1);

        glm::vec3 p1(r1 * cos(theta1), -0.5f, r1 * sin(theta1));
        glm::vec3 p2(r1 * cos(theta2), -0.5f, r1 * sin(theta2));
        glm::vec3 p3(r2 * cos(theta2), -0.5f, r2 * sin(theta2));
        glm::vec3 p4(r2 * cos(theta1), -0.5f, r2 * sin(theta1));

        makeCapTile(center, p4, p3);
        makeCapTile(center, p1, p2);
    }
}

void Cone::makeSlopeSlice(float theta1, float theta2)
{
    int div = std::max(1, m_param1);
    float yStep = 1.0f / div;

    for (int i = 0; i < div; i++) {
        float y1 = -0.5f + i * yStep;
        float y2 = -0.5f + (i + 1) * yStep;
        float r1 = 0.5f * (1 - (y1 + 0.5f));
        float r2 = 0.5f * (1 - (y2 + 0.5f));

        glm::vec3 p1(r1 * cos(theta1), y1, r1 * sin(theta1));
        glm::vec3 p2(r1 * cos(theta2), y1, r1 * sin(theta2));
        glm::vec3 p3(r2 * cos(theta1), y2, r2 * sin(theta1));
        glm::vec3 p4(r2 * cos(theta2), y2, r2 * sin(theta2));

        makeSlopeTile(p1, p2, p3, p4);
    }
}

void Cone::makeWedge(float theta1, float theta2) {
    makeCapSlice(theta1, theta2);
    makeSlopeSlice(theta1, theta2);
}

void Cone::updateParams(int param1, int param2) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    setVertexData();
}

void Cone::setVertexData() {
    m_vertexData.clear();

    int wedges = std::max(3, m_param2);
    float thetaStep = glm::radians(360.f / wedges);

    for (int i = 0; i < wedges; i++) {
        float theta1 = i * thetaStep;
        float theta2 = (i + 1) * thetaStep;
        makeWedge(theta1, theta2);
    }
}

void Cone::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void Cone::insertVec2(std::vector<float> &data, glm::vec2 v) {
    data.push_back(v.x);
    data.push_back(v.y);
}
