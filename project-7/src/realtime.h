#pragma once

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <memory>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

#include "camera/Camera.h"
#include "shapes/ShapeManager.h"
#include "rendering/ShaderManager.h"
#include "rendering/TextureManager.h"
#include "rendering/InstanceManager.h"
#include "rendering/ShadowManager.h"
#include "terrain/TerrainGenerator.h"
#include "shapes/Baguette.h"
#include "shapes/Loaf.h"
#include "utils/sceneparser.h"

class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();
    void sceneChanged();
    void settingsChanged();
    void saveViewportImage(std::string filePath);
    void regenerateTerrain();

public slots:
    void tick(QTimerEvent* event);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

private:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    void setGlobalUniforms();
    void renderShape(const RenderShapeData& shape);
    void renderSceneToShadowMap();
    void renderSceneNormal();
    void renderTerrain();
    void generateTerrainVAO();
    void renderBaguettes();
    void generateBaguetteVAO();
    void renderLoaves();
    void generateLoafVAO();

    int m_timer;
    QElapsedTimer m_elapsedTimer;

    bool m_mouseDown = false;
    glm::vec2 m_prev_mouse_pos;
    std::unordered_map<Qt::Key, bool> m_keyMap;

    double m_devicePixelRatio;

    RenderData m_renderData;
    bool m_sceneLoaded = false;
    bool m_initialized = false;

    std::unique_ptr<Camera> m_camera;
    ShapeManager m_shapeManager;
    ShaderManager m_shaderManager;
    TextureManager m_textureManager;
    InstanceManager m_instanceManager;

    GLuint m_testNormalMapId = 0;  // test normal map texture
    GLuint m_defaultWhiteTexture = 0;  // default 1x1 white texture
    GLuint m_breadTextureId = 0;  // bread diffuse texture

    float m_elapsedTime = 0.0f;  // total elapsed time for animations

    ShadowManager m_shadowManager;
    GLuint m_shadowShaderProgram;
    glm::mat4 m_lightSpaceMatrix;

    TerrainGenerator m_terrainGenerator;
    GLuint m_terrainVAO;
    GLuint m_terrainVBO;
    int m_terrainVertexCount;
    bool m_terrainGenerated;
    GLuint m_terrainTexture;
    GLuint m_terrainNormalMap;

    Baguette m_baguette;
    GLuint m_baguetteVAO;
    GLuint m_baguetteVBO;
    int m_baguetteVertexCount;
    std::vector<glm::mat4> m_baguetteTransforms;

    Loaf m_loaf;
    GLuint m_loafVAO;
    GLuint m_loafVBO;
    int m_loafVertexCount;
    std::vector<glm::mat4> m_loafTransforms;

    // bloom post-processing
    GLuint m_bloomFBO;
    GLuint m_sceneTexture;
    GLuint m_sceneDepthBuffer;
    GLuint m_brightTexture;
    GLuint m_blurTexture[2];  // ping-pong buffers for blur
    GLuint m_bloomBrightShader;
    GLuint m_bloomBlurShader;
    GLuint m_bloomCompositeShader;
    GLuint m_fullscreenQuadVAO;
    GLuint m_fullscreenQuadVBO;

    // color grading post-processing
    GLuint m_colorGradingShader;
    GLuint m_colorGradingTexture;  // intermediate texture for color grading

    void initializeBloom();
    void cleanupBloom();
    void renderBloom();
    void renderFullscreenQuad();
    void initializeColorGrading();
    void cleanupColorGrading();
    void applyColorGrading(GLuint inputTexture);
};
