#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

layout(location = 5) in mat4 instanceMatrix;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 lightSpaceMatrix;
uniform bool useInstancing;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragUV;
out vec3 fragTangent;
out vec3 fragBitangent;
out vec4 fragPositionLightSpace;

void main() {
    mat4 finalModelMatrix = useInstancing ? instanceMatrix : modelMatrix;

    vec4 worldPosition = finalModelMatrix * vec4(position, 1.0);
    fragPosition = worldPosition.xyz;
    fragNormal = mat3(transpose(inverse(finalModelMatrix))) * normal;
    fragUV = uv;

    mat3 normalMatrix = mat3(transpose(inverse(finalModelMatrix)));
    fragTangent = normalMatrix * tangent;
    fragBitangent = normalMatrix * bitangent;

    fragPositionLightSpace = lightSpaceMatrix * worldPosition;

    gl_Position = projectionMatrix * viewMatrix * worldPosition;
}