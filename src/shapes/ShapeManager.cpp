#include "ShapeManager.h"
#include "Cube.h"
#include "Sphere.h"
#include "Cone.h"
#include "Cylinder.h"
#include <iostream>

ShapeManager::ShapeManager()
    : m_param1(1)
    , m_param2(1)
{
    m_cube = std::make_unique<Cube>();
    m_sphere = std::make_unique<Sphere>();
    m_cone = std::make_unique<Cone>();
    m_cylinder = std::make_unique<Cylinder>();
}

ShapeManager::~ShapeManager() {
    cleanup();
}



GLuint ShapeManager::createVAO(const std::vector<float>& vertexData, GLuint& vbo) {
    GLuint vao;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    // position attribute (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), reinterpret_cast<void*>(0));

    // normal attribute (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));

    // uv attribute (location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), reinterpret_cast<void*>(6 * sizeof(float)));

    // tangent attribute (location 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), reinterpret_cast<void*>(8 * sizeof(float)));

    // bitangent attribute (location 4)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), reinterpret_cast<void*>(11 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return vao;
}




void ShapeManager::generateShape(PrimitiveType type, int param1, int param2) {
    deleteShape(type);

    std::vector<float> vertexData;

    switch (type) {
        case PrimitiveType::PRIMITIVE_CUBE:
            m_cube->updateParams(std::max(param1, 1));
            vertexData = m_cube->generateShape();
            break;
        case PrimitiveType::PRIMITIVE_SPHERE:
            m_sphere->updateParams(std::max(param1, 2), std::max(param2, 3));
            vertexData = m_sphere->generateShape();
            break;
        case PrimitiveType::PRIMITIVE_CONE:
            m_cone->updateParams(std::max(param1, 1), std::max(param2, 3));
            vertexData = m_cone->generateShape();
            break;
        case PrimitiveType::PRIMITIVE_CYLINDER:
            m_cylinder->updateParams(std::max(param1, 1), std::max(param2, 3));
            vertexData = m_cylinder->generateShape();
            break;
        default:
            return;
    }

    ShapeData data;
    data.vao = createVAO(vertexData, data.vbo);
    data.vertexCount = vertexData.size() / 14;

    m_shapes[type] = data;
}

void ShapeManager::deleteShape(PrimitiveType type) {
    auto it = m_shapes.find(type);
    if (it != m_shapes.end()) {
        ShapeData& data = it->second;
        if (data.vao != 0) {
            glDeleteVertexArrays(1, &data.vao);
        }
        if (data.vbo != 0) {
            glDeleteBuffers(1, &data.vbo);
        }
        m_shapes.erase(it);
    }
}

void ShapeManager::initialize(int param1, int param2) {
    m_param1 = param1;
    m_param2 = param2;

    generateShape(PrimitiveType::PRIMITIVE_CUBE, param1, param2);
    generateShape(PrimitiveType::PRIMITIVE_SPHERE, param1, param2);
    generateShape(PrimitiveType::PRIMITIVE_CONE, param1, param2);
    generateShape(PrimitiveType::PRIMITIVE_CYLINDER, param1, param2);
}

void ShapeManager::updateTessellation(int param1, int param2) {
    if (param1 == m_param1 && param2 == m_param2) {
        return;
    }

    m_param1 = param1;
    m_param2 = param2;

    generateShape(PrimitiveType::PRIMITIVE_CUBE, param1, param2);
    generateShape(PrimitiveType::PRIMITIVE_SPHERE, param1, param2);
    generateShape(PrimitiveType::PRIMITIVE_CONE, param1, param2);
    generateShape(PrimitiveType::PRIMITIVE_CYLINDER, param1, param2);
}

GLuint ShapeManager::getVAO(PrimitiveType type) const {
    auto it = m_shapes.find(type);
    if (it != m_shapes.end()) {
        return it->second.vao;
    }
    return 0;
}

int ShapeManager::getVertexCount(PrimitiveType type) const {
    auto it = m_shapes.find(type);
    if (it != m_shapes.end()) {
        return it->second.vertexCount;
    }
    return 0;
}

void ShapeManager::cleanup() {
    for (auto& pair : m_shapes) {
        ShapeData& data = pair.second;
        if (data.vao != 0) {
            glDeleteVertexArrays(1, &data.vao);
        }
        if (data.vbo != 0) {
            glDeleteBuffers(1, &data.vbo);
        }
    }
    m_shapes.clear();
}