#include "Cube.h"
#include "utils/uvmapper.h"

void Cube::updateParams(int param1) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    setVertexData();
}

void Cube::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    // get per face normal
    glm::vec3 u = bottomLeft - topLeft;
    glm::vec3 v = topRight - topLeft;
    glm::vec3 normal = glm::normalize(glm::cross(u, v));

    // helper to insert position + normal + uv for a vertex
    auto helper = [&](glm::vec3 pos) {
        insertVec3(m_vertexData, pos);
        insertVec3(m_vertexData, normal);
        insertVec2(m_vertexData, getUVCoords(PrimitiveType::PRIMITIVE_CUBE, pos));
    };

    // triangle 1
    helper(topLeft);
    helper(bottomLeft);
    helper(bottomRight);

    // triangle 2
    helper(topLeft);
    helper(bottomRight);
    helper(topRight);
}

void Cube::makeFace(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight) {
    // create a single side of the cube out of the 4 given points and makeTile()
    int div = std::max(1, m_param1);  // avoid divide-by-zero

    for (int i = 0; i < div; i++) {
        for (int j = 0; j < div; j++) {
            // interpolate vertically
            glm::vec3 leftTop  = glm::mix(topLeft, bottomLeft, (float)i / div);
            glm::vec3 leftBot  = glm::mix(topLeft, bottomLeft, (float)(i + 1) / div);
            glm::vec3 rightTop = glm::mix(topRight, bottomRight, (float)i / div);
            glm::vec3 rightBot = glm::mix(topRight, bottomRight, (float)(i + 1) / div);

            // interpolate horizontally
            glm::vec3 pTL = glm::mix(leftTop, rightTop, (float)j / div);
            glm::vec3 pTR = glm::mix(leftTop, rightTop, (float)(j + 1) / div);
            glm::vec3 pBL = glm::mix(leftBot, rightBot, (float)j / div);
            glm::vec3 pBR = glm::mix(leftBot, rightBot, (float)(j + 1) / div);

            makeTile(pTL, pTR, pBL, pBR);
        }
    }
}

void Cube::setVertexData() {
    m_vertexData.clear();
    float h = 0.5f;

    // +z front
    makeFace({-h,  h,  h}, { h,  h,  h},
             {-h, -h,  h}, { h, -h,  h});

    // -z back
    makeFace({ h,  h, -h}, {-h,  h, -h},
             { h, -h, -h}, {-h, -h, -h});

    // +y top
    makeFace({-h,  h, -h}, { h,  h, -h},
             {-h,  h,  h}, { h,  h,  h});

    // -y bottom
    makeFace({-h, -h,  h}, { h, -h,  h},
             {-h, -h, -h}, { h, -h, -h});

    // +x right
    makeFace({ h,  h,  h}, { h,  h, -h},
             { h, -h,  h}, { h, -h, -h});

    // -x left
    makeFace({-h,  h, -h}, {-h,  h,  h},
             {-h, -h, -h}, {-h, -h,  h});
}

void Cube::insertVec3(std::vector<float> &data, glm::vec3 v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void Cube::insertVec2(std::vector<float> &data, glm::vec2 v) {
    data.push_back(v.x);
    data.push_back(v.y);
}