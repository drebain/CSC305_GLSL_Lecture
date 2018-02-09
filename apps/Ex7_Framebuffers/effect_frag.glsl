#version 330 core

uniform sampler2D tex;

in vec2 uv;
out vec4 fcolor;

// These kernels will be convolved with the rendered image to produce an edge
// detection post-processing effect
//
// Here I am representing a 3x3 matrix as a flat array, but newer versions of
// OpenGL support multidimensional arrays in shaders. I also could use a mat3,
// but some effects may require larger kernels such as 5x5, so arrays are a more
// general solution
float[9] kernel_1 = float[9](
    -1, 0, 1,
    -2, 0, 2,
    -1, 0, 1
);

float[9] kernel_2 = float[9](
    -1, -2, -1,
    0, 0, 0,
    1, 2, 1
);

// This function computes one element of the result of a 2D convolution
float convolve(float[9] kernel) {
    float result = 0;
    for (int i = 0;i < 3;i++) {
        for (int j = 0;j < 3;j++) {
            // The textureOffset function is useful for reading from textures
            // with an exact pixel offset from a particular uv point
            vec3 rgb_val = textureOffset(tex, uv, ivec2(i-1, j-1)).rgb;
            float norm_val = length(rgb_val);
            result += kernel[i + 3 * j] * norm_val;
        }
    }
    return result;
}

void main() {
    vec3 rgb = texture(tex, uv).rgb;

    float edges = 0;
    edges += abs(convolve(kernel_1));
    edges += abs(convolve(kernel_2));

    edges = pow(edges, 2) / 2;

    fcolor = vec4(rgb + edges * vec3(1, 1, 1), 1);
}