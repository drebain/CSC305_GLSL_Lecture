#version 330 core

uniform vec3 eye;

uniform vec3 light_dir;
uniform vec3 light_color;
uniform vec3 ambient_light;

uniform vec3 bunny_color;
uniform vec3 bunny_specular;
uniform float specular_power;

in vec3 fposition;
in vec3 fnormal;

out vec4 fcolor;


void main() {
    vec3 normal = normalize(fnormal);

    vec3 view = normalize(eye - fposition);

    float d_weight = max(-dot(light_dir, normal), 0);
    float s_weight = pow(max(dot(reflect(view, normal), light_dir), 0),
                         specular_power);

    vec3 illumination = ambient_light * bunny_color
                      + d_weight * light_color * bunny_color
                      + s_weight * light_color * bunny_specular;

    fcolor = vec4(illumination, 1);
}