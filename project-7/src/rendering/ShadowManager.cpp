#include "ShadowManager.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

ShadowManager::ShadowManager()
    : m_shadowFBO(0)
    , m_shadowTexture(0)
    , m_shadowWidth(2048)
    , m_shadowHeight(2048)
    , m_initialized(false)
{
}

ShadowManager::~ShadowManager() {
    cleanup();
}

void ShadowManager::initialize(int shadowWidth, int shadowHeight) {
    if (m_initialized) {
        cleanup();
    }

    m_shadowWidth = shadowWidth;
    m_shadowHeight = shadowHeight;

    glGenTextures(1, &m_shadowTexture);
    glBindTexture(GL_TEXTURE_2D, m_shadowTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F,
                 m_shadowWidth, m_shadowHeight, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

    glGenFramebuffers(1, &m_shadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                          GL_TEXTURE_2D, m_shadowTexture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Shadow framebuffer is not complete! Status: " << status << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_initialized = true;
    std::cout << "Shadow mapping initialized: " << m_shadowWidth
              << "x" << m_shadowHeight << " (32-bit depth)" << std::endl;
}

void ShadowManager::bindForWriting() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowFBO);
    glViewport(0, 0, m_shadowWidth, m_shadowHeight);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);

    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowManager::bindForReading(GLuint textureUnit) {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_shadowTexture);
}

void ShadowManager::unbind(GLuint defaultFBO) {

    glDisable(GL_POLYGON_OFFSET_FILL);

    glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);
}

glm::mat4 ShadowManager::computeLightSpaceMatrix(const glm::vec3& lightDir,
                                                   const glm::vec3& sceneCenter,
                                                   float sceneRadius) {

    glm::vec3 normalizedLightDir = glm::normalize(lightDir);
    glm::vec3 lightPos = sceneCenter - normalizedLightDir * sceneRadius * 2.5f;

    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    if (glm::abs(glm::dot(normalizedLightDir, up)) > 0.95f) {

        up = glm::vec3(0.0f, 0.0f, 1.0f);
    }

    glm::mat4 lightView = glm::lookAt(lightPos, sceneCenter, up);

    float orthoSize = sceneRadius * 1.8f;
    float nearPlane = 0.1f;
    float farPlane = sceneRadius * 5.0f;

    glm::mat4 lightProjection = glm::ortho(
        -orthoSize, orthoSize,
        -orthoSize, orthoSize,
        nearPlane, farPlane
    );
    return lightProjection * lightView;
}

void ShadowManager::cleanup() {
    if (m_shadowTexture != 0) {
        glDeleteTextures(1, &m_shadowTexture);
        m_shadowTexture = 0;
    }
    if (m_shadowFBO != 0) {
        glDeleteFramebuffers(1, &m_shadowFBO);
        m_shadowFBO = 0;
    }
    m_initialized = false;
}
