#version 330 core

uniform sampler2D tex;

in vec2 uv;
out vec4 fcolor;

void main() {
    fcolor = texture(tex, uv).rgba;
}