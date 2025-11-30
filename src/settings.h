// settings.h - MODIFIED VERSION
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
    float farPlane = 100.0f;




    
    //proj6 toggle
    bool enableFog = false;
    bool enableNormalMapping = false;
    bool enableScrolling = false;
    bool enableInstancing = false;
    
    //fog
    float fogDensity = 0.05f;
    float fogStart = 10.0f;
    float fogEnd = 50.0f;
    glm::vec3 fogColor = glm::vec3(0.9f, 0.7f, 0.4f); // Bread steam color
    
    //nromal mapping
    float normalMapIntensity = 1.0f;
    
    //scrolling
    float scrollSpeed = 0.5f;
    glm::vec2 scrollDirection = glm::vec2(1.0f, 0.0f);
};

extern Settings settings;

#endif // SETTINGS_H
