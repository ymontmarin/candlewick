#version 450

layout(location=0) in vec3 position;
layout(location=1) in vec4 color;

layout(location=0) out vec4 interpColor;


void main() {
    mat4 cam_M = mat4(1.0);

    gl_Position = cam_M * vec4(position, 1.0);

    interpColor = color;
}
