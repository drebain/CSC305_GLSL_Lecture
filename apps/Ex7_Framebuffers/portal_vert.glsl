#version 330 core

uniform mat4 MVP;

in vec3 vposition;
out vec2 uv;

void main() {
    gl_Position = MVP * vec4(vposition, 1);
    uv = vec2((vposition.x + 1) / 2, (vposition.y + 1) / 2);
}