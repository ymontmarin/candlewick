#version 450
#extension GL_GOOGLE_include_directive : enable

#include "tone_mapping.glsl"

layout(location=0) in vec3 fragWorldPos;
layout(location=1) in vec3 fragNormal;

// Light structure
struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

struct PbrMaterial {
    vec4 baseColor;
    float metalness;
    float roughness;
    float ao;
};

// set=3 is required, see SDL3's documentation for SDL_CreateGPUShader
// https://wiki.libsdl.org/SDL3/SDL_CreateGPUShader
layout (set=3, binding=0) uniform Material {
    // material diffuse color
    PbrMaterial material;
};

layout(set=3, binding=1) uniform LightBlock {
    DirectionalLight light;
    vec3 viewPos;
};

layout(location=0) out vec4 fragColor;

// Constants
const float PI = 3.14159265359;

// Schlick's Fresnel approximation
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float distributionGGX(vec3 normal, vec3 H, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(normal, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// Geometry function (Smith's method with Schlick-GGX)
float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float geometrySmith(vec3 normal, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(normal, V), 0.0);
    float NdotL = max(dot(normal, L), 0.0);
    float ggx2  = geometrySchlickGGX(NdotV, roughness);
    float ggx1  = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

void main() {
    vec3 lightDir = normalize(-light.direction);
    vec3 normal = normalize(fragNormal);
    vec3 V = normalize(viewPos - fragWorldPos);
    vec3 H = normalize(lightDir + V);

    if (!gl_FrontFacing) {
        // Flip normal for back faces
        normal = -normal;
    }
    vec3 lightCol = light.intensity * light.color;

    // Base reflectivity
    vec3 F0 = vec3(0.04); // Standard base reflectivity
    F0 = mix(F0, material.baseColor.rgb, material.metalness);

    // Cook-Torrance BRDF
    float NDF = distributionGGX(normal, H, material.roughness);
    float G   = geometrySmith(normal, V, lightDir, material.roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    // Specular and diffuse components
    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, lightDir), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;

    // Energy conservation: diffuse and specular
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    // Only non-metallic surfaces have diffuse lighting
    kD *= 1.0 - material.metalness;

    // Combine lighting (no attenuation for directional light)
    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 Lo = (kD * material.baseColor.rgb / PI + specular) * lightCol * NdotL;

    // Ambient term (very simple)
    vec3 ambient = vec3(0.03) * material.baseColor.rgb * material.ao;

    // Final color
    vec3 color = ambient + Lo;

    // Tone mapping and gamma correction
    color = uncharted2ToneMapping(color);
    color = pow(color, vec3(1.0/2.2));

    // Output
    fragColor = vec4(color, material.baseColor.a);
}
