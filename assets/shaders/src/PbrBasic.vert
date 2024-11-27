#version 450

layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;

// world-space position-normal
layout(location=0) out vec3 fragWorldPos;
layout(location=1) out vec3 fragWorldNormal;


// set=1 is required, for some reason
layout(set=1, binding=0) uniform TranformBlock
{
    mat4 model;
    mat4 mvp;
    mat3 normalMatrix;
};

void main() {
    fragWorldPos = vec3(model * vec4(inPosition, 1.0));
    fragWorldNormal = normalize(normalMatrix * inNormal);
    gl_Position = mvp * vec4(inPosition, 1.0);
}
