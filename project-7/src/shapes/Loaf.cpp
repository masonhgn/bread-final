#include "Loaf.h"
#include <cmath>
#include <cstdlib>

namespace {
    // simple hash based noise for procedural bumps
    float hash(float x, float y, int seed) {
        int xi = static_cast<int>(std::floor(x * 100.0f));
        int yi = static_cast<int>(std::floor(y * 100.0f));
        int n = xi + yi * 57 + seed * 131;
        n = (n << 13) ^ n;
        return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
    }

    float noise(float x, float y, int seed) {
        int xi = static_cast<int>(std::floor(x));
        int yi = static_cast<int>(std::floor(y));
        float xf = x - xi;
        float yf = y - yi;

        float u = xf * xf * (3.0f - 2.0f * xf);
        float v = yf * yf * (3.0f - 2.0f * yf);

        float n00 = hash(xi, yi, seed);
        float n10 = hash(xi + 1, yi, seed);
        float n01 = hash(xi, yi + 1, seed);
        float n11 = hash(xi + 1, yi + 1, seed);

        float nx0 = n00 * (1.0f - u) + n10 * u;
        float nx1 = n01 * (1.0f - u) + n11 * u;

        return nx0 * (1.0f - v) + nx1 * v;
    }

    // smooth min function for blending rounded edges
    float smoothMin(float a, float b, float k) {
        float h = std::max(k - std::abs(a - b), 0.0f) / k;
        return std::min(a, b) - h * h * k * 0.25f;
    }

    // calculate scoring depth on top of loaf (diagonal slashes)
    float calculateScoreDepth(float x, float z) {
        // 2 diagonal scoring marks on top
        float score1 = std::abs((x + 0.3f) - z * 0.5f);
        float score2 = std::abs((x - 0.3f) - z * 0.5f);

        float depth1 = 0.0f;
        float depth2 = 0.0f;

        if (score1 < 0.1f) {
            float t = score1 / 0.1f;
            depth1 = (std::cos(t * M_PI) + 1.0f) * 0.5f * 0.05f;  // deeper scoring
        }

        if (score2 < 0.1f) {
            float t = score2 / 0.1f;
            depth2 = (std::cos(t * M_PI) + 1.0f) * 0.5f * 0.05f;  // deeper scoring
        }

        return depth1 + depth2;
    }
}

Loaf::Loaf() {
}

void Loaf::insertVec3(const glm::vec3& v) {
    m_vertexData.push_back(v.x);
    m_vertexData.push_back(v.y);
    m_vertexData.push_back(v.z);
}

void Loaf::insertVec2(const glm::vec2& v) {
    m_vertexData.push_back(v.x);
    m_vertexData.push_back(v.y);
}

glm::vec3 Loaf::computeNormal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    return glm::normalize(glm::cross(b - a, c - a));
}

glm::vec3 Loaf::computeTangent(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
                                const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2) {
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec2 deltaUV1 = uv1 - uv0;
    glm::vec2 deltaUV2 = uv2 - uv0;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
    glm::vec3 tangent;
    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

    return glm::normalize(tangent);
}

void Loaf::makeTile(const glm::vec3& topLeft, const glm::vec3& topRight,
                    const glm::vec3& bottomLeft, const glm::vec3& bottomRight,
                    const glm::vec2& uvTopLeft, const glm::vec2& uvTopRight,
                    const glm::vec2& uvBottomLeft, const glm::vec2& uvBottomRight) {
    // triangle 1: topLeft, bottomLeft, bottomRight
    glm::vec3 normal1 = computeNormal(topLeft, bottomLeft, bottomRight);
    glm::vec3 tangent1 = computeTangent(topLeft, bottomLeft, bottomRight, uvTopLeft, uvBottomLeft, uvBottomRight);
    glm::vec3 bitangent1 = glm::normalize(glm::cross(normal1, tangent1));

    insertVec3(topLeft);
    insertVec3(normal1);
    insertVec2(uvTopLeft);
    insertVec3(tangent1);
    insertVec3(bitangent1);

    insertVec3(bottomLeft);
    insertVec3(normal1);
    insertVec2(uvBottomLeft);
    insertVec3(tangent1);
    insertVec3(bitangent1);

    insertVec3(bottomRight);
    insertVec3(normal1);
    insertVec2(uvBottomRight);
    insertVec3(tangent1);
    insertVec3(bitangent1);

    // triangle 2: topLeft, bottomRight, topRight
    glm::vec3 normal2 = computeNormal(topLeft, bottomRight, topRight);
    glm::vec3 tangent2 = computeTangent(topLeft, bottomRight, topRight, uvTopLeft, uvBottomRight, uvTopRight);
    glm::vec3 bitangent2 = glm::normalize(glm::cross(normal2, tangent2));

    insertVec3(topLeft);
    insertVec3(normal2);
    insertVec2(uvTopLeft);
    insertVec3(tangent2);
    insertVec3(bitangent2);

    insertVec3(bottomRight);
    insertVec3(normal2);
    insertVec2(uvBottomRight);
    insertVec3(tangent2);
    insertVec3(bitangent2);

    insertVec3(topRight);
    insertVec3(normal2);
    insertVec2(uvTopRight);
    insertVec3(tangent2);
    insertVec3(bitangent2);
}

void Loaf::generate(int segments, int slices) {
    m_vertexData.clear();

    // loaf dimensions (before scaling)
    float width = 1.5f;   // x dimension
    float height = 0.6f;  // y dimension
    float depth = 1.0f;   // z dimension
    float roundRadius = 0.12f;  // edge rounding

    int xDivisions = segments;
    int yDivisions = segments / 2;
    int zDivisions = segments;

    // generate vertices in a grid pattern
    for (int y = 0; y < yDivisions; y++) {
        for (int z = 0; z < zDivisions; z++) {
            for (int x = 0; x < xDivisions; x++) {
                // calculate normalized positions
                float tx1 = (float)x / (xDivisions - 1);
                float tx2 = (float)(x + 1) / (xDivisions - 1);
                float ty1 = (float)y / (yDivisions - 1);
                float ty2 = (float)(y + 1) / (yDivisions - 1);
                float tz1 = (float)z / (zDivisions - 1);
                float tz2 = (float)(z + 1) / (zDivisions - 1);

                // convert to world space positions
                auto calcPosition = [&](float tx, float ty, float tz) -> glm::vec3 {
                    float x = (tx - 0.5f) * width;
                    float y = (ty - 0.5f) * height;
                    float z = (tz - 0.5f) * depth;

                    // calculate distance from edges for rounding
                    float edgeDistX = std::abs(std::abs(x) - width * 0.5f);
                    float edgeDistY = std::abs(std::abs(y) - height * 0.5f);
                    float edgeDistZ = std::abs(std::abs(z) - depth * 0.5f);

                    // apply rounding to edges and corners
                    glm::vec3 pos(x, y, z);

                    // round the top edges more (bread rises)
                    if (y > 0) {
                        float topRound = roundRadius * 1.5f;
                        float cornerDist = std::sqrt(edgeDistX * edgeDistX + edgeDistY * edgeDistY + edgeDistZ * edgeDistZ);

                        if (cornerDist < topRound) {
                            float blend = cornerDist / topRound;
                            float offset = (1.0f - blend) * topRound * 0.3f;
                            pos.y += offset * (1.0f - ty);  // pillow effect
                        }
                    }

                    // add heavily jagged surface bumps (crust texture) - multiple layers
                    glm::vec3 surfaceNormal = glm::normalize(pos);

                    // layer 1: very large bumps
                    float largeBumps = noise(x * 4.0f, z * 4.0f, 123) * 0.08f;
                    // layer 2: medium bumps
                    float mediumBumps = noise(x * 10.0f, z * 10.0f, 456) * 0.05f;
                    // layer 3: fine detail
                    float fineBumps = noise(x * 20.0f, z * 20.0f, 789) * 0.03f;
                    // layer 4: very fine detail
                    float microBumps = noise(x * 35.0f, z * 35.0f, 321) * 0.02f;

                    float totalBump = largeBumps + mediumBumps + fineBumps + microBumps;
                    pos += surfaceNormal * totalBump;

                    // add scoring marks on top surface
                    if (ty > 0.8f) {
                        float scoreDepth = calculateScoreDepth(x, z);
                        pos.y -= scoreDepth;
                    }

                    return pos;
                };

                // calculate 4 corners of quad
                glm::vec3 p1 = calcPosition(tx1, ty1, tz1);
                glm::vec3 p2 = calcPosition(tx2, ty1, tz1);
                glm::vec3 p3 = calcPosition(tx1, ty2, tz1);
                glm::vec3 p4 = calcPosition(tx2, ty2, tz1);
                glm::vec3 p5 = calcPosition(tx1, ty1, tz2);
                glm::vec3 p6 = calcPosition(tx2, ty1, tz2);
                glm::vec3 p7 = calcPosition(tx1, ty2, tz2);
                glm::vec3 p8 = calcPosition(tx2, ty2, tz2);

                // uv coordinates with tiling
                float uTile = 2.0f;
                float vTile = 2.0f;

                // front face (z-)
                if (z == 0) {
                    makeTile(p3, p4, p1, p2,
                            glm::vec2(tx1 * uTile, ty2 * vTile),
                            glm::vec2(tx2 * uTile, ty2 * vTile),
                            glm::vec2(tx1 * uTile, ty1 * vTile),
                            glm::vec2(tx2 * uTile, ty1 * vTile));
                }

                // back face (z+)
                if (z == zDivisions - 1) {
                    makeTile(p7, p8, p5, p6,
                            glm::vec2(tx1 * uTile, ty2 * vTile),
                            glm::vec2(tx2 * uTile, ty2 * vTile),
                            glm::vec2(tx1 * uTile, ty1 * vTile),
                            glm::vec2(tx2 * uTile, ty1 * vTile));
                }

                // left face (x-)
                if (x == 0) {
                    makeTile(p3, p7, p1, p5,
                            glm::vec2(tz1 * uTile, ty2 * vTile),
                            glm::vec2(tz2 * uTile, ty2 * vTile),
                            glm::vec2(tz1 * uTile, ty1 * vTile),
                            glm::vec2(tz2 * uTile, ty1 * vTile));
                }

                // right face (x+)
                if (x == xDivisions - 1) {
                    makeTile(p4, p8, p2, p6,
                            glm::vec2(tz1 * uTile, ty2 * vTile),
                            glm::vec2(tz2 * uTile, ty2 * vTile),
                            glm::vec2(tz1 * uTile, ty1 * vTile),
                            glm::vec2(tz2 * uTile, ty1 * vTile));
                }

                // top face (y+)
                if (y == yDivisions - 1) {
                    makeTile(p7, p8, p3, p4,
                            glm::vec2(tx1 * uTile, tz2 * vTile),
                            glm::vec2(tx2 * uTile, tz2 * vTile),
                            glm::vec2(tx1 * uTile, tz1 * vTile),
                            glm::vec2(tx2 * uTile, tz1 * vTile));
                }

                // bottom face (y-)
                if (y == 0) {
                    makeTile(p1, p2, p5, p6,
                            glm::vec2(tx1 * uTile, tz1 * vTile),
                            glm::vec2(tx2 * uTile, tz1 * vTile),
                            glm::vec2(tx1 * uTile, tz2 * vTile),
                            glm::vec2(tx2 * uTile, tz2 * vTile));
                }
            }
        }
    }
}
