#pragma once

#include <GL/glew.h>
#include <memory>
#include <unordered_map>
#include "utils/scenedata.h"

class Cube;
class Sphere;
class Cone;
class Cylinder;

class ShapeManager {
public:
    ShapeManager();
    ~ShapeManager();

    void initialize(int param1, int param2);
    void updateTessellation(int param1, int param2);

    GLuint getVAO(PrimitiveType type) const;
    int getVertexCount(PrimitiveType type) const;

    // configure instance attributes for a VAO
    void setupInstanceAttributes(PrimitiveType type, GLuint instanceVBO);

    void cleanup();

private:
    std::unique_ptr<Cube> m_cube;
    std::unique_ptr<Sphere> m_sphere;
    std::unique_ptr<Cone> m_cone;
    std::unique_ptr<Cylinder> m_cylinder;

    int m_param1;
    int m_param2;

    struct ShapeData {
        GLuint vao = 0;
        GLuint vbo = 0;
        int vertexCount = 0;
    };

    std::unordered_map<PrimitiveType, ShapeData> m_shapes;

    void generateShape(PrimitiveType type, int param1, int param2);
    void deleteShape(PrimitiveType type);
    GLuint createVAO(const std::vector<float>& vertexData, GLuint& vbo);
};