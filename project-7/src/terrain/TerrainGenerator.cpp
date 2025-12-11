#include "TerrainGenerator.h"
#include <cmath>
#include <algorithm>
#include <iostream>

const int TerrainGenerator::PERMUTATION[512] = {
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
    8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
    35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
    134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
    55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
    18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
    250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
    189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
    172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
    228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
    107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
    151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,
    8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,
    35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,
    134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,
    55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,
    18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,
    250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,
    189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,
    172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
    228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,
    107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
    138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

TerrainGenerator::TerrainGenerator()
    : m_gridWidth(200)
    , m_gridDepth(200)
    , m_worldWidth(50.0f)
    , m_worldDepth(50.0f)
    , m_heightScale(5.0f)
    , m_octaves(4)
    , m_persistence(0.5f)
    , m_lacunarity(2.0f)
{
}

float TerrainGenerator::fade(float t) const {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float TerrainGenerator::lerp(float t, float a, float b) const {
    return a + t * (b - a);
}

float TerrainGenerator::grad(int hash, float x, float z) const {
    int h = hash & 15;
    float u = h < 8 ? x : z;
    float v = h < 4 ? z : (h == 12 || h == 14 ? x : 0);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float TerrainGenerator::perlinNoise(float x, float z) const {
    int X = (int)std::floor(x) & 255;
    int Z = (int)std::floor(z) & 255;
    x -= std::floor(x);
    z -= std::floor(z);
    float u = fade(x);
    float v = fade(z);
    int A = PERMUTATION[X] + Z;
    int B = PERMUTATION[X + 1] + Z;
    float result = lerp(v,
        lerp(u, grad(PERMUTATION[A], x, z),
                grad(PERMUTATION[B], x - 1, z)),
        lerp(u, grad(PERMUTATION[A + 1], x, z - 1),
                grad(PERMUTATION[B + 1], x - 1, z - 1))
    );
    return result;
}

float TerrainGenerator::fractalNoise(float x, float z, int octaves,
                                      float persistence, float lacunarity) const {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; i++) {
        total += perlinNoise(x * frequency, z * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return total / maxValue;
}

float TerrainGenerator::calculateHeight(int gridX, int gridZ) const {
    float worldX = (gridX / (float)m_gridWidth) * m_worldWidth - m_worldWidth * 0.5f;
    float worldZ = (gridZ / (float)m_gridDepth) * m_worldDepth - m_worldDepth * 0.5f;
    float noiseX = worldX * 0.1f;
    float noiseZ = worldZ * 0.1f;
    float noise = fractalNoise(noiseX, noiseZ, m_octaves, m_persistence, m_lacunarity);
    float height = (noise + 1.0f) * 0.5f * m_heightScale;
    if (height > m_heightScale * 0.6f) {
        height = height + (height - m_heightScale * 0.6f) * 0.5f;
    }
    return height;
}

glm::vec3 TerrainGenerator::calculateNormal(int gridX, int gridZ) const {
    float heightL = (gridX > 0) ? m_heightField[gridX - 1][gridZ] : m_heightField[gridX][gridZ];
    float heightR = (gridX < m_gridWidth - 1) ? m_heightField[gridX + 1][gridZ] : m_heightField[gridX][gridZ];
    float heightD = (gridZ > 0) ? m_heightField[gridX][gridZ - 1] : m_heightField[gridX][gridZ];
    float heightU = (gridZ < m_gridDepth - 1) ? m_heightField[gridX][gridZ + 1] : m_heightField[gridX][gridZ];
    float spacing = m_worldWidth / m_gridWidth;
    glm::vec3 tangentX(2.0f * spacing, heightR - heightL, 0.0f);
    glm::vec3 tangentZ(0.0f, heightU - heightD, 2.0f * spacing);
    glm::vec3 normal = glm::normalize(glm::cross(tangentX, tangentZ));
    return normal;
}

glm::vec2 TerrainGenerator::calculateUV(int gridX, int gridZ) const {
    float uvScale = 3.0f;
    float u = (gridX / (float)m_gridWidth) * uvScale;
    float v = (gridZ / (float)m_gridDepth) * uvScale;
    return glm::vec2(u, v);
}

void TerrainGenerator::insertVec3(std::vector<float>& data, const glm::vec3& v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void TerrainGenerator::insertVec2(std::vector<float>& data, const glm::vec2& v) {
    data.push_back(v.x);
    data.push_back(v.y);
}

void TerrainGenerator::calculateTangentBitangent(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2,
                                                   const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2,
                                                   glm::vec3& tangent, glm::vec3& bitangent) const {
    glm::vec3 edge1 = p1 - p0;
    glm::vec3 edge2 = p2 - p0;
    glm::vec2 deltaUV1 = uv1 - uv0;
    glm::vec2 deltaUV2 = uv2 - uv0;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    tangent = glm::normalize(tangent);

    bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    bitangent = glm::normalize(bitangent);
}

void TerrainGenerator::generate(int gridWidth, int gridDepth,
                                 float worldWidth, float worldDepth,
                                 float heightScale, int octaves,
                                 float persistence, float lacunarity) {
    m_gridWidth = gridWidth;
    m_gridDepth = gridDepth;
    m_worldWidth = worldWidth;
    m_worldDepth = worldDepth;
    m_heightScale = heightScale;
    m_octaves = octaves;
    m_persistence = persistence;
    m_lacunarity = lacunarity;
    regenerate();
}

void TerrainGenerator::regenerate() {
    std::cout << "Generating bread terrain: " << m_gridWidth << "x" << m_gridDepth
              << " grid, world size " << m_worldWidth << "x" << m_worldDepth << std::endl;

    m_vertexData.clear();

    m_heightField.resize(m_gridWidth);
    for (int x = 0; x < m_gridWidth; x++) {
        m_heightField[x].resize(m_gridDepth);
        for (int z = 0; z < m_gridDepth; z++) {
            m_heightField[x][z] = calculateHeight(x, z);
        }
    }

    for (int z = 0; z < m_gridDepth - 1; z++) {
        for (int x = 0; x < m_gridWidth - 1; x++) {
            float x0 = (x / (float)m_gridWidth) * m_worldWidth - m_worldWidth * 0.5f;
            float x1 = ((x + 1) / (float)m_gridWidth) * m_worldWidth - m_worldWidth * 0.5f;
            float z0 = (z / (float)m_gridDepth) * m_worldDepth - m_worldDepth * 0.5f;
            float z1 = ((z + 1) / (float)m_gridDepth) * m_worldDepth - m_worldDepth * 0.5f;

            float h00 = m_heightField[x][z];
            float h10 = m_heightField[x + 1][z];
            float h01 = m_heightField[x][z + 1];
            float h11 = m_heightField[x + 1][z + 1];

            glm::vec3 p00(x0, h00, z0);
            glm::vec3 p10(x1, h10, z0);
            glm::vec3 p01(x0, h01, z1);
            glm::vec3 p11(x1, h11, z1);

            glm::vec3 n00 = calculateNormal(x, z);
            glm::vec3 n10 = calculateNormal(x + 1, z);
            glm::vec3 n01 = calculateNormal(x, z + 1);
            glm::vec3 n11 = calculateNormal(x + 1, z + 1);

            glm::vec2 uv00 = calculateUV(x, z);
            glm::vec2 uv10 = calculateUV(x + 1, z);
            glm::vec2 uv01 = calculateUV(x, z + 1);
            glm::vec2 uv11 = calculateUV(x + 1, z + 1);

            glm::vec3 tangent1, bitangent1;
            calculateTangentBitangent(p00, p10, p01, uv00, uv10, uv01, tangent1, bitangent1);

            glm::vec3 tangent2, bitangent2;
            calculateTangentBitangent(p10, p11, p01, uv10, uv11, uv01, tangent2, bitangent2);

            insertVec3(m_vertexData, p00);
            insertVec3(m_vertexData, n00);
            insertVec2(m_vertexData, uv00);
            insertVec3(m_vertexData, tangent1);
            insertVec3(m_vertexData, bitangent1);

            insertVec3(m_vertexData, p10);
            insertVec3(m_vertexData, n10);
            insertVec2(m_vertexData, uv10);
            insertVec3(m_vertexData, tangent1);
            insertVec3(m_vertexData, bitangent1);

            insertVec3(m_vertexData, p01);
            insertVec3(m_vertexData, n01);
            insertVec2(m_vertexData, uv01);
            insertVec3(m_vertexData, tangent1);
            insertVec3(m_vertexData, bitangent1);

            insertVec3(m_vertexData, p10);
            insertVec3(m_vertexData, n10);
            insertVec2(m_vertexData, uv10);
            insertVec3(m_vertexData, tangent2);
            insertVec3(m_vertexData, bitangent2);

            insertVec3(m_vertexData, p11);
            insertVec3(m_vertexData, n11);
            insertVec2(m_vertexData, uv11);
            insertVec3(m_vertexData, tangent2);
            insertVec3(m_vertexData, bitangent2);

            insertVec3(m_vertexData, p01);
            insertVec3(m_vertexData, n01);
            insertVec2(m_vertexData, uv01);
            insertVec3(m_vertexData, tangent2);
            insertVec3(m_vertexData, bitangent2);
        }
    }

    int triangleCount = (m_gridWidth - 1) * (m_gridDepth - 1) * 2;
    int vertexCount = m_vertexData.size() / 14;
    std::cout << "Terrain generated: " << triangleCount << " triangles, "
              << vertexCount << " vertices (14-float format)" << std::endl;
}
