#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class InstanceManager {
public:
    InstanceManager();
    ~InstanceManager();

    // generate random instance transformations
    void generateInstances(int count, float spreadRadius);

    // upload instance data to GPU
    void uploadToGPU();

    // get the instance VBO for binding
    GLuint getInstanceVBO() const { return m_instanceVBO; }

    // get number of instances
    int getInstanceCount() const { return m_instanceCount; }

    // cleanup
    void cleanup();

private:
    std::vector<glm::mat4> m_instanceMatrices;
    GLuint m_instanceVBO = 0;
    int m_instanceCount = 0;
};
