#include <fstream>

#include <OpenGP/GL/Application.h>
#include <OpenGP/Image/Image.h>


using namespace OpenGP;

constexpr int width = 1024;
constexpr int height = 768;

std::unique_ptr<Shader> shader;
std::unique_ptr<GPUMesh> mesh;
std::unique_ptr<RGBA8Texture> texture;

void draw() {

    glViewport(0, 0, width, height);

    glClearColor(0.0f, 0.2f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Enable blending so that we don't ignore the texture alpha channel
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shader->bind();

    // Make texture unit 0 active
    glActiveTexture(GL_TEXTURE0);

    // Bind the texture to the active unit for drawing
    texture->bind();

    // Set the shader's texture uniform to the index of the texture unit we have
    // bound the texture to
    shader->set_uniform("tex", 0);

    mesh->set_attributes(*shader);
    mesh->draw();

    texture->unbind();

    shader->unbind();

}

std::string load_source(const char* fname) {
    std::ifstream f(fname);
    std::stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}

int main(int argc, char** argv) {

    Application app;

    shader = std::unique_ptr<Shader>(new Shader());
    mesh = std::unique_ptr<GPUMesh>(new GPUMesh());

    std::vector<Vec3> vposition = {
        Vec3(-1, -1, 0),
        Vec3(-1,  1, 0),
        Vec3(1,  -1, 0),
        Vec3(1,   1, 0)
    };
    mesh->set_vbo<Vec3>("vposition", vposition);

    std::vector<unsigned int> triangle_indices = {
        0, 1, 2,
        1, 2, 3
    };
    mesh->set_triangles(triangle_indices);

    texture = std::unique_ptr<RGBA8Texture>(new RGBA8Texture());

    // Load the texture from the hard drive
    using RGBA8Pixel = Eigen::Matrix<uint8_t, 4, 1>;
    Image<RGBA8Pixel> im;
    imread("texture.png", im);

    // Upload the texture to the GPU
    texture->upload(im);

    shader->add_vshader_from_source(load_source("vert.glsl").c_str());
    shader->add_fshader_from_source(load_source("frag.glsl").c_str());
    shader->link();

    Window& window = app.create_window([](Window&){ draw(); });
    window.set_title("Minimal Texture Example");
    window.set_size(width, height);

    return app.run();

}
