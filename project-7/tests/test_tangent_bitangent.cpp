// automated tests for tangent and bitangent calculations
// verifies orthogonality, normalization, and handedness

#include <iostream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>

// include shape classes to test their vertex generation
#include "../src/shapes/Cube.h"
#include "../src/shapes/Sphere.h"
#include "../src/shapes/Cylinder.h"
#include "../src/shapes/Cone.h"

// test result tracking
struct TestResult {
    std::string testName;
    bool passed;
    std::string message;
};

std::vector<TestResult> results;

// tolerance for floating point comparisons
const float EPSILON = 0.001f;

bool nearEqual(float a, float b, float epsilon = EPSILON) {
    return std::abs(a - b) < epsilon;
}

bool nearZero(float value, float epsilon = EPSILON) {
    return std::abs(value) < epsilon;
}

// extract vertex data from shape and verify tbn properties
void testTBNOrthogonality(const std::string& shapeName, const std::vector<float>& vertexData) {
    int vertexCount = vertexData.size() / 14;
    bool allPassed = true;
    std::string failMessage = "";

    for (int i = 0; i < vertexCount; i++) {
        int offset = i * 14;

        // extract normal, tangent, bitangent
        glm::vec3 N(vertexData[offset + 3], vertexData[offset + 4], vertexData[offset + 5]);
        glm::vec3 T(vertexData[offset + 8], vertexData[offset + 9], vertexData[offset + 10]);
        glm::vec3 B(vertexData[offset + 11], vertexData[offset + 12], vertexData[offset + 13]);

        // test 1: vectors should be normalized
        float nLen = glm::length(N);
        float tLen = glm::length(T);
        float bLen = glm::length(B);

        if (!nearEqual(nLen, 1.0f)) {
            allPassed = false;
            failMessage = "normal not normalized at vertex " + std::to_string(i) +
                         " (length = " + std::to_string(nLen) + ")";
            break;
        }
        if (!nearEqual(tLen, 1.0f)) {
            allPassed = false;
            failMessage = "tangent not normalized at vertex " + std::to_string(i) +
                         " (length = " + std::to_string(tLen) + ")";
            break;
        }
        if (!nearEqual(bLen, 1.0f)) {
            allPassed = false;
            failMessage = "bitangent not normalized at vertex " + std::to_string(i) +
                         " (length = " + std::to_string(bLen) + ")";
            break;
        }

        // test 2: vectors should be orthogonal
        float dotTN = glm::dot(T, N);
        float dotBN = glm::dot(B, N);
        float dotTB = glm::dot(T, B);

        if (!nearZero(dotTN)) {
            allPassed = false;
            failMessage = "tangent not orthogonal to normal at vertex " + std::to_string(i) +
                         " (dot = " + std::to_string(dotTN) + ")";
            break;
        }
        if (!nearZero(dotBN)) {
            allPassed = false;
            failMessage = "bitangent not orthogonal to normal at vertex " + std::to_string(i) +
                         " (dot = " + std::to_string(dotBN) + ")";
            break;
        }
        if (!nearZero(dotTB)) {
            allPassed = false;
            failMessage = "tangent not orthogonal to bitangent at vertex " + std::to_string(i) +
                         " (dot = " + std::to_string(dotTB) + ")";
            break;
        }
    }

    results.push_back({
        shapeName + " TBN orthogonality and normalization",
        allPassed,
        allPassed ? "all " + std::to_string(vertexCount) + " vertices passed" : failMessage
    });
}

// test that TBN forms a right-handed coordinate system
void testTBNHandedness(const std::string& shapeName, const std::vector<float>& vertexData) {
    int vertexCount = vertexData.size() / 14;
    bool allPassed = true;
    std::string failMessage = "";

    for (int i = 0; i < vertexCount; i++) {
        int offset = i * 14;

        glm::vec3 N(vertexData[offset + 3], vertexData[offset + 4], vertexData[offset + 5]);
        glm::vec3 T(vertexData[offset + 8], vertexData[offset + 9], vertexData[offset + 10]);
        glm::vec3 B(vertexData[offset + 11], vertexData[offset + 12], vertexData[offset + 13]);

        // cross product of T and B should be approximately N or -N
        glm::vec3 crossTB = glm::cross(T, B);
        float dotWithN = glm::dot(crossTB, N);

        // should be close to 1.0 (right-handed) or -1.0 (left-handed)
        // but should be consistent
        if (!nearEqual(std::abs(dotWithN), 1.0f, 0.1f)) {
            allPassed = false;
            failMessage = "TBN does not form proper coordinate system at vertex " +
                         std::to_string(i) + " (cross(T,B) · N = " + std::to_string(dotWithN) + ")";
            break;
        }
    }

    results.push_back({
        shapeName + " TBN handedness",
        allPassed,
        allPassed ? "all " + std::to_string(vertexCount) + " vertices passed" : failMessage
    });
}

// test cube vertices
void testCube() {
    Cube cube;
    cube.updateParams(2);  // simple tessellation
    const std::vector<float>& vertexData = cube.generateShape();

    testTBNOrthogonality("Cube", vertexData);
    testTBNHandedness("Cube", vertexData);
}

// test sphere vertices
void testSphere() {
    Sphere sphere;
    sphere.updateParams(5, 5);  // low tessellation for speed
    const std::vector<float>& vertexData = sphere.generateShape();

    testTBNOrthogonality("Sphere", vertexData);
    testTBNHandedness("Sphere", vertexData);
}

// test cylinder vertices
void testCylinder() {
    Cylinder cylinder;
    cylinder.updateParams(5, 5);  // low tessellation for speed
    const std::vector<float>& vertexData = cylinder.generateShape();

    testTBNOrthogonality("Cylinder", vertexData);
    testTBNHandedness("Cylinder", vertexData);
}

// test cone vertices
void testCone() {
    Cone cone;
    cone.updateParams(5, 5);  // low tessellation for speed
    const std::vector<float>& vertexData = cone.generateShape();

    testTBNOrthogonality("Cone", vertexData);
    testTBNHandedness("Cone", vertexData);
}

int main() {
    std::cout << "=== running tangent/bitangent automated tests ===" << std::endl;
    std::cout << std::endl;

    testCube();
    testSphere();
    testCylinder();
    testCone();

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

    return failCount > 0 ? 1 : 0;
}
