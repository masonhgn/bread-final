#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragUV;
out vec3 fragTangent;
out vec3 fragBitangent;

void main() {
    vec4 worldPosition = modelMatrix * vec4(position, 1.0);
    fragPosition = worldPosition.xyz;
    fragNormal = mat3(transpose(inverse(modelMatrix))) * normal;
    fragUV = uv;

    // transform tangent and bitangent to world space (not used yet)
    mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));
    fragTangent = normalMatrix * tangent;
    fragBitangent = normalMatrix * bitangent;

    gl_Position = projectionMatrix * viewMatrix * worldPosition;
}