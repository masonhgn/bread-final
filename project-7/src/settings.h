#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <glm/glm.hpp> 

struct Settings {
    std::string sceneFilePath;
    
    //tesselation
    int shapeParameter1 = 5;
    int shapeParameter2 = 5;
    
    //camera
    float nearPlane = 0.1f;
    float farPlane = 500.0f;  // increased for large terrain

    //project 6 feature toggles
    bool enableFog = true;
    bool enableNormalMapping = true;
    bool enableScrolling = true;
    bool enableInstancing = true;

    //bloom (mason's feature)
    bool enableBloom = true;
    float bloomThreshold = 0.8f;
    float bloomIntensity = 0.5f;
    float bloomBlurRadius = 1.0f;
    int bloomBlurPasses = 5;

    //color grading (mason's feature)
    bool enableColorGrading = true;
    float exposure = 0.0f;          // -2 to 2, 0 is neutral
    float contrast = 1.0f;          // 0 to 2, 1 is neutral
    float saturation = 1.0f;        // 0 to 2, 1 is neutral
    float temperature = 0.0f;       // -1 to 1, 0 is neutral
    glm::vec3 tint = glm::vec3(1.0f, 1.0f, 1.0f);  // rgb tint multiplier

    //shadows (sai's feature)
    bool enableShadows = true;
    float shadowBias = 0.005f;
    float shadowIntensity = 0.8f;

    //terrain (sai's feature)
    bool enableTerrain = true;
    int terrainGridSize = 500;
    float terrainScale = 200.0f;
    float terrainHeightScale = 8.0f;
    float terrainDetailLevel = 4.0f;
    
    //fog
    float fogDensity = 0.05f;
    float fogStart = 50.0f;   // adjusted for large terrain
    float fogEnd = 300.0f;    // adjusted for large terrain
    glm::vec3 fogColor = glm::vec3(0.8f, 0.85f, 0.9f); // soft blue-gray fog

    //normal mapping
    float normalMapIntensity = 1.0f;
    
    //scrolling
    float scrollSpeed = 0.5f;
    glm::vec2 scrollDirection = glm::vec2(1.0f, 0.0f);
};

extern Settings settings;

#endif // SETTINGS_H
