#include "Triangle.h"

void Triangle::updateParams() {
    m_vertexData = std::vector<float>();
    setVertexData();
}

void Triangle::setVertexData() {
    glm::vec3 v1(-0.5f, 0.5f, 0.0f);
    glm::vec3 v2(-0.5f, -0.5f, 0.0f);
    glm::vec3 v3(0.5f, -0.5f, 0.0f);

    glm::vec3 edge1 = v2 - v1;
    glm::vec3 edge2 = v3 - v1;
    glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

    insertVec3(m_vertexData, v1);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, v2);
    insertVec3(m_vertexData, normal);
    insertVec3(m_vertexData, v3);
    insertVec3(m_vertexData, normal);
}

// Inserts a glm::vec3 into a vector of floats.
// This will come in handy if you want to take advantage of vectors to build your shape!
void Triangle::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}
