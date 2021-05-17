#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../functions/constants.glsl"
#include "../functions/interference.glsl"

// Buffers ==================================================
layout(set = 1, binding = 0) buffer outputBuffer {
    vec4 imageData[];
};

layout(set = 1, binding = 1) uniform Misc {
    vec3 viewPosition;
    uint buffSize;
};

// Textures ==================================================
layout(set = 2, binding = 0) uniform sampler2D albedoMap;
layout(set = 2, binding = 1) uniform sampler2D aoMap;
layout(set = 2, binding = 2) uniform sampler2D metallicMap;
layout(set = 2, binding = 3) uniform sampler2D normalMap;
layout(set = 2, binding = 4) uniform sampler2D roughnessMap;

// Inputs ==================================================
layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPosition;

// Outputs ==================================================
layout(location = 0) out vec4 outColor;

// Functions ==================================================
#include "../functions/render_function.glsl"
#include "../functions/pbr.glsl"

void main() {
    
    vec3  N = fragNormal;
    float theta1 = getTheta1(N);
    float theta2 = refractionAngle(n1, theta1, n2);
    float opd    = getOPD(d, theta2, n2);
    
    vec3  color  = vec3(interferences(650e-9, 60e-9, opd),
                        interferences(532e-9, 40e-9, opd),
                        interferences(441e-9, 30e-9, opd));
    
    vec4 pbrColor = vec4(pbr(), 1.0);

    outColor = vec4(color, 1.0);
}