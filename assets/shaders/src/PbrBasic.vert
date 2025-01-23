#version 450

layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;

// world-space position-normal
layout(location=0) out vec3 fragViewPos;
layout(location=1) out vec3 fragViewNormal;


// set=1 is required, for some reason
layout(set=1, binding=0) uniform TranformBlock
{
    mat4 modelView;
    mat4 mvp;
    mat3 normalMatrix;
};

void main() {
    fragViewPos = vec3(modelView * vec4(inPosition, 1.0));
    fragViewNormal = normalize(normalMatrix * inNormal);
    gl_Position = mvp * vec4(inPosition, 1.0);
}
