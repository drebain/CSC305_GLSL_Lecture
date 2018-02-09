#version 330 core

uniform sampler2D tex;

in vec2 uv;
out vec4 fcolor;

void main() {
    // Here we read from the texture at coordinates (u, v)
    fcolor = texture(tex, uv).rgba;
}