#version 330 core

// Here we define the input and output primitive types for this geometry shader
//
// In this case we are taking in single points, and for each point outputting a
// triangle strip with up to 4 vertices (two triangles)
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec2 g_uv[];
out vec2 uv;

// This function runs once for every primitive (i.e. triangles, lines, points)
//
// 
void main() {
    gl_Position = gl_in[0].gl_Position + vec4(-0.05, -0.05, 0.0, 0.0);
    uv = g_uv[0];
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(-0.05,  0.05, 0.0, 0.0);
    uv = g_uv[0];
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(0.05,  -0.05, 0.0, 0.0);
    uv = g_uv[0];
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4(0.05,   0.05, 0.0, 0.0);
    uv = g_uv[0];
    EmitVertex();

    EndPrimitive();
}