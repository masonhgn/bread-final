#version 330 core

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragUV;
in vec3 fragTangent;
in vec3 fragBitangent;

out vec4 fragColor;

// light data structure
struct Light {
    int type;
    vec4 color;
    vec3 function;
    vec3 pos;
    vec3 dir;
    float penumbra;
    float angle;
};

// material properties
uniform vec4 ambientColor;
uniform vec4 diffuseColor;
uniform vec4 specularColor;
uniform float shininess;

// lighting
uniform int numLights;
uniform Light lights[8];
uniform vec3 cameraPos;

// project 6 feature toggles
uniform bool enableFog;
uniform bool enableNormalMapping;
uniform bool enableScrolling;

// fog parameters
uniform vec3 fogColor;
uniform float fogStart;
uniform float fogEnd;
uniform float fogDensity;

// texture parameters
uniform sampler2D diffuseTexture;
uniform sampler2D normalMap;
uniform bool hasDiffuseTexture;
uniform bool hasNormalMap;
uniform float time;

// scrolling parameters
uniform vec2 scrollDirection;
uniform float scrollSpeed;

vec3 computeLight(Light light, vec3 normal, vec3 viewDir) {
    vec3 lightDir;
    float attenuation = 1.0;
    
    // directional light
    if (light.type == 0) {
        lightDir = normalize(-light.dir);
    }
    // point or spot light
    else {
        vec3 lightToFrag = light.pos - fragPosition;
        float distance = length(lightToFrag);
        lightDir = normalize(lightToFrag);
        
        // attenuation
        float a = light.function.x;
        float b = light.function.y;
        float c = light.function.z;
        attenuation = min(1.0, 1.0 / (a + b * distance + c * distance * distance));
        
        // spot light cone
        if (light.type == 2) {
            float theta = acos(dot(-lightDir, normalize(light.dir)));
            float outer = light.angle;
            float inner = light.angle - light.penumbra;
            float falloff = clamp((outer - theta) / (outer - inner), 0.0, 1.0);
            attenuation *= falloff;
        }
    }
    
    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(diffuseColor) * vec3(light.color);
    
    // specular
    vec3 halfVec = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfVec), 0.0), shininess);
    vec3 specular = spec * vec3(specularColor) * vec3(light.color);
    
    return attenuation * (diffuse + specular);
}

void main() {
    vec3 normal = normalize(fragNormal);

    if (enableNormalMapping && hasNormalMap) {
        vec3 T = normalize(fragTangent);
        vec3 B = normalize(fragBitangent);
        vec3 N = normalize(fragNormal);
        mat3 TBN = mat3(T, B, N);

        vec3 normalMapSample = texture(normalMap, fragUV).rgb;
        vec3 tangentSpaceNormal = normalMapSample * 2.0 - 1.0;
        normal = normalize(TBN * tangentSpaceNormal);
    }

    vec3 viewDir = normalize(cameraPos - fragPosition);

    vec2 uv = fragUV;
    if (enableScrolling && hasDiffuseTexture) {
        uv += scrollDirection * scrollSpeed * time;
    }

    // sample diffuse texture if available
    vec3 texColor = vec3(1.0);
    if (hasDiffuseTexture) {
        texColor = texture(diffuseTexture, uv).rgb;
    }

    // ambient (modulated by texture)
    vec3 ambient = vec3(ambientColor) * texColor;

    // accumulate lighting from all lights
    vec3 lighting = vec3(0.0);
    for (int i = 0; i < numLights && i < 8; i++) {
        vec3 lightContrib = computeLight(lights[i], normal, viewDir);
        // apply texture to diffuse component only (diffuse is first part of lightContrib)
        lighting += lightContrib * texColor;
    }

    vec3 result = ambient + lighting;

    // apply distance-based fog
    if (enableFog) {
        float distance = length(cameraPos - fragPosition);

        // linear fog: fogFactor = 1.0 (no fog) at fogStart, 0.0 (full fog) at fogEnd
        float fogFactor = (fogEnd - distance) / (fogEnd - fogStart);
        fogFactor = clamp(fogFactor, 0.0, 1.0);

        // blend between fog color and scene color
        result = mix(fogColor, result, fogFactor);
    }

    fragColor = vec4(result, 1.0);
}