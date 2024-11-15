#version 450

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;

layout(location=0) out vec3 f_normal;

// set=1 is required, for some god forsaken reason
// i am not that smart
layout(set=1, binding=0) uniform UniformBlock
{
    mat4 viewProj;
    mat3 normalMatrix;
};

void main() {

    f_normal = normalMatrix * normal;
    gl_Position = viewProj * vec4(position, 1.0);

}
