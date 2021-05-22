#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragCubeCoord;

void main() {
    fragCubeCoord = inPosition;
    gl_Position = proj * mat4(mat3(view)) * vec4(inPosition, 1.0);
}

