#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "settings.h"

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
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    m_shapeManager.cleanup();
    m_shaderManager.cleanup();
    m_textureManager.cleanup();
    m_instanceManager.cleanup();

    if (m_defaultWhiteTexture != 0) {
        glDeleteTextures(1, &m_defaultWhiteTexture);
        m_defaultWhiteTexture = 0;
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

    glGenTextures(1, &m_defaultWhiteTexture);
    glBindTexture(GL_TEXTURE_2D, m_defaultWhiteTexture);
    unsigned char whitePixel[4] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_shaderManager.use();
    m_shaderManager.setUniformInt("diffuseTexture", 0);
    m_shaderManager.setUniformInt("normalMap", 1);
    glUseProgram(0);

    m_shapeManager.initialize(settings.shapeParameter1, settings.shapeParameter2);

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



    for (const RenderShapeData& shape : m_renderData.shapes) {
        renderShape(shape);
    }

    if (settings.enableInstancing && m_instanceManager.getInstanceCount() > 0) {
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

    glBindVertexArray(0);
    glUseProgram(0);
}

void Realtime::resizeGL(int w, int h) {
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    if (m_camera) {
        float aspectRatio = static_cast<float>(w) / h;
        m_camera->updateAspectRatio(aspectRatio);
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

    std::string projectRoot = "/Users/fluffy/projects/cs1230/bread-final/";
    std::string normalMapPath = projectRoot + "resources/textures/test_normal.png";
    m_testNormalMapId = m_textureManager.loadTexture(normalMapPath);
    if (m_testNormalMapId == 0) {
        std::cout << "no test normal map found at: " << normalMapPath << std::endl;
    }

    std::string breadTexturePath = projectRoot + "resources/textures/bread.jpg";
    m_breadTextureId = m_textureManager.loadTexture(breadTexturePath);
    if (m_breadTextureId == 0) {
        std::cout << "failed to load bread texture at: " << breadTexturePath << std::endl;
    }

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
    if (m_keyMap[Qt::Key_Control]) {
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
    QImage flippedImage = image.mirrored(); // Flip the image vertically

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
