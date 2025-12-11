#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

class ShadowManager {
public:
    ShadowManager();
    ~ShadowManager();

    void initialize(int shadowWidth = 2048, int shadowHeight = 2048);
    void bindForWriting();
    void bindForReading(GLuint textureUnit = 1);
    void unbind(GLuint defaultFBO = 0);
    GLuint getShadowTexture() const { return m_shadowTexture; }
    glm::mat4 computeLightSpaceMatrix(const glm::vec3& lightDir,
                                       const glm::vec3& sceneCenter,
                                       float sceneRadius);
    void cleanup();
    int getShadowWidth() const { return m_shadowWidth; }
    int getShadowHeight() const { return m_shadowHeight; }

private:
    GLuint m_shadowFBO;
    GLuint m_shadowTexture;
    int m_shadowWidth;
    int m_shadowHeight;
    bool m_initialized;
};
