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

    m_shapeManager.initialize(settings.shapeParameter1, settings.shapeParameter2);

    m_initialized = true;
}

void Realtime::setGlobalUniforms() {
    m_shaderManager.setUniformMat4("viewMatrix", m_camera->getViewMatrix());
    m_shaderManager.setUniformMat4("projectionMatrix", m_camera->getProjectionMatrix());
    m_shaderManager.setUniformVec3("cameraPos", m_camera->getPosition());

    int numLights = std::min(static_cast<int>(m_renderData.lights.size()), 8);
    m_shaderManager.setUniformInt("numLights", numLights);

    for (int i = 0; i < numLights; i++) {
        m_shaderManager.setLight(i, m_renderData.lights[i]);
    }
}

void Realtime::renderShape(const RenderShapeData& shape) {
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

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shaderManager.use();

    setGlobalUniforms();


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



    for (const RenderShapeData& shape : m_renderData.shapes) {
        renderShape(shape);
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
    // Make sure we have the right context and everything has been drawn
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

    // Optional: Create a depth buffer if your rendering uses depth testing
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

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
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
