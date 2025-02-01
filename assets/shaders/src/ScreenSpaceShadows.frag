#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 0) out float fragShadow;

// Depth texture from pre-pass
layout(set=2, binding=0) uniform sampler2D depthTexture;

// Parameters for the effect
layout(set=3, binding=0) uniform ShadowParams {
    // Camera projection matrix
    mat4 projection;
    // Inverse projection matrix
    mat4 invProjection;
    // View-space light direction
    vec3 lightDir;
    // Maximum ray march distance
    float maxDistance;
    // Ray march step size
    float stepSize;
    // Maximum number of steps
    int numSteps;
    // Depth comparison bias
    float bias;
};

// Reconstruct view-space position from depth
vec3 reconstructViewPos(vec2 uv, float depth) {
    // Convert to NDC space
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth, 1.0);

    // Convert to view space
    vec4 viewPos = invProjection * ndc;
    return viewPos.xyz / viewPos.w;
}

void main() {
    // Sample depth
    float depth = texture(depthTexture, fragUV).r;

    // Early out if this is the far plane
    if (depth >= 1.0) {
        fragShadow = 1.0;
        return;
    }

    // Reconstruct view-space position
    vec3 viewPos = reconstructViewPos(fragUV, depth);

    // Ray march towards light
    vec3 rayStep = normalize(-lightDir) * stepSize;
    vec3 rayPos = viewPos;

    for (int i = 0; i < numSteps; i++) {
        // Advance ray
        rayPos += rayStep;

        // Project current position to screen space
        vec4 projectedPos = projection * vec4(rayPos, 1.0);
        vec3 screenPos = projectedPos.xyz / projectedPos.w;

        // Check if we're still in screen space
        if (abs(screenPos.x) > 1.0 || abs(screenPos.y) > 1.0) {
            break;
        }

        // Convert to UV coordinates
        vec2 sampleUV = 0.5 + screenPos.xy * 0.5;

        // Sample depth at current position
        float sampledDepth = texture(depthTexture, sampleUV).r;

        // Convert ray position to NDC depth
        float rayDepth = screenPos.z - bias;

        // Compare depths
        if (sampledDepth < rayDepth) {
            fragShadow = 0.0;
            // return;
        }

        // Check distance
        if (length(rayPos - viewPos) > maxDistance) {
            break;
        }
    }

    // No shadow found
    fragShadow = 1.0;
}
