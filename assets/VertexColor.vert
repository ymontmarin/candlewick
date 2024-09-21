#version 450

layout(location=0)
in vec4 position;

layout(location=1)
in vec4 color;

layout(location=0) out vec4 interpColor;

const mat4 cam_M = mat4(1.0);

void main() {

    gl_Position = cam_M * position;

    interpColor = color;
}
