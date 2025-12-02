#include "TextureManager.h"
#include <QImage>
#include <iostream>

TextureManager::TextureManager() {
}

TextureManager::~TextureManager() {
    cleanup();
}

GLuint TextureManager::loadTexture(const std::string& filepath) {
    // check if texture is already loaded
    auto it = m_textureCache.find(filepath);
    if (it != m_textureCache.end()) {
        return it->second;
    }

    // load image using qt
    QImage image(QString::fromStdString(filepath));
    if (image.isNull()) {
        std::cerr << "failed to load texture: " << filepath << std::endl;
        return 0;
    }

    // convert to opengl format (rgba, flipped vertically)
    image = image.convertToFormat(QImage::Format_RGBA8888);
    image = image.flipped(Qt::Vertical);  // flip vertically for opengl

    // generate and bind texture
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    // upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // generate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);

    // unbind
    glBindTexture(GL_TEXTURE_2D, 0);

    // cache the texture
    m_textureCache[filepath] = textureId;

    std::cout << "loaded texture: " << filepath << " (id: " << textureId << ")" << std::endl;

    return textureId;
}

void TextureManager::bindTexture(GLuint textureId, GLenum textureUnit) {
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, textureId);
}

void TextureManager::cleanup() {
    for (auto& pair : m_textureCache) {
        glDeleteTextures(1, &pair.second);
    }
    m_textureCache.clear();
}
