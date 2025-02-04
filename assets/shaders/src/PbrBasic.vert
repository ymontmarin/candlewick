#version 450

layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;

// world-space position-normal
layout(location=0) out vec3 fragViewPos;
layout(location=1) out vec3 fragViewNormal;
layout(location=2) out vec3 fragLightPos;
layout(location=3) out vec4 fragClipPos;


// set=1 is required, for some reason
layout(set=1, binding=0) uniform TranformBlock
{
    mat4 modelView;
    mat4 mvp;
    mat3 normalMatrix;
};

layout(set=1, binding=1) uniform LightBlockV
{
    mat4 lightMvp;
};

void main() {
    vec4 hp = vec4(inPosition, 1.0);
    fragViewPos = vec3(modelView * hp);
    fragViewNormal = normalize(normalMatrix * inNormal);
    gl_Position = mvp * vec4(inPosition, 1.0);
    fragClipPos = gl_Position;

    vec4 flps = lightMvp * hp;
    fragLightPos = flps.xyz / flps.w;
}
