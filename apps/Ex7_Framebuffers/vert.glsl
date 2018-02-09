#version 330 core

uniform mat4 model;
uniform mat4 model_rotation;
uniform mat4 MVP;

in vec3 vposition;

in vec3 vnormal;

out vec3 fposition;
out vec3 fnormal;


void main() {
    gl_Position = MVP * vec4(vposition, 1);

    fposition = (model * vec4(vposition, 1)).xyz;
    fnormal = (model_rotation * vec4(vnormal, 1)).xyz;
}