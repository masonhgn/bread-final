#version 330 core

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragUV;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec4 fragPositionLightSpace;

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
uniform bool enableShadows;

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

// shadow parameters
uniform sampler2D shadowMap;
uniform float shadowBias;

// simple noise function for butter rivers
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// determine if this fragment is in a butter river
float isButterRiver(vec3 worldPos) {
    // create winding rivers using multiple octaves of noise
    float riverNoise = 0.0;
    riverNoise += noise(worldPos.xz * 0.02) * 1.0;
    riverNoise += noise(worldPos.xz * 0.04 + vec2(10.0, 20.0)) * 0.5;

    // create river bands - rivers appear where noise is in certain range
    float river1 = smoothstep(0.45, 0.50, riverNoise) - smoothstep(0.50, 0.55, riverNoise);
    float river2 = smoothstep(0.70, 0.75, riverNoise) - smoothstep(0.75, 0.80, riverNoise);
    float river3 = smoothstep(0.20, 0.25, riverNoise) - smoothstep(0.25, 0.30, riverNoise);

    return clamp(river1 + river2 + river3, 0.0, 1.0);
}

float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    if (!enableShadows) {
        return 0.0;
    }

    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // check if outside shadow map bounds
    if (projCoords.z > 1.0 || projCoords.z < 0.0 ||
        projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0) {
        return 0.0;
    }

    float currentDepth = projCoords.z;

    float bias = shadowBias;

    // pcf (percentage closer filtering) for soft shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias) > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

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

    // apply shadow factor for directional lights
    float shadowFactor = 1.0;
    if (light.type == 0) {  // directional light
        float shadow = calculateShadow(fragPositionLightSpace, normal, lightDir);
        shadowFactor = 1.0 - shadow;
    }

    return attenuation * shadowFactor * (diffuse + specular);
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

    // check if this fragment is in a butter river
    float butterRiverFactor = isButterRiver(fragPosition);

    // apply scrolling only in butter rivers
    vec2 uv = fragUV;
    if (enableScrolling && butterRiverFactor > 0.5) {
        // butter flows very slowly in a specific direction
        uv += vec2(1.0, 0.3) * scrollSpeed * time * 0.05;
    }

    // sample diffuse texture if available
    vec3 texColor = vec3(1.0);
    if (hasDiffuseTexture) {
        texColor = texture(diffuseTexture, uv).rgb;
    }

    // whitish-yellow butter that contrasts with brown bread
    vec3 butterColor = vec3(1.0, 1.0, 0.65);  // bright whitish-yellow butter
    // strong blend to override bread texture completely in rivers
    texColor = mix(texColor, butterColor, butterRiverFactor * butterRiverFactor);

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

    // add extra glossy specular highlights to butter rivers
    if (butterRiverFactor > 0.5) {
        for (int i = 0; i < numLights && i < 8; i++) {
            vec3 lightDir;
            if (lights[i].type == 0) {
                lightDir = normalize(-lights[i].dir);
            } else {
                lightDir = normalize(lights[i].pos - fragPosition);
            }

            // shiny butter specular
            vec3 reflectDir = reflect(-lightDir, normal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 50.0);  // high shininess
            result += spec * vec3(lights[i].color) * butterRiverFactor * 0.5;
        }
    }

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