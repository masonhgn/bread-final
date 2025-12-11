#pragma once

#include <vector>
#include <glm/glm.hpp>

class Baguette {
public:
    Baguette();
    void generate(int segments, int slices);
    const std::vector<float>& getVertexData() const { return m_vertexData; }

private:
    void insertVec3(const glm::vec3& v);
    void insertVec2(const glm::vec2& v);
    void makeTile(const glm::vec3& topLeft, const glm::vec3& topRight,
                  const glm::vec3& bottomLeft, const glm::vec3& bottomRight,
                  const glm::vec2& uvTopLeft, const glm::vec2& uvTopRight,
                  const glm::vec2& uvBottomLeft, const glm::vec2& uvBottomRight);

    glm::vec3 computeNormal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);
    glm::vec3 computeTangent(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                             const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2);

    std::vector<float> m_vertexData;
};
