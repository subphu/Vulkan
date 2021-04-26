#version 450
#extension GL_ARB_separate_shader_objects : enable

// Buffers ==================================================
layout(set = 1, binding = 0) buffer outputBuffer {
    vec4 imageData[];
};

layout(set = 1, binding = 1) uniform Misc {
    vec3 camera;
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
void main() {
    const float PI = 3.141592653589793;
    
//    uint x = uint(fragTexCoord.x * buffSize);
//    uint y = uint(fragTexCoord.y * buffSize);
//    uint idx = y * buffSize + x;
//    outColor = imageData[idx];
    
    vec3 lightDir = normalize( camera * 5.0 - fragPosition );
    float angle   = dot(lightDir, normalize( fragNormal ));
    float scale   = acos(angle) / PI * 2.0;

    uint x   = uint(buffSize * scale);
    uint y   = uint(buffSize * 0.5);
    uint idx = uint(y * buffSize + x);

    outColor = imageData[idx];
    
//    vec3 lightDir = normalize( u_camPos * 5.0 - v_FragPos );
//    float angle   = dot(lightDir, normalize( v_Normal ));
//    float scale   = acos(angle) / PI * 2.0;
//
//    int x = int(scale * u_texSize.x);
//    int y = int(u_thickness * u_texSize.y);
//    int index = int(y * u_texSize.x + x);
//
//    fragColor = imageData[index];
    
//    outColor = texture(albedoMap, fragTexCoord);
}
