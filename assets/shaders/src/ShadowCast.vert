#version 450

layout(location=0) in vec3 inPosition;

struct LightUniform {
    mat4 viewProj;
};

layout(set=1, binding=0) uniform LightingBlock {
    LightUniform light;
};

layout(set=1, binding=1) uniform ModelUniforms {
    mat4 model;
};

void main() {
    gl_Position = light.viewProj * model * vec4(inPosition, 1.0);
}
