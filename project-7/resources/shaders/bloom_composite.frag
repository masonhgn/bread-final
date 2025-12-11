#version 330 core

in vec2 fragUV;
out vec4 fragColor;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;
uniform float bloomIntensity;

void main() {
    vec3 sceneColor = texture(sceneTexture, fragUV).rgb;
    vec3 bloomColor = texture(bloomTexture, fragUV).rgb;

    // additive blending with intensity control
    vec3 result = sceneColor + bloomColor * bloomIntensity;

    fragColor = vec4(result, 1.0);
}
