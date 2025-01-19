#version 450

layout(location=0) in vec3 inPosition;

layout(set=1, binding=0) uniform CameraBlock {
    mat4 viewProj;
};

layout(set=1, binding=1) uniform ModelUniforms {
    mat4 model;
};

void main() {
    gl_Position = viewProj * model * vec4(inPosition, 1.0);
}
