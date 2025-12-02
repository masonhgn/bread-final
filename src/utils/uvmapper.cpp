#include "uvmapper.h"
#include <cmath>
#include <glm/glm.hpp>

namespace {
    inline float wrap01(float t) {
        return t - floorf(t);
    }
}

glm::vec2 getUVCoords(PrimitiveType type, const glm::vec3 &pOS) {
    switch (type) {
        //cube
        case PrimitiveType::PRIMITIVE_CUBE: {
            const float x = pOS.x, y = pOS.y, z = pOS.z;
            const glm::vec3 a = glm::abs(pOS);
            glm::vec3 center, T, B;
            const float eps = 0.0001f;

            if (a.x > a.y + eps && a.x > a.z + eps) {
                if (x > 0.f) { center = {+0.5f, 0.f, 0.f}; T = {0, 0, -1}; B = {0, 1, 0}; }
                else         { center = {-0.5f, 0.f, 0.f}; T = {0, 0, +1}; B = {0, 1, 0}; }
            } else if (a.y > a.x + eps && a.y > a.z + eps) {
                if (y > 0.f) { center = {0.f, +0.5f, 0.f}; T = {+1, 0, 0}; B = {0, 0, -1}; }
                else         { center = {0.f, -0.5f, 0.f}; T = {+1, 0, 0}; B = {0, 0, +1}; }
            } else {
                if (z > 0.f) { center = {0.f, 0.f, +0.5f}; T = {+1, 0, 0}; B = {0, 1, 0}; }
                else         { center = {0.f, 0.f, -0.5f}; T = {-1, 0, 0}; B = {0, 1, 0}; }
            }

            glm::vec3 q = pOS - center;
            float u = glm::dot(q, T) + 0.5f;
            float v = 0.5f - glm::dot(q, B);

            return {u, v};
        }

        //sphere
        case PrimitiveType::PRIMITIVE_SPHERE: {
            glm::vec3 n = glm::normalize(pOS);
            float theta = atan2f(n.z, n.x);
            float u = 0.5f + theta / (2.0f * float(M_PI));
            float v = 0.5f - asin(glm::clamp(n.y, -1.0f, 1.0f)) / float(M_PI); 

            // u *= textureScale;
            // v *= textureScale;
            u = u - floorf(u);
            v = v - floorf(v);
            return {u, v};
        }

        //cylinder
        case PrimitiveType::PRIMITIVE_CYLINDER: {
            if (fabsf(pOS.y - 0.5f) < UV_EPSILON) { // top cap
                glm::vec3 q = pOS - glm::vec3(0, +0.5f, 0);
                float u = q.x + 0.5f, v = 0.5f - q.z;
                // return {wrap01(u * textureScale), wrap01(v * textureScale)};
                return {u,v};
            }
            if (fabsf(pOS.y + 0.5f) < UV_EPSILON) { // bottom cap
                glm::vec3 q = pOS - glm::vec3(0, -0.5f, 0);
                float u = q.x + 0.5f, v = 0.5f - q.z;
                // return {wrap01(u * textureScale), wrap01(v * textureScale)};
                return {u,v};
            }

            float theta = atan2f(pOS.x, -pOS.z);
            float u = 0.5f - theta / (2.0f * float(M_PI)) - 0.25f;
            float v = 0.5f - pOS.y;
            // u *= textureScale;
            // v *= textureScale;
            // return {wrap01(u), wrap01(v)};
            return {u,v};
        }

        //cone
        case PrimitiveType::PRIMITIVE_CONE: {
            if (fabsf(pOS.y + 0.5f) < UV_EPSILON) {
                float u = pOS.x + 0.5f, v = 0.5f - pOS.z;
                // return {wrap01(u * textureScale), wrap01(v * textureScale)};
                return {u,v};
            }

            float theta = atan2f(pOS.x, pOS.z);
            float u = theta / (2.0f * float(M_PI)) - 0.25f;
            float v = 0.5f - pOS.y;
            // u *= textureScale;
            // v *= textureScale;
            // return {wrap01(u), wrap01(v)};
            return {u,v};
        }


        case PrimitiveType::PRIMITIVE_MESH: {

            float u = pOS.x;
            float v = pOS.z;


            // u *= textureScale;
            // v *= textureScale;
            // u = u - floorf(u);
            // v = v - floorf(v);
            return {u, v};
        }



        default:
            return {pOS.x, pOS.z};
    }
}
