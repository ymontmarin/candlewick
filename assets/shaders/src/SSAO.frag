#version 450


layout(location=0) in vec2 inUV;
layout(location=0) out float aoValue;

layout(set=2, binding=0) uniform sampler2D depthTex;
layout(set=2, binding=1) uniform sampler2D normalMap;

const int SSAO_KERNEL_SIZE = 64;
const float SSAO_RADIUS = 0.5;
const float SSAO_BIAS = 0.025;
const float SSAO_INTENSITY = 1.0;

layout(set=3, binding=0) uniform SSAOParams {
    vec4 samples[SSAO_KERNEL_SIZE];
} kernel;

layout(set=3, binding=1) uniform Camera {
    mat4 projection;
} camera;

vec3 getViewPos(float depth, vec2 uv) {
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 viewPos = inverse(camera.projection) * clipPos;
    return viewPos.xyz / viewPos.w;
}

vec3 getRandom(vec2 uv) {
    // Generate a random vector from UV
    vec3 rand;
    rand.x = fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
    rand.y = fract(sin(dot(uv, vec2(4.898, 7.23))) * 23421.631);
    rand.z = fract(sin(dot(uv, vec2(4.1239, 9.12))) * 32974.821);
    return normalize(rand * 2.0 - 1.0);
}

float calculatePixelAO(vec2 uv) {
    float depth = texture(depthTex, uv).r;
    vec3 viewPos = getViewPos(depth, uv);
    vec3 viewNormal;
    viewNormal.xy = texture(normalMap, uv).xy;
    viewNormal.z = sqrt(1 - dot(viewNormal.xy, viewNormal.xy));

    vec3 randVec = getRandom(uv);

    // tbn matrix for rotating samples
    vec3 tangent = normalize(randVec - viewNormal * dot(randVec, viewNormal));
    vec3 bitangent = cross(viewNormal, tangent);
    mat3 TBN = mat3(tangent, bitangent, viewNormal);

    // accumulate occlusion
    float occlusion = 0.0;
    for(int i = 0; i < SSAO_KERNEL_SIZE; i++) {
        // get sample position
        vec3 samplePos = TBN * kernel.samples[i].xyz; // Rotate sample vector
        samplePos = viewPos + samplePos * SSAO_RADIUS;         // Move it to view-space position

        // project sample to get its screen-space coordinates
        vec4 offset = camera.projection * vec4(samplePos, 1.0);
        offset.xy /= offset.w;                            // Perspective divide
        offset.xy = offset.xy * 0.5 + 0.5;               // Transform to [0,1] range

        // get sample depth
        float sampleDepth = texture(depthTex, offset.xy).r;

        // convert sample's depth to view space for comparison
        vec3 sampleViewPos = getViewPos(sampleDepth, offset.xy);

        // Rarnge check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, SSAO_RADIUS / abs(viewPos.z - sampleViewPos.z));
        occlusion += (sampleViewPos.z >= samplePos.z + SSAO_BIAS ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / 64.0) * SSAO_INTENSITY;
    return occlusion;
}

void main() {
    float center = calculatePixelAO(inUV);
    aoValue = center;
}
