#pragma once
#include <glm/glm.hpp>
#include "utils/sceneparser.h" 
#include "utils/scenedata.h" 


constexpr float UV_EPSILON = 1e-4f;


glm::vec2 getUVCoords(PrimitiveType type, const glm::vec3 &pOS);
