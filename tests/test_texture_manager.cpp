// automated tests for texture manager
// verifies texture loading and caching work correctly

#include <iostream>
#include <vector>
#include "../src/rendering/TextureManager.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <QApplication>
#include <QOffscreenSurface>
#include <QOpenGLContext>

struct TestResult {
    std::string testName;
    bool passed;
    std::string message;
};

std::vector<TestResult> results;

// helper to initialize opengl context for testing
bool initializeGLContext(QOpenGLContext*& context, QOffscreenSurface*& surface) {
    // create opengl context
    context = new QOpenGLContext();
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    context->setFormat(format);

    if (!context->create()) {
        std::cerr << "failed to create opengl context" << std::endl;
        return false;
    }

    // create offscreen surface
    surface = new QOffscreenSurface();
    surface->setFormat(format);
    surface->create();

    if (!surface->isValid()) {
        std::cerr << "failed to create offscreen surface" << std::endl;
        return false;
    }

    // make context current
    context->makeCurrent(surface);

    // initialize glew
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "glew initialization failed: " << glewGetErrorString(err) << std::endl;
        return false;
    }

    return true;
}

// test that texture manager can load a valid texture
void testTextureLoading(TextureManager& manager) {
    std::string texturePath = "/Users/fluffy/projects/cs1230/bread-final/resources/textures/test_normal.png";

    GLuint textureId = manager.loadTexture(texturePath);

    bool passed = (textureId != 0);
    results.push_back({
        "TextureManager load valid texture",
        passed,
        passed ? "texture id = " + std::to_string(textureId) : "failed to load texture"
    });
}

// test that texture manager returns 0 for invalid path
void testInvalidTexture(TextureManager& manager) {
    std::string invalidPath = "/nonexistent/path/to/texture.png";

    GLuint textureId = manager.loadTexture(invalidPath);

    bool passed = (textureId == 0);
    results.push_back({
        "TextureManager invalid texture path",
        passed,
        passed ? "correctly returned 0" : "should return 0 for invalid path"
    });
}

// test that texture manager caches textures
void testTextureCaching(TextureManager& manager) {
    std::string texturePath = "/Users/fluffy/projects/cs1230/bread-final/resources/textures/test_normal.png";

    GLuint firstId = manager.loadTexture(texturePath);
    GLuint secondId = manager.loadTexture(texturePath);

    bool passed = (firstId == secondId && firstId != 0);
    results.push_back({
        "TextureManager caching",
        passed,
        passed ? "same texture id returned: " + std::to_string(firstId) :
                 "different ids returned: " + std::to_string(firstId) + " vs " + std::to_string(secondId)
    });
}

// test that binding texture doesn't crash
void testTextureBinding(TextureManager& manager) {
    std::string texturePath = "/Users/fluffy/projects/cs1230/bread-final/resources/textures/test_normal.png";

    GLuint textureId = manager.loadTexture(texturePath);

    bool passed = true;
    try {
        manager.bindTexture(textureId, GL_TEXTURE0);
        manager.bindTexture(textureId, GL_TEXTURE1);

        // verify texture is bound by checking opengl state
        GLint boundTexture = 0;
        glActiveTexture(GL_TEXTURE1);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);

        passed = (boundTexture == (GLint)textureId);
    } catch (...) {
        passed = false;
    }

    results.push_back({
        "TextureManager binding",
        passed,
        passed ? "texture bound successfully" : "texture binding failed"
    });
}

int main(int argc, char *argv[]) {
    // need qapplication for qt image loading
    QApplication app(argc, argv);

    std::cout << "=== running texture manager automated tests ===" << std::endl;
    std::cout << std::endl;

    // initialize opengl context
    QOpenGLContext* context = nullptr;
    QOffscreenSurface* surface = nullptr;

    if (!initializeGLContext(context, surface)) {
        std::cerr << "failed to initialize opengl context for testing" << std::endl;
        return 1;
    }

    std::cout << "opengl context initialized successfully" << std::endl;
    std::cout << std::endl;

    // create texture manager
    TextureManager manager;

    // run tests
    testTextureLoading(manager);
    testInvalidTexture(manager);
    testTextureCaching(manager);
    testTextureBinding(manager);

    // cleanup
    manager.cleanup();

    // print results
    int passCount = 0;
    int failCount = 0;

    for (const auto& result : results) {
        if (result.passed) {
            std::cout << "[PASS] " << result.testName << ": " << result.message << std::endl;
            passCount++;
        } else {
            std::cout << "[FAIL] " << result.testName << ": " << result.message << std::endl;
            failCount++;
        }
    }

    std::cout << std::endl;
    std::cout << "=== test summary ===" << std::endl;
    std::cout << "passed: " << passCount << std::endl;
    std::cout << "failed: " << failCount << std::endl;

    // cleanup context
    context->doneCurrent();
    delete surface;
    delete context;

    return failCount > 0 ? 1 : 0;
}
