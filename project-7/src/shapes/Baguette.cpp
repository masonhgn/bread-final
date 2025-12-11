#include "Baguette.h"
#include <cmath>
#include <cstdlib>

namespace {
    // simple hash-based noise for procedural bumps
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

    // calculate slash depth at a point (diagonal grooves on top of baguette)
    float calculateSlashDepth(float theta, float z) {
        // 3 diagonal slashes along the top of the baguette WHY NOT WORKING
        float slashPositions[] = {-0.5f, 0.0f, 0.5f};

        //only apply slashes on the top  of e baguette
        float topTheta = theta;
        if (topTheta > (float)M_PI) topTheta -= 2.0f * (float)M_PI;

        // slashes only on top hemisphere (roughly -PI/2 to PI/2)
        if (std::abs(topTheta) > (float)M_PI / 3.0f) return 0.0f;

        float totalDepth = 0.0f;

        for (int i = 0; i < 3; i++) {
            float slashCenter = slashPositions[i];

            // diagonal slash pattern
            float diagonalOffset = topTheta * 0.8f;
            float distFromSlash = std::abs(z - slashCenter - diagonalOffset);

            // slash width and depth
            float slashWidth = 0.12f;
            float maxDepth = 0.035f;

            if (distFromSlash < slashWidth) {
                // smooth falloff using cosine
                float t = distFromSlash / slashWidth;
                float falloff = (std::cos(t * (float)M_PI) + 1.0f) * 0.5f;
                totalDepth += maxDepth * falloff;
            }
        }

        return totalDepth;
    }
}

Baguette::Baguette() {
}

void Baguette::insertVec3(const glm::vec3& v) {
    m_vertexData.push_back(v.x);
    m_vertexData.push_back(v.y);
    m_vertexData.push_back(v.z);
}

void Baguette::insertVec2(const glm::vec2& v) {
    m_vertexData.push_back(v.x);
    m_vertexData.push_back(v.y);
}

glm::vec3 Baguette::computeNormal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    return glm::normalize(glm::cross(b - a, c - a));
}

glm::vec3 Baguette::computeTangent(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
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

void Baguette::makeTile(const glm::vec3& topLeft, const glm::vec3& topRight,
                        const glm::vec3& bottomLeft, const glm::vec3& bottomRight,
                        const glm::vec2& uvTopLeft, const glm::vec2& uvTopRight,
                        const glm::vec2& uvBottomLeft, const glm::vec2& uvBottomRight) {
    // triangle 1 topLeft, bottomLeft, bottomRight
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

void Baguette::generate(int segments, int slices) {
    m_vertexData.clear();

    float radius = 0.15f;  // constant radius for capsule
    float cylinderLength = 1.6f;  // length of cylindrical body

    // generate cylindrical body with surface detail
    int cylinderSegments = segments / 2;
    for (int seg = 0; seg < cylinderSegments; seg++) {
        float t1 = (float)seg / cylinderSegments;
        float t2 = (float)(seg + 1) / cylinderSegments;

        float z1 = -cylinderLength/2.0f + t1 * cylinderLength;
        float z2 = -cylinderLength/2.0f + t2 * cylinderLength;

        for (int sl = 0; sl < slices; sl++) {
            float theta1 = (float)sl / slices * 2.0f * (float)M_PI;
            float theta2 = (float)(sl + 1) / slices * 2.0f * (float)M_PI;

            // base positions
            glm::vec3 p1(radius * std::cos(theta1), radius * std::sin(theta1), z1);
            glm::vec3 p2(radius * std::cos(theta2), radius * std::sin(theta2), z1);
            glm::vec3 p3(radius * std::cos(theta1), radius * std::sin(theta1), z2);
            glm::vec3 p4(radius * std::cos(theta2), radius * std::sin(theta2), z2);

            // add surface bumps (crust texture)
            glm::vec3 normal1 = glm::normalize(glm::vec3(std::cos(theta1), std::sin(theta1), 0.0f));
            glm::vec3 normal2 = glm::normalize(glm::vec3(std::cos(theta2), std::sin(theta2), 0.0f));

            float bump1 = noise(theta1 * 8.0f, z1 * 5.0f, 42) * 0.015f;
            float bump2 = noise(theta2 * 8.0f, z1 * 5.0f, 42) * 0.015f;
            float bump3 = noise(theta1 * 8.0f, z2 * 5.0f, 42) * 0.015f;
            float bump4 = noise(theta2 * 8.0f, z2 * 5.0f, 42) * 0.015f;

            // add diagonal slash grooves
            float slash1 = calculateSlashDepth(theta1, z1);
            float slash2 = calculateSlashDepth(theta2, z1);
            float slash3 = calculateSlashDepth(theta1, z2);
            float slash4 = calculateSlashDepth(theta2, z2);

            // apply bumps and slashes to positions
            p1 += normal1 * (bump1 - slash1);
            p2 += normal2 * (bump2 - slash2);
            p3 += normal1 * (bump3 - slash3);
            p4 += normal2 * (bump4 - slash4);

            // tile UVs for more detail (repeat texture 4x along length, 2x around)
            float uTile = 4.0f;
            float vTile = 2.0f;
            glm::vec2 uv1(t1 * uTile, (float)sl / slices * vTile);
            glm::vec2 uv2(t1 * uTile, (float)(sl + 1) / slices * vTile);
            glm::vec2 uv3(t2 * uTile, (float)sl / slices * vTile);
            glm::vec2 uv4(t2 * uTile, (float)(sl + 1) / slices * vTile);

            makeTile(p1, p2, p3, p4, uv1, uv2, uv3, uv4);
        }
    }

    // generate domed ends (hemispheres)
    int domeSegments = segments / 4;
    for (int end = 0; end < 2; end++) {
        float zOffset = (end == 0) ? -cylinderLength/2.0f : cylinderLength/2.0f;
        float direction = (end == 0) ? -1.0f : 1.0f;

        // hemisphere
        for (int seg = 0; seg < domeSegments; seg++) {
            float phi1 = (float)seg / domeSegments * (float)M_PI / 2.0f;
            float phi2 = (float)(seg + 1) / domeSegments * (float)M_PI / 2.0f;

            for (int sl = 0; sl < slices; sl++) {
                float theta1 = (float)sl / slices * 2.0f * (float)M_PI;
                float theta2 = (float)(sl + 1) / slices * 2.0f * (float)M_PI;

                // sphere coordinates: x = r*sin(phi)*cos(theta), y = r*sin(phi)*sin(theta), z = r*cos(phi)
                glm::vec3 p1(
                    radius * std::sin(phi1) * std::cos(theta1),
                    radius * std::sin(phi1) * std::sin(theta1),
                    zOffset + direction * radius * std::cos(phi1)
                );
                glm::vec3 p2(
                    radius * std::sin(phi1) * std::cos(theta2),
                    radius * std::sin(phi1) * std::sin(theta2),
                    zOffset + direction * radius * std::cos(phi1)
                );
                glm::vec3 p3(
                    radius * std::sin(phi2) * std::cos(theta1),
                    radius * std::sin(phi2) * std::sin(theta1),
                    zOffset + direction * radius * std::cos(phi2)
                );
                glm::vec3 p4(
                    radius * std::sin(phi2) * std::cos(theta2),
                    radius * std::sin(phi2) * std::sin(theta2),
                    zOffset + direction * radius * std::cos(phi2)
                );

                // add subtle bumps to dome ends too
                glm::vec3 normal1 = glm::normalize(p1 - glm::vec3(0, 0, zOffset));
                glm::vec3 normal2 = glm::normalize(p2 - glm::vec3(0, 0, zOffset));
                glm::vec3 normal3 = glm::normalize(p3 - glm::vec3(0, 0, zOffset));
                glm::vec3 normal4 = glm::normalize(p4 - glm::vec3(0, 0, zOffset));

                float bump1 = noise(theta1 * 6.0f, phi1 * 6.0f, 99) * 0.01f;
                float bump2 = noise(theta2 * 6.0f, phi1 * 6.0f, 99) * 0.01f;
                float bump3 = noise(theta1 * 6.0f, phi2 * 6.0f, 99) * 0.01f;
                float bump4 = noise(theta2 * 6.0f, phi2 * 6.0f, 99) * 0.01f;

                p1 += normal1 * bump1;
                p2 += normal2 * bump2;
                p3 += normal3 * bump3;
                p4 += normal4 * bump4;

                // tile UVs on dome ends too
                float uTile = 2.0f;
                float vTile = 2.0f;
                float u1 = (float)seg / domeSegments * uTile;
                float u2 = (float)(seg + 1) / domeSegments * uTile;
                float v1 = (float)sl / slices * vTile;
                float v2 = (float)(sl + 1) / slices * vTile;

                glm::vec2 uv1(u1, v1);
                glm::vec2 uv2(u1, v2);
                glm::vec2 uv3(u2, v1);
                glm::vec2 uv4(u2, v2);

                makeTile(p1, p2, p3, p4, uv1, uv2, uv3, uv4);
            }
        }
    }
}
