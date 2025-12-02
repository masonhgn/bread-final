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
    float farPlane = 30.0f;




    
    //proj6 toggle
    bool enableFog = true;
    bool enableNormalMapping = true;
    bool enableScrolling = true;
    bool enableInstancing = true;
    
    //fog
    float fogDensity = 0.05f;
    float fogStart = 8.0f;
    float fogEnd = 20.0f;
    glm::vec3 fogColor = glm::vec3(0.8f, 0.85f, 0.9f); // soft blue-gray fog
    
    //nromal mapping
    float normalMapIntensity = 1.0f;
    
    //scrolling
    float scrollSpeed = 0.5f;
    glm::vec2 scrollDirection = glm::vec2(1.0f, 0.0f);
};

extern Settings settings;

#endif // SETTINGS_H
