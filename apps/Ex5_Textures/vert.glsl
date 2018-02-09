#version 330 core

in vec3 vposition;
out vec2 uv;

void main() {
    gl_Position = vec4(vposition, 1);
    uv = vec2((vposition.x + 1) / 2, (vposition.y + 1) / 2);
}