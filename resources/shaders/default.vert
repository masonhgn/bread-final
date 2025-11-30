#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragUV;

void main() {
    vec4 worldPosition = modelMatrix * vec4(position, 1.0);
    fragPosition = worldPosition.xyz;
    fragNormal = mat3(transpose(inverse(modelMatrix))) * normal;
    fragUV = uv;
    
    gl_Position = projectionMatrix * viewMatrix * worldPosition;
}