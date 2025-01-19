// To be used with DrawQuad.vert
#version 450
#extension GL_GOOGLE_include_directive : enable

#include "depth_utils.glsl"

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout (set=2, binding=0) uniform sampler2D depthTex;

layout (set=3, binding=0) uniform CameraParams {
    int mode;
    float near;
    float far;
};

vec3 visualizeDepthGrayscale(float depth) {
    float lin = linearizeDepth(depth, near, far);
    float norm = (lin - near) / (far - near);
    return vec3(norm);
}

vec3 visualizeDepthHeatmap(float depth) {
    float linear_depth = linearizeDepth(depth, near, far);
    // Normalize to 0-1 range based on near/far planes
    float normalized = (linear_depth - near) / (far - near);

    // Convert to heatmap colors:
    // Blue (cold) -> Cyan -> Green -> Yellow -> Red (hot)
    vec3 color;
    if(normalized < 0.25) {
        // Blue to Cyan
        float t = normalized / 0.25;
        color = mix(vec3(0,0,1), vec3(0,1,1), t);
    } else if(normalized < 0.5) {
        // Cyan to Green
        float t = (normalized - 0.25) / 0.25;
        color = mix(vec3(0,1,1), vec3(0,1,0), t);
    } else if(normalized < 0.75) {
        // Green to Yellow
        float t = (normalized - 0.5) / 0.25;
        color = mix(vec3(0,1,0), vec3(1,1,0), t);
    } else {
        // Yellow to Red
        float t = (normalized - 0.75) / 0.25;
        color = mix(vec3(1,1,0), vec3(1,0,0), t);
    }
    return color;
}

void main() {
    float depth = texture(depthTex, inUV).r;

    vec3 color;
    switch (mode) {
        case 0:
            color = visualizeDepthGrayscale(depth);
            break;
        case 1:
            color = visualizeDepthHeatmap(depth);
            break;
    }
    outColor.rgb = color;
    outColor.w = 1.0;
}
