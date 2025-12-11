#version 330 core

in vec2 fragUV;
out vec4 fragColor;

uniform sampler2D inputTexture;
uniform bool horizontal;
uniform float blurRadius;

// gaussian blur weights for 5-tap filter
const float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    vec2 texelSize = 1.0 / textureSize(inputTexture, 0);
    vec3 result = texture(inputTexture, fragUV).rgb * weights[0];

    if (horizontal) {
        // horizontal blur
        for(int i = 1; i < 5; ++i) {
            result += texture(inputTexture, fragUV + vec2(texelSize.x * i * blurRadius, 0.0)).rgb * weights[i];
            result += texture(inputTexture, fragUV - vec2(texelSize.x * i * blurRadius, 0.0)).rgb * weights[i];
        }
    } else {
        // vertical blur
        for(int i = 1; i < 5; ++i) {
            result += texture(inputTexture, fragUV + vec2(0.0, texelSize.y * i * blurRadius)).rgb * weights[i];
            result += texture(inputTexture, fragUV - vec2(0.0, texelSize.y * i * blurRadius)).rgb * weights[i];
        }
    }

    fragColor = vec4(result, 1.0);
}
