#include "ShaderManager.h"
#include "utils/shaderloader.h"
#include <iostream>

ShaderManager::ShaderManager() {}

ShaderManager::~ShaderManager() {
    cleanup();
}

bool ShaderManager::loadShaders(const std::string& vertPath, const std::string& fragPath) {
    try {
        m_program = ShaderLoader::createShaderProgram(vertPath.c_str(), fragPath.c_str());
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Shader loading failed: " << e.what() << std::endl;
        return false;
    }
}

void ShaderManager::use() const {
    glUseProgram(m_program);
}

void ShaderManager::cleanup() {
    if (m_program != 0) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
}

GLint ShaderManager::getUniformLocation(const std::string& name) const {
    return glGetUniformLocation(m_program, name.c_str());
}

void ShaderManager::setUniformMat4(const std::string& name, const glm::mat4& mat) const {
    GLint loc = getUniformLocation(name);
    if (loc != -1) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
    }
}

void ShaderManager::setUniformVec3(const std::string& name, const glm::vec3& vec) const {
    GLint loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform3fv(loc, 1, &vec[0]);
    }
}

void ShaderManager::setUniformVec4(const std::string& name, const glm::vec4& vec) const {
    GLint loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform4fv(loc, 1, &vec[0]);
    }
}

void ShaderManager::setUniformFloat(const std::string& name, float value) const {
    GLint loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform1f(loc, value);
    }
}

void ShaderManager::setUniformInt(const std::string& name, int value) const {
    GLint loc = getUniformLocation(name);
    if (loc != -1) {
        glUniform1i(loc, value);
    }
}

void ShaderManager::setLight(int index, const SceneLightData& light) const {
    std::string base = "lights[" + std::to_string(index) + "]";

    setUniformInt(base + ".type", static_cast<int>(light.type));
    setUniformVec4(base + ".color", light.color);
    setUniformVec3(base + ".function", light.function);
    setUniformVec3(base + ".pos", glm::vec3(light.pos));
    setUniformVec3(base + ".dir", glm::vec3(light.dir));
    setUniformFloat(base + ".penumbra", light.penumbra);
    setUniformFloat(base + ".angle", light.angle);
}
