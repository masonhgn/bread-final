#pragma once

#include <vector>
#include <glm/glm.hpp>

class TerrainGenerator {
public:
    TerrainGenerator();

    void generate(int gridWidth = 200,
                  int gridDepth = 200,
                  float worldWidth = 50.0f,
                  float worldDepth = 50.0f,
                  float heightScale = 5.0f,
                  int octaves = 4,
                  float persistence = 0.5f,
                  float lacunarity = 2.0f);

    const std::vector<float>& getVertexData() const { return m_vertexData; }
    int getGridWidth() const { return m_gridWidth; }
    int getGridDepth() const { return m_gridDepth; }
    float getWorldWidth() const { return m_worldWidth; }
    float getWorldDepth() const { return m_worldDepth; }
    void regenerate();

private:
    float perlinNoise(float x, float z) const;
    float fade(float t) const;
    float lerp(float t, float a, float b) const;
    float grad(int hash, float x, float z) const;
    float fractalNoise(float x, float z, int octaves, float persistence, float lacunarity) const;
    float calculateHeight(int gridX, int gridZ) const;
    glm::vec3 calculateNormal(int gridX, int gridZ) const;
    glm::vec2 calculateUV(int gridX, int gridZ) const;
    void calculateTangentBitangent(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2,
                                     const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2,
                                     glm::vec3& tangent, glm::vec3& bitangent) const;
    void insertVec3(std::vector<float>& data, const glm::vec3& v);
    void insertVec2(std::vector<float>& data, const glm::vec2& v);

    int m_gridWidth;
    int m_gridDepth;
    float m_worldWidth;
    float m_worldDepth;
    float m_heightScale;
    int m_octaves;
    float m_persistence;
    float m_lacunarity;
    std::vector<std::vector<float>> m_heightField;
    std::vector<float> m_vertexData;

    static const int PERMUTATION[512];
};
