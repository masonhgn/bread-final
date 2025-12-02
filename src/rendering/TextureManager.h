#pragma once

#include <GL/glew.h>
#include <string>
#include <map>

class TextureManager {
public:
    TextureManager();
    ~TextureManager();

    // load texture from file path (cached)
    GLuint loadTexture(const std::string& filepath);

    // bind a texture to a specific texture unit
    void bindTexture(GLuint textureId, GLenum textureUnit);

    // clean up all loaded textures
    void cleanup();

private:
    // cache: filepath -> texture id
    std::map<std::string, GLuint> m_textureCache;
};
