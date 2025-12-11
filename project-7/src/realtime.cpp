#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "settings.h"
#include "utils/shaderloader.h"

namespace {
    // simple hash-based noise function for procedural texture generation
    float hash(float x, float y, int seed) {
        int xi = static_cast<int>(std::floor(x));
        int yi = static_cast<int>(std::floor(y));
        int n = xi + yi * 57 + seed * 131;
        n = (n << 13) ^ n;
        return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
    }

    float noise(float x, float y, int seed) {
        int xi = static_cast<int>(std::floor(x));
        int yi = static_cast<int>(std::floor(y));
        float xf = x - xi;
        float yf = y - yi;

        float u = xf * xf * (3.0f - 2.0f * xf);
        float v = yf * yf * (3.0f - 2.0f * yf);

        float n00 = hash(xi, yi, seed);
        float n10 = hash(xi + 1, yi, seed);
        float n01 = hash(xi, yi + 1, seed);
        float n11 = hash(xi + 1, yi + 1, seed);

        float nx0 = n00 * (1.0f - u) + n10 * u;
        float nx1 = n01 * (1.0f - u) + n11 * u;

        return nx0 * (1.0f - v) + nx1 * v;
    }

    // fractal brownian motion - layered noise for detail
    float fbm(float x, float y, int seed, int octaves) {
        float result = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;
        float maxValue = 0.0f;

        for (int i = 0; i < octaves; i++) {
            result += noise(x * frequency, y * frequency, seed + i) * amplitude;
            maxValue += amplitude;
            amplitude *= 0.5f;
            frequency *= 2.0f;
        }

        return result / maxValue;
    }
}

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Shift]   = false;
    m_keyMap[Qt::Key_Space]   = false;

    m_shadowShaderProgram = 0;
    m_terrainVAO = 0;
    m_terrainVBO = 0;
    m_terrainVertexCount = 0;
    m_terrainGenerated = false;
    m_terrainTexture = 0;
    m_terrainNormalMap = 0;

    m_baguetteVAO = 0;
    m_baguetteVBO = 0;
    m_baguetteVertexCount = 0;

    m_loafVAO = 0;
    m_loafVBO = 0;
    m_loafVertexCount = 0;

    m_bloomFBO = 0;
    m_sceneTexture = 0;
    m_sceneDepthBuffer = 0;
    m_brightTexture = 0;
    m_blurTexture[0] = 0;
    m_blurTexture[1] = 0;
    m_bloomBrightShader = 0;
    m_bloomBlurShader = 0;
    m_bloomCompositeShader = 0;
    m_fullscreenQuadVAO = 0;
    m_fullscreenQuadVBO = 0;

    m_colorGradingShader = 0;
    m_colorGradingTexture = 0;
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    m_shapeManager.cleanup();
    m_shaderManager.cleanup();
    m_textureManager.cleanup();
    m_instanceManager.cleanup();
    m_shadowManager.cleanup();
    cleanupBloom();
    cleanupColorGrading();

    if (m_defaultWhiteTexture != 0) {
        glDeleteTextures(1, &m_defaultWhiteTexture);
        m_defaultWhiteTexture = 0;
    }

    if (m_shadowShaderProgram != 0) {
        glDeleteProgram(m_shadowShaderProgram);
        m_shadowShaderProgram = 0;
    }

    if (m_terrainVAO != 0) {
        glDeleteVertexArrays(1, &m_terrainVAO);
        m_terrainVAO = 0;
    }
    if (m_terrainVBO != 0) {
        glDeleteBuffers(1, &m_terrainVBO);
        m_terrainVBO = 0;
    }

    if (m_terrainTexture != 0) {
        glDeleteTextures(1, &m_terrainTexture);
        m_terrainTexture = 0;
    }
    if (m_terrainNormalMap != 0) {
        glDeleteTextures(1, &m_terrainNormalMap);
        m_terrainNormalMap = 0;
    }

    if (m_baguetteVAO != 0) {
        glDeleteVertexArrays(1, &m_baguetteVAO);
        m_baguetteVAO = 0;
    }
    if (m_baguetteVBO != 0) {
        glDeleteBuffers(1, &m_baguetteVBO);
        m_baguetteVBO = 0;
    }

    this->doneCurrent();
}

void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    bool shadersLoaded = m_shaderManager.loadShaders(
        ":/resources/shaders/default.vert",
        ":/resources/shaders/default.frag"
    );

    if (!shadersLoaded) {
        std::cerr << "Failed to load shaders!" << std::endl;
        return;
    }

    // create default white texture for objects without textures
    glGenTextures(1, &m_defaultWhiteTexture);
    glBindTexture(GL_TEXTURE_2D, m_defaultWhiteTexture);
    unsigned char whitePixel[4] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // set up texture units for default shader
    m_shaderManager.use();
    m_shaderManager.setUniformInt("diffuseTexture", 0);
    m_shaderManager.setUniformInt("normalMap", 1);
    glUseProgram(0);

    m_shapeManager.initialize(settings.shapeParameter1, settings.shapeParameter2);

    // load shadow shaders
    try {
        m_shadowShaderProgram = ShaderLoader::createShaderProgram(
            ":/resources/shaders/shadow.vert",
            ":/resources/shaders/shadow.frag"
        );
        m_shadowManager.initialize(4096, 4096);
        std::cout << "Shadow shaders loaded successfully (4096x4096 shadow map)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to load shadow shaders: " << e.what() << std::endl;
        std::cerr << "Continuing without shadows..." << std::endl;
        m_shadowShaderProgram = 0;
        settings.enableShadows = false;
    }

    // generate large terrain (500x500 grid for pseudo-infinite effect)
    m_terrainGenerator.generate(500, 500, settings.terrainScale, settings.terrainScale,
                                 settings.terrainHeightScale, (int)settings.terrainDetailLevel,
                                 0.5f, 2.0f);
    generateTerrainVAO();

    // generate baguette mesh
    m_baguette.generate(40, 24);  // 40 segments along length, 24 slices around
    generateBaguetteVAO();

    // generate loaf mesh
    m_loaf.generate(30, 20);  // 30 segments, 20 slices
    generateLoafVAO();

    // generate procedural bread texture with air pockets
    const int texWidth = 1024;
    const int texHeight = 1024;
    std::vector<unsigned char> breadTextureData(texWidth * texHeight * 3);

    auto hash = [](int x, int y, int seed) -> float {
        int n = x + y * 57 + seed * 131;
        n = (n << 13) ^ n;
        return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
    };

    auto smoothNoise = [&hash](float x, float y, int seed) -> float {
        int ix = (int)std::floor(x);
        int iy = (int)std::floor(y);
        float fx = x - ix;
        float fy = y - iy;
        fx = fx * fx * (3.0f - 2.0f * fx);
        fy = fy * fy * (3.0f - 2.0f * fy);
        float v00 = hash(ix, iy, seed);
        float v10 = hash(ix + 1, iy, seed);
        float v01 = hash(ix, iy + 1, seed);
        float v11 = hash(ix + 1, iy + 1, seed);
        return v00 * (1-fx) * (1-fy) + v10 * fx * (1-fy) + v01 * (1-fx) * fy + v11 * fx * fy;
    };

    auto fbm = [&smoothNoise](float x, float y, int seed, int octaves) -> float {
        float value = 0.0f;
        float amplitude = 1.0f;
        float frequency = 1.0f;
        float maxValue = 0.0f;
        for (int i = 0; i < octaves; i++) {
            value += amplitude * smoothNoise(x * frequency, y * frequency, seed + i * 100);
            maxValue += amplitude;
            amplitude *= 0.5f;
            frequency *= 2.0f;
        }
        return value / maxValue;
    };

    for (int y = 0; y < texHeight; y++) {
        for (int x = 0; x < texWidth; x++) {
            int idx = (y * texWidth + x) * 3;

            // lighter, fresh bread color
            float r = 0.98f;
            float g = 0.94f;
            float b = 0.82f;

            float colorVar = fbm(x * 0.006f, y * 0.006f, 1, 4);
            r += (colorVar - 0.5f) * 0.08f;
            g += (colorVar - 0.5f) * 0.06f;
            b += (colorVar - 0.5f) * 0.05f;

            float holeDarkness = 0.0f;

            // multiple scales of air pockets
            float bigHole = smoothNoise(x * 0.012f, y * 0.012f, 42);
            if (bigHole > 0.35f) {
                float t = (bigHole - 0.35f) / 0.65f;
                t = t * t * t;
                holeDarkness = std::max(holeDarkness, t * 0.7f);
            }

            float medHole = smoothNoise(x * 0.028f, y * 0.028f, 137);
            if (medHole > 0.38f) {
                float t = (medHole - 0.38f) / 0.62f;
                t = t * t;
                holeDarkness = std::max(holeDarkness, t * 0.55f);
            }

            float smallHole = smoothNoise(x * 0.06f, y * 0.06f, 293);
            if (smallHole > 0.4f) {
                float t = (smallHole - 0.4f) / 0.6f;
                t = t * t;
                holeDarkness = std::max(holeDarkness, t * 0.45f);
            }

            float tinyHole = smoothNoise(x * 0.12f, y * 0.12f, 419);
            if (tinyHole > 0.42f) {
                float t = (tinyHole - 0.42f) / 0.58f;
                holeDarkness = std::max(holeDarkness, t * 0.35f);
            }

            float microHole = smoothNoise(x * 0.25f, y * 0.25f, 587);
            if (microHole > 0.45f) {
                float t = (microHole - 0.45f) / 0.55f;
                holeDarkness = std::max(holeDarkness, t * 0.25f);
            }

            // lighter shadows for holes (still bread-colored)
            float shadowR = 0.82f, shadowG = 0.76f, shadowB = 0.60f;
            r = r * (1.0f - holeDarkness) + shadowR * holeDarkness;
            g = g * (1.0f - holeDarkness) + shadowG * holeDarkness;
            b = b * (1.0f - holeDarkness) + shadowB * holeDarkness;

            // add grain
            float grain1 = smoothNoise(x * 0.5f, y * 0.5f, 701);
            float grain2 = smoothNoise(x * 0.8f, y * 0.8f, 811);
            float grain3 = hash(x, y, 919) * 0.5f + 0.5f;

            float grainEffect = (grain1 * 0.4f + grain2 * 0.35f + grain3 * 0.25f);
            r *= (0.92f + grainEffect * 0.16f);
            g *= (0.92f + grainEffect * 0.16f);
            b *= (0.90f + grainEffect * 0.20f);

            // random dark spots
            if (hash(x, y, 1009) > 0.92f) {
                r *= 0.75f;
                g *= 0.70f;
                b *= 0.65f;
            }

            // random bright spots (not in holes)
            if (hash(x, y, 1013) > 0.94f && holeDarkness < 0.2f) {
                r = std::min(1.0f, r * 1.15f);
                g = std::min(1.0f, g * 1.12f);
                b = std::min(1.0f, b * 1.08f);
            }

            breadTextureData[idx + 0] = (unsigned char)(std::min(255.0f, std::max(0.0f, r * 255)));
            breadTextureData[idx + 1] = (unsigned char)(std::min(255.0f, std::max(0.0f, g * 255)));
            breadTextureData[idx + 2] = (unsigned char)(std::min(255.0f, std::max(0.0f, b * 255)));
        }
    }

    glGenTextures(1, &m_terrainTexture);
    glBindTexture(GL_TEXTURE_2D, m_terrainTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, breadTextureData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);

    std::cout << "Ultra-realistic bread texture with air pockets: " << texWidth << "x" << texHeight << std::endl;
    std::cout << "Terrain initialized: " << m_terrainVertexCount << " vertices, enabled="
              << settings.enableTerrain << std::endl;

    m_lightSpaceMatrix = glm::mat4(1.0f);

    // initialize bloom post-processing
    initializeBloom();

    // initialize color grading
    initializeColorGrading();

    m_initialized = true;

    if (!settings.sceneFilePath.empty()) {
        std::cout << "Loading default scene: " << settings.sceneFilePath << std::endl;
        sceneChanged();
    }
}

void Realtime::setGlobalUniforms() {
    m_shaderManager.setUniformMat4("viewMatrix", m_camera->getViewMatrix());
    m_shaderManager.setUniformMat4("projectionMatrix", m_camera->getProjectionMatrix());
    m_shaderManager.setUniformVec3("cameraPos", m_camera->getPosition());

    int numLights = std::min(static_cast<int>(m_renderData.lights.size()), 8);
    m_shaderManager.setUniformInt("numLights", numLights);

    // debug: print light count once
    static bool printedLights = false;
    if (!printedLights) {
        std::cout << "number of lights in scene: " << numLights << std::endl;
        printedLights = true;
    }

    for (int i = 0; i < numLights; i++) {
        m_shaderManager.setLight(i, m_renderData.lights[i]);
    }
}

void Realtime::renderShape(const RenderShapeData& shape) {
    if (settings.enableInstancing && shape.primitive.type == PrimitiveType::PRIMITIVE_CUBE) {
        return;
    }

    m_shaderManager.setUniformBool("useInstancing", false);
    m_shaderManager.setUniformMat4("modelMatrix", shape.ctm);

    const SceneMaterial& mat = shape.primitive.material;
    const SceneGlobalData& global = m_renderData.globalData;

    glm::vec4 ambient = mat.cAmbient * global.ka;
    glm::vec4 diffuse = mat.cDiffuse * global.kd;
    glm::vec4 specular = mat.cSpecular * global.ks;

    m_shaderManager.setUniformVec4("ambientColor", ambient);
    m_shaderManager.setUniformVec4("diffuseColor", diffuse);
    m_shaderManager.setUniformVec4("specularColor", specular);
    m_shaderManager.setUniformFloat("shininess", mat.shininess);

    bool hasDiffuseTexture = (m_breadTextureId != 0);
    m_shaderManager.setUniformBool("hasDiffuseTexture", hasDiffuseTexture);

    if (hasDiffuseTexture) {
        m_textureManager.bindTexture(m_breadTextureId, GL_TEXTURE0);
        m_shaderManager.setUniformInt("diffuseTexture", 0);

        static bool printed = false;
        if (!printed) {
            std::cout << "bread diffuse texture is active (texture id: " << m_breadTextureId << ")" << std::endl;
            printed = true;
        }
    } else {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_defaultWhiteTexture);
    }

    bool hasNormalMap = (m_testNormalMapId != 0) && settings.enableNormalMapping;
    m_shaderManager.setUniformBool("hasNormalMap", hasNormalMap);

    if (hasNormalMap) {
        m_textureManager.bindTexture(m_testNormalMapId, GL_TEXTURE1);
        m_shaderManager.setUniformInt("normalMap", 1);

        static bool printed = false;
        if (!printed) {
            std::cout << "normal mapping is active (texture id: " << m_testNormalMapId << ")" << std::endl;
            printed = true;
        }
    } else {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    GLuint vao = m_shapeManager.getVAO(shape.primitive.type);
    int vertexCount = m_shapeManager.getVertexCount(shape.primitive.type);

    if (vao == 0 || vertexCount == 0) {
        return;
    }

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

void Realtime::paintGL() {
    if (!m_sceneLoaded || !m_camera) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return;
    }

    // render shadow map first if shadows are enabled
    if (settings.enableShadows && m_shadowShaderProgram != 0 && !m_renderData.lights.empty()) {

        glm::vec3 lightDir(0, -1, 0);
        bool foundDirectionalLight = false;
        for (const auto& light : m_renderData.lights) {
            if (light.type == LightType::LIGHT_DIRECTIONAL) {
                lightDir = glm::normalize(glm::vec3(light.dir));
                foundDirectionalLight = true;
                break;
            }
        }

        static bool loggedOnce = false;
        static int frameCount = 0;
        frameCount++;
        if (!loggedOnce || frameCount % 300 == 0) {
            std::cout << "=== SHADOW DEBUG ===" << std::endl;
            std::cout << "Light direction: [" << lightDir.x << ", " << lightDir.y << ", " << lightDir.z << "]" << std::endl;
            std::cout << "Num shapes to render: " << m_renderData.shapes.size() << std::endl;
            std::cout << "Shadow map size: " << m_shadowManager.getShadowWidth() << "x" << m_shadowManager.getShadowHeight() << std::endl;
            std::cout << "Shadow FBO texture: " << m_shadowManager.getShadowTexture() << std::endl;
            loggedOnce = true;
        }

        // shadow map center and radius adjusted for large terrain
        glm::vec3 sceneCenter(0.0f, -5.0f, 0.0f);
        float sceneRadius = 150.0f;
        m_lightSpaceMatrix = m_shadowManager.computeLightSpaceMatrix(
            lightDir, sceneCenter, sceneRadius
        );

        m_shadowManager.bindForWriting();

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glDisable(GL_CULL_FACE);

        renderSceneToShadowMap();

        m_shadowManager.unbind(defaultFramebufferObject());

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }

    // render normal scene
    int width = size().width() * m_devicePixelRatio;
    int height = size().height() * m_devicePixelRatio;
    glViewport(0, 0, width, height);

    // bind bloom framebuffer if enabled
    if (settings.enableBloom && m_bloomFBO != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_bloomFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_sceneTexture, 0);
    }

    // set background color to match fog color if fog is enabled
    if (settings.enableFog) {
        glClearColor(settings.fogColor.r, settings.fogColor.g, settings.fogColor.b, 1.0f);
    } else {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_defaultWhiteTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_defaultWhiteTexture);

    m_shaderManager.use();

    setGlobalUniforms();

    // set all shader uniforms
    m_shaderManager.setUniformInt("enableShadows", settings.enableShadows ? 1 : 0);
    m_shaderManager.setUniformMat4("lightSpaceMatrix", m_lightSpaceMatrix);
    m_shaderManager.setUniformFloat("shadowBias", settings.shadowBias);

    if (m_shadowShaderProgram != 0) {
        m_shadowManager.bindForReading(1);
        m_shaderManager.setUniformInt("shadowMap", 1);
    }

    m_shaderManager.setUniformInt("enableFog", settings.enableFog ? 1 : 0);
    m_shaderManager.setUniformInt("enableNormalMapping", settings.enableNormalMapping ? 1 : 0);
    m_shaderManager.setUniformInt("enableScrolling", settings.enableScrolling ? 1 : 0);
    m_shaderManager.setUniformInt("hasDiffuseTexture", 0);
    m_shaderManager.setUniformInt("hasNormalMap", 0);

    m_shaderManager.setUniformInt("diffuseTexture", 0);
    m_shaderManager.setUniformInt("normalMap", 1);

    m_shaderManager.setUniformVec3("fogColor", settings.fogColor);
    m_shaderManager.setUniformFloat("fogStart", settings.fogStart);
    m_shaderManager.setUniformFloat("fogEnd", settings.fogEnd);
    m_shaderManager.setUniformFloat("fogDensity", settings.fogDensity);
    m_shaderManager.setUniformFloat("time", m_elapsedTime);

    m_shaderManager.setUniformVec2("scrollDirection", settings.scrollDirection);
    m_shaderManager.setUniformFloat("scrollSpeed", settings.scrollSpeed);

    // render baguettes (custom mesh instead of cylinders)
    renderBaguettes();

    // render loaves (custom rounded rectangular bread)
    renderLoaves();

    // disabled: render cylinder primitives from scene file
    // for (const RenderShapeData& shape : m_renderData.shapes) {
    //     renderShape(shape);
    // }

    // disabled: render instanced cubes (showing only terrain)
    if (false && settings.enableInstancing && m_instanceManager.getInstanceCount() > 0) {
        m_shaderManager.setUniformBool("useInstancing", true);

        glm::vec4 ambient = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f) * m_renderData.globalData.ka;
        glm::vec4 diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) * m_renderData.globalData.kd;
        glm::vec4 specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f) * m_renderData.globalData.ks;

        m_shaderManager.setUniformVec4("ambientColor", ambient);
        m_shaderManager.setUniformVec4("diffuseColor", diffuse);
        m_shaderManager.setUniformVec4("specularColor", specular);
        m_shaderManager.setUniformFloat("shininess", 25.0f);

        bool hasDiffuseTexture = (m_breadTextureId != 0);
        m_shaderManager.setUniformBool("hasDiffuseTexture", hasDiffuseTexture);

        if (hasDiffuseTexture) {
            m_textureManager.bindTexture(m_breadTextureId, GL_TEXTURE0);
        }

        GLuint vao = m_shapeManager.getVAO(PrimitiveType::PRIMITIVE_CUBE);
        int vertexCount = m_shapeManager.getVertexCount(PrimitiveType::PRIMITIVE_CUBE);

        if (vao != 0 && vertexCount > 0) {
            glBindVertexArray(vao);
            glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount, m_instanceManager.getInstanceCount());
        }
    }

    // render terrain if enabled
    if (settings.enableTerrain) {
        renderTerrain();
    }

    glBindVertexArray(0);
    glUseProgram(0);

    // apply bloom post-processing if enabled
    if (settings.enableBloom && m_bloomFBO != 0) {
        renderBloom();

        // apply color grading to bloomed result
        if (settings.enableColorGrading && m_colorGradingShader != 0) {
            applyColorGrading(m_colorGradingTexture);
        }
    } else if (settings.enableColorGrading && m_colorGradingShader != 0) {
        // bloom disabled but color grading enabled - apply directly to scene
        applyColorGrading(m_sceneTexture);
    }
}

void Realtime::resizeGL(int w, int h) {
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    if (m_camera) {
        float aspectRatio = static_cast<float>(w) / h;
        m_camera->updateAspectRatio(aspectRatio);
    }

    // recreate bloom textures with new size
    if (m_bloomFBO != 0) {
        cleanupBloom();
        initializeBloom();
    }

    // recreate color grading texture with new size
    if (m_colorGradingShader != 0) {
        cleanupColorGrading();
        initializeColorGrading();
    }

    update();
}

void Realtime::sceneChanged() {
    makeCurrent();

    m_sceneLoaded = SceneParser::parse(settings.sceneFilePath, m_renderData);

    if (!m_sceneLoaded) {
        std::cerr << "Failed to load scene: " << settings.sceneFilePath << std::endl;
        return;
    }

    int width = size().width() * m_devicePixelRatio;
    int height = size().height() * m_devicePixelRatio;
    float aspectRatio = static_cast<float>(width) / height;

    m_camera = std::make_unique<Camera>(
        m_renderData.cameraData,
        aspectRatio,
        settings.nearPlane,
        settings.farPlane
    );

    // use procedural bread texture already generated for terrain
    m_breadTextureId = m_terrainTexture;

    // generate procedural normal map for crust detail
    const int nmWidth = 512;
    const int nmHeight = 512;
    std::vector<unsigned char> normalMapData(nmWidth * nmHeight * 3);

    for (int y = 0; y < nmHeight; y++) {
        for (int x = 0; x < nmWidth; x++) {
            int idx = (y * nmWidth + x) * 3;

            // create bumpy crust pattern with noise
            float bumpScale = 0.15f;
            float bumpNoise = fbm(x * 0.05f, y * 0.05f, 42, 3);
            float crustBump = fbm(x * 0.1f, y * 0.1f, 99, 2) * bumpScale;

            // normal pointing mostly up (0,0,1) with slight variation
            float nx = (crustBump - 0.5f) * 0.4f;
            float ny = (bumpNoise - 0.5f) * 0.4f;
            float nz = 1.0f;

            // normalize
            float len = std::sqrt(nx*nx + ny*ny + nz*nz);
            nx /= len;
            ny /= len;
            nz /= len;

            // convert to 0-255 range (normal maps store normals as RGB)
            normalMapData[idx + 0] = (unsigned char)((nx * 0.5f + 0.5f) * 255);
            normalMapData[idx + 1] = (unsigned char)((ny * 0.5f + 0.5f) * 255);
            normalMapData[idx + 2] = (unsigned char)((nz * 0.5f + 0.5f) * 255);
        }
    }

    glGenTextures(1, &m_testNormalMapId);
    glBindTexture(GL_TEXTURE_2D, m_testNormalMapId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, nmWidth, nmHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, normalMapData.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);

    std::cout << "Generated procedural crust normal map: " << nmWidth << "x" << nmHeight << std::endl;

    // setup instancing if enabled
    if (settings.enableInstancing) {
        m_instanceManager.generateInstances(100, 15.0f);
        m_instanceManager.uploadToGPU();
        m_shapeManager.setupInstanceAttributes(PrimitiveType::PRIMITIVE_CUBE,
                                                m_instanceManager.getInstanceVBO());
        std::cout << "generated " << m_instanceManager.getInstanceCount()
                  << " instances for cube" << std::endl;
    }

    update();
}

void Realtime::settingsChanged() {
    if (!m_initialized) {
        return;
    }

    makeCurrent();

    if (m_camera) {
        m_camera->updateClippingPlanes(settings.nearPlane, settings.farPlane);
    }

    m_shapeManager.updateTessellation(settings.shapeParameter1, settings.shapeParameter2);

    update();
}

void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown && m_camera) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        const float sensitivity = 0.005f;

        float yawAngle = -deltaX * sensitivity;
        m_camera->rotateAroundWorldY(yawAngle);

        float pitchAngle = -deltaY * sensitivity;
        m_camera->rotateAroundRightVector(pitchAngle);

        update();
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // update elapsed time for animations
    m_elapsedTime += deltaTime;

    if (!m_camera) {
        return;
    }

    const float speed = 5.0f * deltaTime;

    if (m_keyMap[Qt::Key_W]) {
        m_camera->translateForward(speed);
    }
    if (m_keyMap[Qt::Key_S]) {
        m_camera->translateBackward(speed);
    }
    if (m_keyMap[Qt::Key_A]) {
        m_camera->translateLeft(speed);
    }
    if (m_keyMap[Qt::Key_D]) {
        m_camera->translateRight(speed);
    }
    if (m_keyMap[Qt::Key_Space]) {
        m_camera->translateUp(speed);
    }
    if (m_keyMap[Qt::Key_Shift]) {
        m_camera->translateDown(speed);
    }

    update();
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.flipped(Qt::Vertical); // flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}

void Realtime::renderSceneToShadowMap() {
    glUseProgram(m_shadowShaderProgram);

    GLint lightSpaceLoc = glGetUniformLocation(m_shadowShaderProgram, "lightSpaceMatrix");
    glUniformMatrix4fv(lightSpaceLoc, 1, GL_FALSE, &m_lightSpaceMatrix[0][0]);

    // render shapes to shadow map (baguettes!)
    for (const RenderShapeData& shape : m_renderData.shapes) {
        GLint modelLoc = glGetUniformLocation(m_shadowShaderProgram, "modelMatrix");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &shape.ctm[0][0]);

        GLuint vao = m_shapeManager.getVAO(shape.primitive.type);
        int vertexCount = m_shapeManager.getVertexCount(shape.primitive.type);

        if (vao != 0 && vertexCount > 0) {
            glBindVertexArray(vao);
            glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        }
    }

    // render terrain to shadow map
    if (settings.enableTerrain && m_terrainGenerated && m_terrainVAO != 0) {
        glm::mat4 terrainModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -5.0f, 0.0f));
        GLint modelLoc = glGetUniformLocation(m_shadowShaderProgram, "modelMatrix");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &terrainModel[0][0]);

        glBindVertexArray(m_terrainVAO);
        glDrawArrays(GL_TRIANGLES, 0, m_terrainVertexCount);
    }

    // render baguettes to shadow map
    if (m_baguetteVAO != 0) {
        GLint modelLoc = glGetUniformLocation(m_shadowShaderProgram, "modelMatrix");
        for (const auto& transform : m_baguetteTransforms) {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &transform[0][0]);

            glBindVertexArray(m_baguetteVAO);
            glDrawArrays(GL_TRIANGLES, 0, m_baguetteVertexCount);
        }
    }

    // render loaves to shadow map
    if (m_loafVAO != 0) {
        GLint modelLoc = glGetUniformLocation(m_shadowShaderProgram, "modelMatrix");
        for (const auto& transform : m_loafTransforms) {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &transform[0][0]);

            glBindVertexArray(m_loafVAO);
            glDrawArrays(GL_TRIANGLES, 0, m_loafVertexCount);
        }
    }

    glBindVertexArray(0);
}

void Realtime::renderSceneNormal() {
    m_shaderManager.use();
    setGlobalUniforms();

    m_shaderManager.setUniformInt("enableShadows", settings.enableShadows ? 1 : 0);
    m_shaderManager.setUniformMat4("lightSpaceMatrix", m_lightSpaceMatrix);
    m_shaderManager.setUniformFloat("shadowBias", settings.shadowBias);

    if (m_shadowShaderProgram != 0) {
        m_shadowManager.bindForReading(1);
        m_shaderManager.setUniformInt("shadowMap", 1);
    }

    if (m_terrainTexture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_terrainTexture);
        m_shaderManager.setUniformInt("diffuseTexture", 0);
    }

    m_shaderManager.setUniformInt("enableFog", settings.enableFog ? 1 : 0);
    m_shaderManager.setUniformInt("enableNormalMapping", settings.enableNormalMapping ? 1 : 0);
    m_shaderManager.setUniformInt("enableScrolling", settings.enableScrolling ? 1 : 0);
    m_shaderManager.setUniformInt("hasDiffuseTexture", 0);
    m_shaderManager.setUniformInt("hasNormalMap", 0);
    m_shaderManager.setUniformVec3("fogColor", settings.fogColor);
    m_shaderManager.setUniformFloat("fogStart", settings.fogStart);
    m_shaderManager.setUniformFloat("fogEnd", settings.fogEnd);
    m_shaderManager.setUniformFloat("fogDensity", settings.fogDensity);
    m_shaderManager.setUniformFloat("time", 0.0f);

    // render shapes in shadow pass (baguettes!)
    for (const RenderShapeData& shape : m_renderData.shapes) {
        renderShape(shape);
    }

    if (settings.enableTerrain) {
        renderTerrain();
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

void Realtime::renderTerrain() {
    if (!m_terrainGenerated || m_terrainVAO == 0) {
        return;
    }

    // disable instancing for terrain
    m_shaderManager.setUniformBool("useInstancing", false);

    // super bright orange/white bread terrain
    glm::vec4 breadAmbient(1.0f, 0.85f, 0.55f, 1.0f);   // strong orange ambient
    glm::vec4 breadDiffuse(1.0f, 0.85f, 0.5f, 1.0f);    // vibrant orange
    glm::vec4 breadSpecular(0.5f, 0.45f, 0.4f, 1.0f);
    float breadShininess = 8.0f;

    // center terrain at origin with slight y offset
    glm::mat4 terrainModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -5.0f, 0.0f));
    m_shaderManager.setUniformMat4("modelMatrix", terrainModel);
    m_shaderManager.setUniformVec4("ambientColor", breadAmbient);
    m_shaderManager.setUniformVec4("diffuseColor", breadDiffuse);
    m_shaderManager.setUniformVec4("specularColor", breadSpecular);
    m_shaderManager.setUniformFloat("shininess", breadShininess);

    if (settings.enableShadows && m_shadowShaderProgram != 0) {
        m_shadowManager.bindForReading(1);
        m_shaderManager.setUniformInt("shadowMap", 1);
    }

    if (m_terrainTexture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_terrainTexture);
        m_shaderManager.setUniformInt("diffuseTexture", 0);
        m_shaderManager.setUniformInt("hasDiffuseTexture", 1);
    } else {
        m_shaderManager.setUniformInt("hasDiffuseTexture", 0);
    }

    glBindVertexArray(m_terrainVAO);
    glDrawArrays(GL_TRIANGLES, 0, m_terrainVertexCount);
    glBindVertexArray(0);

    m_shaderManager.setUniformInt("hasDiffuseTexture", 0);
}

void Realtime::generateTerrainVAO() {
    if (m_terrainVAO != 0) {
        glDeleteVertexArrays(1, &m_terrainVAO);
        glDeleteBuffers(1, &m_terrainVBO);
    }

    const std::vector<float>& vertexData = m_terrainGenerator.getVertexData();

    if (vertexData.empty()) {
        std::cerr << "No terrain data to upload!" << std::endl;
        return;
    }

    glGenVertexArrays(1, &m_terrainVAO);
    glGenBuffers(1, &m_terrainVBO);

    glBindVertexArray(m_terrainVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_terrainVBO);

    glBufferData(GL_ARRAY_BUFFER,
                 vertexData.size() * sizeof(float),
                 vertexData.data(),
                 GL_STATIC_DRAW);

    // position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          14 * sizeof(float),
                          (void*)0);

    // normal (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          14 * sizeof(float),
                          (void*)(3 * sizeof(float)));

    // uv (location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          14 * sizeof(float),
                          (void*)(6 * sizeof(float)));

    // tangent (location 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE,
                          14 * sizeof(float),
                          (void*)(8 * sizeof(float)));

    // bitangent (location 4)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE,
                          14 * sizeof(float),
                          (void*)(11 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_terrainVertexCount = vertexData.size() / 14;
    m_terrainGenerated = true;

    std::cout << "Terrain VAO created with " << m_terrainVertexCount << " vertices (14-float format)" << std::endl;
}

void Realtime::regenerateTerrain() {
    makeCurrent();

    std::cout << "Regenerating terrain with scale=" << settings.terrainScale
              << ", height=" << settings.terrainHeightScale
              << ", detail=" << settings.terrainDetailLevel << std::endl;

    m_terrainGenerator.generate(
        500,
        500,
        settings.terrainScale,
        settings.terrainScale,
        settings.terrainHeightScale,
        (int)settings.terrainDetailLevel,
        0.5f,
        2.0f
    );

    generateTerrainVAO();
    update();
}

void Realtime::generateBaguetteVAO() {
    if (m_baguetteVAO != 0) {
        glDeleteVertexArrays(1, &m_baguetteVAO);
        glDeleteBuffers(1, &m_baguetteVBO);
    }

    const std::vector<float>& vertexData = m_baguette.getVertexData();

    if (vertexData.empty()) {
        std::cerr << "No baguette data to upload!" << std::endl;
        return;
    }

    glGenVertexArrays(1, &m_baguetteVAO);
    glGenBuffers(1, &m_baguetteVBO);

    glBindVertexArray(m_baguetteVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_baguetteVBO);

    glBufferData(GL_ARRAY_BUFFER,
                 vertexData.size() * sizeof(float),
                 vertexData.data(),
                 GL_STATIC_DRAW);

    // position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          14 * sizeof(float),
                          (void*)0);

    // normal (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          14 * sizeof(float),
                          (void*)(3 * sizeof(float)));

    // uv (location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          14 * sizeof(float),
                          (void*)(6 * sizeof(float)));

    // tangent (location 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE,
                          14 * sizeof(float),
                          (void*)(8 * sizeof(float)));

    // bitangent (location 4)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE,
                          14 * sizeof(float),
                          (void*)(11 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_baguetteVertexCount = vertexData.size() / 14;

    std::cout << "Baguette VAO created with " << m_baguetteVertexCount << " vertices (14-float format)" << std::endl;

    // generate random baguette positions scattered across the terrain like trees
    m_baguetteTransforms.clear();

    int numBaguettes = 150;  // scatter 150 baguettes across the landscape (was 80)
    float terrainSize = 200.0f;  // match terrain scale from settings

    for (int i = 0; i < numBaguettes; i++) {
        // random position across terrain
        float x = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * terrainSize * 0.9f;
        float z = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * terrainSize * 0.9f;

        // random rotation around vertical axis
        float yRotation = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;

        // random larger tilt from vertical for more dramatic angles
        float tiltAngle = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.8f;  // bigger angle variation
        float tiltDirection = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;

        // life-sized baguette scale (roughly 3-4 feet long)
        float scale = 3.5f;

        // position sticking way out of ground (mostly above ground)
        // terrain is at Y=-5.0, baguette is ~1.6 units long * scale = ~5.6 units tall
        // to have ~92% above ground: terrain_y + (height * 0.92)
        float yPos = -5.0f + (1.6f * scale * 0.92f);  // 92% above, only 8% below terrain

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(x, yPos, z));
        transform = glm::rotate(transform, yRotation, glm::vec3(0, 1, 0));
        transform = glm::rotate(transform, -(float)M_PI / 2.0f, glm::vec3(1, 0, 0));  // rotate to vertical (pointing up)
        transform = glm::rotate(transform, tiltAngle, glm::vec3(std::cos(tiltDirection), 0, std::sin(tiltDirection)));
        transform = glm::scale(transform, glm::vec3(scale, scale, scale));

        m_baguetteTransforms.push_back(transform);
    }

    std::cout << "Generated " << numBaguettes << " random baguette positions" << std::endl;
}

void Realtime::generateLoafVAO() {
    if (m_loafVAO != 0) {
        glDeleteVertexArrays(1, &m_loafVAO);
        glDeleteBuffers(1, &m_loafVBO);
    }

    const std::vector<float>& vertexData = m_loaf.getVertexData();

    if (vertexData.empty()) {
        std::cerr << "No loaf data to upload!" << std::endl;
        return;
    }

    glGenVertexArrays(1, &m_loafVAO);
    glGenBuffers(1, &m_loafVBO);

    glBindVertexArray(m_loafVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_loafVBO);

    glBufferData(GL_ARRAY_BUFFER,
                 vertexData.size() * sizeof(float),
                 vertexData.data(),
                 GL_STATIC_DRAW);

    // position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          14 * sizeof(float),
                          (void*)0);

    // normal (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          14 * sizeof(float),
                          (void*)(3 * sizeof(float)));

    // uv (location 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          14 * sizeof(float),
                          (void*)(6 * sizeof(float)));

    // tangent (location 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE,
                          14 * sizeof(float),
                          (void*)(8 * sizeof(float)));

    // bitangent (location 4)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE,
                          14 * sizeof(float),
                          (void*)(11 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_loafVertexCount = vertexData.size() / 14;

    std::cout << "Loaf VAO created with " << m_loafVertexCount << " vertices (14-float format)" << std::endl;

    // generate random loaf positions scattered across the terrain
    m_loafTransforms.clear();

    int numLoaves = 60;  // scatter 60 loaves across the landscape
    float terrainSize = 200.0f;  // match terrain scale from settings

    for (int i = 0; i < numLoaves; i++) {
        // random position across terrain
        float x = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * terrainSize * 0.9f;
        float z = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * terrainSize * 0.9f;

        // random rotation
        float yRotation = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;

        // life-sized loaf scale (a bit smaller than baguettes)
        float scale = 2.5f + (static_cast<float>(rand()) / RAND_MAX) * 1.0f;  // varying sizes

        // position floating in the sky like clouds
        float yPos = 15.0f + (static_cast<float>(rand()) / RAND_MAX) * 10.0f;  // floating between Y=15 and Y=25

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(x, yPos, z));
        transform = glm::rotate(transform, yRotation, glm::vec3(0, 1, 0));
        transform = glm::rotate(transform, (float)M_PI / 2.0f, glm::vec3(0, 0, 1));  // rotate on side
        transform = glm::scale(transform, glm::vec3(scale, scale, scale));

        m_loafTransforms.push_back(transform);
    }

    std::cout << "Generated " << numLoaves << " random loaf positions" << std::endl;
}

void Realtime::renderBaguettes() {
    if (m_baguetteVAO == 0) {
        return;
    }

    // disable instancing for baguettes
    m_shaderManager.setUniformBool("useInstancing", false);

    // baguette material - darker, redder crusty bread
    glm::vec4 baguetteAmbient(0.75f, 0.5f, 0.3f, 1.0f);   // darker red/brown
    glm::vec4 baguetteDiffuse(1.0f, 0.65f, 0.35f, 1.0f);  // red/orange crust
    glm::vec4 baguetteSpecular(0.4f, 0.3f, 0.2f, 1.0f);
    float baguetteShininess = 12.0f;

    m_shaderManager.setUniformVec4("ambientColor", baguetteAmbient);
    m_shaderManager.setUniformVec4("diffuseColor", baguetteDiffuse);
    m_shaderManager.setUniformVec4("specularColor", baguetteSpecular);
    m_shaderManager.setUniformFloat("shininess", baguetteShininess);

    // bind bread texture
    if (m_breadTextureId != 0) {
        m_textureManager.bindTexture(m_breadTextureId, GL_TEXTURE0);
        m_shaderManager.setUniformInt("diffuseTexture", 0);
        m_shaderManager.setUniformBool("hasDiffuseTexture", true);
    } else {
        m_shaderManager.setUniformBool("hasDiffuseTexture", false);
    }

    // bind normal map for crusty texture
    if (m_testNormalMapId != 0) {
        m_textureManager.bindTexture(m_testNormalMapId, GL_TEXTURE1);
        m_shaderManager.setUniformInt("normalMap", 1);
        m_shaderManager.setUniformBool("hasNormalMap", true);
        m_shaderManager.setUniformFloat("normalMapIntensity", 1.5f);  // enhanced crust detail
    } else {
        m_shaderManager.setUniformBool("hasNormalMap", false);
    }

    // render all scattered baguettes across the terrain
    for (const auto& transform : m_baguetteTransforms) {
        m_shaderManager.setUniformMat4("modelMatrix", transform);

        glBindVertexArray(m_baguetteVAO);
        glDrawArrays(GL_TRIANGLES, 0, m_baguetteVertexCount);
        glBindVertexArray(0);
    }

    m_shaderManager.setUniformBool("hasDiffuseTexture", false);
    m_shaderManager.setUniformBool("hasNormalMap", false);
}

void Realtime::renderLoaves() {
    if (m_loafVAO == 0) {
        return;
    }

    // disable instancing for loaves
    m_shaderManager.setUniformBool("useInstancing", false);

    // loaf material - darker, redder crusty bread
    glm::vec4 loafAmbient(0.7f, 0.48f, 0.28f, 1.0f);  // darker red/brown
    glm::vec4 loafDiffuse(1.0f, 0.62f, 0.32f, 1.0f);  // red/orange crust
    glm::vec4 loafSpecular(0.35f, 0.28f, 0.18f, 1.0f);
    float loafShininess = 10.0f;

    m_shaderManager.setUniformVec4("ambientColor", loafAmbient);
    m_shaderManager.setUniformVec4("diffuseColor", loafDiffuse);
    m_shaderManager.setUniformVec4("specularColor", loafSpecular);
    m_shaderManager.setUniformFloat("shininess", loafShininess);

    // bind bread texture
    if (m_breadTextureId != 0) {
        m_textureManager.bindTexture(m_breadTextureId, GL_TEXTURE0);
        m_shaderManager.setUniformInt("diffuseTexture", 0);
        m_shaderManager.setUniformBool("hasDiffuseTexture", true);
    } else {
        m_shaderManager.setUniformBool("hasDiffuseTexture", false);
    }

    // bind normal map for crusty texture
    if (m_testNormalMapId != 0) {
        m_textureManager.bindTexture(m_testNormalMapId, GL_TEXTURE1);
        m_shaderManager.setUniformInt("normalMap", 1);
        m_shaderManager.setUniformBool("hasNormalMap", true);
        m_shaderManager.setUniformFloat("normalMapIntensity", 1.2f);  // subtle crust detail
    } else {
        m_shaderManager.setUniformBool("hasNormalMap", false);
    }

    // render all scattered loaves across the terrain
    for (const auto& transform : m_loafTransforms) {
        m_shaderManager.setUniformMat4("modelMatrix", transform);

        glBindVertexArray(m_loafVAO);
        glDrawArrays(GL_TRIANGLES, 0, m_loafVertexCount);
        glBindVertexArray(0);
    }

    m_shaderManager.setUniformBool("hasDiffuseTexture", false);
    m_shaderManager.setUniformBool("hasNormalMap", false);
}

// bloom post-processing implementation

void Realtime::initializeBloom() {
    int width = size().width() * m_devicePixelRatio;
    int height = size().height() * m_devicePixelRatio;

    // create framebuffer for scene rendering
    glGenFramebuffers(1, &m_bloomFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_bloomFBO);

    // create scene color texture
    glGenTextures(1, &m_sceneTexture);
    glBindTexture(GL_TEXTURE_2D, m_sceneTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_sceneTexture, 0);

    // create depth buffer
    glGenRenderbuffers(1, &m_sceneDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_sceneDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_sceneDepthBuffer);

    // check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "error: bloom framebuffer not complete!" << std::endl;
    }

    // create bright pass texture
    glGenTextures(1, &m_brightTexture);
    glBindTexture(GL_TEXTURE_2D, m_brightTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // create ping-pong blur textures
    for (int i = 0; i < 2; i++) {
        glGenTextures(1, &m_blurTexture[i]);
        glBindTexture(GL_TEXTURE_2D, m_blurTexture[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

    // load bloom shaders
    try {
        m_bloomBrightShader = ShaderLoader::createShaderProgram(
            ":/resources/shaders/bloom.vert",
            ":/resources/shaders/bloom_bright.frag"
        );
        m_bloomBlurShader = ShaderLoader::createShaderProgram(
            ":/resources/shaders/bloom.vert",
            ":/resources/shaders/bloom_blur.frag"
        );
        m_bloomCompositeShader = ShaderLoader::createShaderProgram(
            ":/resources/shaders/bloom.vert",
            ":/resources/shaders/bloom_composite.frag"
        );
        std::cout << "bloom shaders loaded successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "failed to load bloom shaders: " << e.what() << std::endl;
        m_bloomBrightShader = 0;
        m_bloomBlurShader = 0;
        m_bloomCompositeShader = 0;
        settings.enableBloom = false;
        return;
    }

    // create fullscreen quad
    float quadVertices[] = {
        // positions   // uvs
        -1.0f,  1.0f,  0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  0.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_fullscreenQuadVAO);
    glGenBuffers(1, &m_fullscreenQuadVBO);
    glBindVertexArray(m_fullscreenQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreenQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
}

void Realtime::cleanupBloom() {
    if (m_bloomFBO != 0) {
        glDeleteFramebuffers(1, &m_bloomFBO);
        m_bloomFBO = 0;
    }
    if (m_sceneTexture != 0) {
        glDeleteTextures(1, &m_sceneTexture);
        m_sceneTexture = 0;
    }
    if (m_sceneDepthBuffer != 0) {
        glDeleteRenderbuffers(1, &m_sceneDepthBuffer);
        m_sceneDepthBuffer = 0;
    }
    if (m_brightTexture != 0) {
        glDeleteTextures(1, &m_brightTexture);
        m_brightTexture = 0;
    }
    for (int i = 0; i < 2; i++) {
        if (m_blurTexture[i] != 0) {
            glDeleteTextures(1, &m_blurTexture[i]);
            m_blurTexture[i] = 0;
        }
    }
    if (m_bloomBrightShader != 0) {
        glDeleteProgram(m_bloomBrightShader);
        m_bloomBrightShader = 0;
    }
    if (m_bloomBlurShader != 0) {
        glDeleteProgram(m_bloomBlurShader);
        m_bloomBlurShader = 0;
    }
    if (m_bloomCompositeShader != 0) {
        glDeleteProgram(m_bloomCompositeShader);
        m_bloomCompositeShader = 0;
    }
    if (m_fullscreenQuadVAO != 0) {
        glDeleteVertexArrays(1, &m_fullscreenQuadVAO);
        m_fullscreenQuadVAO = 0;
    }
    if (m_fullscreenQuadVBO != 0) {
        glDeleteBuffers(1, &m_fullscreenQuadVBO);
        m_fullscreenQuadVBO = 0;
    }
}

void Realtime::renderFullscreenQuad() {
    glBindVertexArray(m_fullscreenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Realtime::renderBloom() {
    // extract bright areas
    glBindFramebuffer(GL_FRAMEBUFFER, m_bloomFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_brightTexture, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_bloomBrightShader);
    glUniform1i(glGetUniformLocation(m_bloomBrightShader, "sceneTexture"), 0);
    glUniform1f(glGetUniformLocation(m_bloomBrightShader, "threshold"), settings.bloomThreshold);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_sceneTexture);

    glDisable(GL_DEPTH_TEST);
    renderFullscreenQuad();

    // blur bright areas with ping-pong passes
    bool horizontal = true;
    GLuint sourceTexture = m_brightTexture;

    glUseProgram(m_bloomBlurShader);
    glUniform1i(glGetUniformLocation(m_bloomBlurShader, "inputTexture"), 0);
    glUniform1f(glGetUniformLocation(m_bloomBlurShader, "blurRadius"), settings.bloomBlurRadius);

    for (int i = 0; i < settings.bloomBlurPasses; i++) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               m_blurTexture[horizontal ? 0 : 1], 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glUniform1i(glGetUniformLocation(m_bloomBlurShader, "horizontal"), horizontal ? 1 : 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sourceTexture);

        renderFullscreenQuad();

        sourceTexture = m_blurTexture[horizontal ? 0 : 1];
        horizontal = !horizontal;
    }

    // composite bloom to intermediate texture (for color grading)
    GLuint outputTarget = settings.enableColorGrading ? m_colorGradingTexture : 0;

    if (outputTarget != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_bloomFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTarget, 0);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_bloomCompositeShader);
    glUniform1i(glGetUniformLocation(m_bloomCompositeShader, "sceneTexture"), 0);
    glUniform1i(glGetUniformLocation(m_bloomCompositeShader, "bloomTexture"), 1);
    glUniform1f(glGetUniformLocation(m_bloomCompositeShader, "bloomIntensity"), settings.bloomIntensity);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_sceneTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, sourceTexture);

    renderFullscreenQuad();

    glEnable(GL_DEPTH_TEST);
}

// color grading implementation

void Realtime::initializeColorGrading() {
    int width = size().width() * m_devicePixelRatio;
    int height = size().height() * m_devicePixelRatio;

    // create texture for color grading output
    glGenTextures(1, &m_colorGradingTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorGradingTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // load color grading shader
    try {
        m_colorGradingShader = ShaderLoader::createShaderProgram(
            ":/resources/shaders/bloom.vert",  // reuse bloom vertex shader
            ":/resources/shaders/color_grading.frag"
        );
        std::cout << "color grading shader loaded successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "failed to load color grading shader: " << e.what() << std::endl;
        m_colorGradingShader = 0;
        settings.enableColorGrading = false;
    }
}

void Realtime::cleanupColorGrading() {
    if (m_colorGradingTexture != 0) {
        glDeleteTextures(1, &m_colorGradingTexture);
        m_colorGradingTexture = 0;
    }
    if (m_colorGradingShader != 0) {
        glDeleteProgram(m_colorGradingShader);
        m_colorGradingShader = 0;
    }
}

void Realtime::applyColorGrading(GLuint inputTexture) {
    if (!settings.enableColorGrading || m_colorGradingShader == 0) {
        return;
    }

    // apply color grading and output to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_colorGradingShader);
    glUniform1i(glGetUniformLocation(m_colorGradingShader, "inputTexture"), 0);
    glUniform1f(glGetUniformLocation(m_colorGradingShader, "exposure"), settings.exposure);
    glUniform1f(glGetUniformLocation(m_colorGradingShader, "contrast"), settings.contrast);
    glUniform1f(glGetUniformLocation(m_colorGradingShader, "saturation"), settings.saturation);
    glUniform1f(glGetUniformLocation(m_colorGradingShader, "temperature"), settings.temperature);
    glUniform3f(glGetUniformLocation(m_colorGradingShader, "tint"),
                settings.tint.r, settings.tint.g, settings.tint.b);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);

    glDisable(GL_DEPTH_TEST);
    renderFullscreenQuad();
    glEnable(GL_DEPTH_TEST);
}
