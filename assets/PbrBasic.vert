#version 450

layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;

layout(location=0) out vec4 fragPosition;
// world-space normal
layout(location=1) out vec3 fragNormal;


// set=1 is required, for some reason
layout(set=1, binding=0) uniform TranformBlock
{
    mat4 modelView;
    mat4 mvp;
    mat3 normalMatrix;
};

void main() {

    // fragPosition = modelView * vec4(inPosition, 1.0);
    fragPosition.xyz = inPosition;
    // fragNormal = normalize(normalMatrix * inNormal);
    fragNormal = normalize(inNormal);
    gl_Position = mvp * vec4(inPosition, 1.0);

}
