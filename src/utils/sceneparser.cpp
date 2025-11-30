#include "sceneparser.h"
#include "scenefilereader.h"
#include <glm/gtx/transform.hpp>
#include <chrono>
#include <iostream>
#include <cmath>

namespace {

glm::mat4 buildTranslateMatrix(const glm::vec3& translate) {
    glm::mat4 T = glm::mat4(1.0f);
    T[3][0] = translate.x;
    T[3][1] = translate.y;
    T[3][2] = translate.z;
    return T;
}

glm::mat4 buildScaleMatrix(const glm::vec3& scale) {
    glm::mat4 S = glm::mat4(1.0f);
    S[0][0] = scale.x;
    S[1][1] = scale.y;
    S[2][2] = scale.z;
    return S;
}

glm::mat4 buildRotateMatrix(const glm::vec3& axis, float angle) {
    glm::vec3 a = glm::normalize(axis);
    float c = cos(angle);
    float s = sin(angle);
    float t = 1.0f - c;

    return glm::mat4(
        t*a.x*a.x + c,     t*a.x*a.y - s*a.z, t*a.x*a.z + s*a.y, 0,
        t*a.x*a.y + s*a.z, t*a.y*a.y + c,     t*a.y*a.z - s*a.x, 0,
        t*a.x*a.z - s*a.y, t*a.y*a.z + s*a.x, t*a.z*a.z + c,     0,
        0,                 0,                 0,                 1
    );
}

glm::mat4 buildTransformMatrix(const SceneTransformation* t) {
    switch (t->type) {
        case TransformationType::TRANSFORMATION_TRANSLATE:
            return buildTranslateMatrix(t->translate);
        case TransformationType::TRANSFORMATION_SCALE:
            return buildScaleMatrix(t->scale);
        case TransformationType::TRANSFORMATION_ROTATE:
            return buildRotateMatrix(t->rotate, t->angle);
        case TransformationType::TRANSFORMATION_MATRIX:
            return t->matrix;
        default:
            return glm::mat4(1.0f);
    }
}

SceneLightData transformLight(const SceneLight* light, const glm::mat4& ctm) {
    SceneLightData transformedLight;

    transformedLight.id = light->id;
    transformedLight.type = light->type;
    transformedLight.color = light->color;
    transformedLight.function = light->function;
    transformedLight.penumbra = light->penumbra;
    transformedLight.angle = light->angle;
    transformedLight.width = light->width;
    transformedLight.height = light->height;

    if (light->type == LightType::LIGHT_POINT || light->type == LightType::LIGHT_SPOT) {
        transformedLight.pos = ctm * glm::vec4(0, 0, 0, 1);
    }

    if (light->type == LightType::LIGHT_DIRECTIONAL || light->type == LightType::LIGHT_SPOT) {
        transformedLight.dir = ctm * light->dir;
        transformedLight.dir = glm::normalize(transformedLight.dir);
    }

    return transformedLight;
}

void traverseSceneGraph(SceneNode* node, const glm::mat4& parentCTM, RenderData& renderData) {
    glm::mat4 currentCTM = parentCTM;

    for (SceneTransformation* transform : node->transformations) {
        glm::mat4 T = buildTransformMatrix(transform);
        currentCTM = currentCTM * T;
    }

    for (ScenePrimitive* primitive : node->primitives) {
        RenderShapeData shapeData;
        shapeData.primitive = *primitive;
        shapeData.ctm = currentCTM;
        renderData.shapes.push_back(shapeData);
    }

    for (SceneLight* light : node->lights) {
        SceneLightData lightData = transformLight(light, currentCTM);
        renderData.lights.push_back(lightData);
    }

    for (SceneNode* child : node->children) {
        traverseSceneGraph(child, currentCTM, renderData);
    }
}

}

bool SceneParser::parse(std::string filepath, RenderData &renderData) {
    ScenefileReader fileReader(filepath);
    bool success = fileReader.readJSON();
    if (!success) {
        return false;
    }

    renderData.globalData = fileReader.getGlobalData();
    renderData.cameraData = fileReader.getCameraData();

    renderData.shapes.clear();
    renderData.lights.clear();

    SceneNode* root = fileReader.getRootNode();
    glm::mat4 identityMatrix = glm::mat4(1.0f);
    traverseSceneGraph(root, identityMatrix, renderData);

    return true;
}
