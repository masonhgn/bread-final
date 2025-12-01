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
    vec3 viewDir = normalize(cameraPos - fragPosition);
    
    // ambient
    vec3 ambient = vec3(ambientColor);
    
    // accumulate lighting from all lights
    vec3 lighting = vec3(0.0);
    for (int i = 0; i < numLights && i < 8; i++) {
        lighting += computeLight(lights[i], normal, viewDir);
    }
    
    vec3 result = ambient + lighting;
    
    // fog stub (person a will implement)
    if (enableFog) {
        // todo: implement fog blending based on distance
        // float distance = length(cameraPos - fragPosition);
        // float fogFactor = ...
        // result = mix(result, fogColor, fogFactor);
    }
    
    // normal mapping stub (person a will implement)
    if (enableNormalMapping && hasNormalMap) {
        // todo: sample normal map and perturb surface normal
    }
    
    // scrolling texture stub (person a will implement)
    if (enableScrolling && hasDiffuseTexture) {
        // todo: offset uv coordinates based on time
        // vec2 scrolledUV = fragUV + scrollDirection * time * scrollSpeed;
    }
    
    fragColor = vec4(result, 1.0);
}