#version 330 core

in vec2 fragUV;
out vec4 fragColor;

uniform sampler2D inputTexture;
uniform float exposure;
uniform float contrast;
uniform float saturation;
uniform float temperature;
uniform vec3 tint;

// convert rgb to luminance
float luminance(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

// adjust saturation
vec3 adjustSaturation(vec3 color, float sat) {
    float lum = luminance(color);
    return mix(vec3(lum), color, sat);
}

// apply temperature adjustment (warm/cool)
vec3 adjustTemperature(vec3 color, float temp) {
    // warm (positive) adds red/yellow, cool (negative) adds blue
    vec3 warm = vec3(1.0 + temp * 0.3, 1.0 + temp * 0.15, 1.0 - temp * 0.2);
    return color * warm;
}

void main() {
    vec3 color = texture(inputTexture, fragUV).rgb;

    // apply exposure
    color *= pow(2.0, exposure);

    // apply contrast (centered around 0.5)
    color = (color - 0.5) * contrast + 0.5;

    // apply saturation
    color = adjustSaturation(color, saturation);

    // apply temperature
    color = adjustTemperature(color, temperature);

    // apply tint
    color *= tint;

    // clamp to valid range
    color = clamp(color, 0.0, 1.0);

    fragColor = vec4(color, 1.0);
}
