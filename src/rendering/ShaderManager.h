#pragma once

#include <GL/glew.h>
#include <string>
#include <glm/glm.hpp>
#include "utils/scenedata.h"

class ShaderManager {
public:
    ShaderManager();
    ~ShaderManager();

    bool loadShaders(const std::string& vertPath, const std::string& fragPath);

    void use() const;
    GLuint getProgram() const { return m_program; }

    void setUniformMat4(const std::string& name, const glm::mat4& mat) const;
    void setUniformVec3(const std::string& name, const glm::vec3& vec) const;
    void setUniformVec4(const std::string& name, const glm::vec4& vec) const;
    void setUniformFloat(const std::string& name, float value) const;
    void setUniformInt(const std::string& name, int value) const;
    void setUniformBool(const std::string& name, bool value) const;

    void setLight(int index, const SceneLightData& light) const;

    void cleanup();

private:
    GLuint m_program = 0;

    GLint getUniformLocation(const std::string& name) const;
};
