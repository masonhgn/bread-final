#include "InstanceManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <iostream>

InstanceManager::InstanceManager() {
}

InstanceManager::~InstanceManager() {
    cleanup();
}

void InstanceManager::generateInstances(int count, float spreadRadius) {
    m_instanceMatrices.clear();
    m_instanceCount = count;

    //rng
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-spreadRadius, spreadRadius);
    std::uniform_real_distribution<float> scaleDist(0.5f, 1.5f);
    std::uniform_real_distribution<float> rotDist(0.0f, 360.0f);

    for (int i = 0; i < count; i++) {
        glm::mat4 model(1.0f);

        //random position
        float x = posDist(gen);
        float y = posDist(gen) * 0.3f;  //less vertical spread
        float z = posDist(gen);
        model = glm::translate(model, glm::vec3(x, y, z));

        float rotationY = rotDist(gen);
        model = glm::rotate(model, glm::radians(rotationY), glm::vec3(0, 1, 0));

        float scale = scaleDist(gen);
        model = glm::scale(model, glm::vec3(scale));

        m_instanceMatrices.push_back(model);
    }

    std::cout << "generated " << count << " instances!!" << std::endl;
}

void InstanceManager::uploadToGPU() {
    if (m_instanceMatrices.empty()) {
        std::cerr << "no instances to upload" << std::endl;
        return;
    }

    // delete old vbo if it exists
    if (m_instanceVBO != 0) {
        glDeleteBuffers(1, &m_instanceVBO);
    }

    //create and populate instance VBO
    glGenBuffers(1, &m_instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_instanceVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 m_instanceMatrices.size() * sizeof(glm::mat4),
                 &m_instanceMatrices[0],
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    std::cout << "uploaded " << m_instanceMatrices.size() << " instance matrices to gpu!" << std::endl;
}

void InstanceManager::cleanup() {
    if (m_instanceVBO != 0) {
        glDeleteBuffers(1, &m_instanceVBO);
        m_instanceVBO = 0;
    }
    m_instanceMatrices.clear();
    m_instanceCount = 0;
}
