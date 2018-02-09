#version 330 core

uniform mat4 model;

in vec3 vposition;
out vec2 g_uv;

void main() {
    gl_Position = model * vec4(vposition, 1);
    g_uv = vec2(vposition.x + 0.5, vposition.y + 0.5);
}