#version 330 core

in vec2 fragUV;
out vec4 fragColor;

uniform sampler2D sceneTexture;
uniform float threshold;

void main() {
    vec3 color = texture(sceneTexture, fragUV).rgb;

    // calculate brightness using luminance formula
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));

    // extract bright areas above threshold
    if (brightness > threshold) {
        fragColor = vec4(color, 1.0);
    } else {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
