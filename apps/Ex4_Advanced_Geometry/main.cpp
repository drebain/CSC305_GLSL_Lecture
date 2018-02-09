#include <fstream>

#include <OpenGP/GL/Application.h>


using namespace OpenGP;

constexpr int width = 1024;
constexpr int height = 768;

std::unique_ptr<Shader> shader;
std::unique_ptr<GPUMesh> mesh;

float t = 0.0f;

void draw() {

    glViewport(0, 0, width, height);

    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    shader->bind();

    Mat4x4 model;
    model << cos(t),  sin(t), 0, 0,
             -sin(t), cos(t), 0, 0,
             0,       0,      1, 0,
             0,       0,      0, 1;

    shader->set_uniform("model", model);

    mesh->set_attributes(*shader);

    mesh->draw();

    shader->unbind();

}

void update() { t += 0.0001f; }

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
        Vec3(-0.5, -0.5, 0),
        Vec3(-0.5,  0.5, 0),
        Vec3(0.5,  -0.5, 0),
        Vec3(0.5,   0.5, 0)
    };

    mesh->set_vbo<Vec3>("vposition", vposition);

    // Now we are going to use individual point primitives. The indices are
    // specified the same as for triangles, but will be interpreted differently
    std::vector<unsigned int> triangle_indices = {
        0, 1, 2, 3
    };

    // Store the point indices in the mesh object
    mesh->set_triangles(triangle_indices);

    // This time set the mesh mode to GL_POINTS (it defaults to GL_TRIANGLES)
    mesh->set_mode(GL_POINTS);

    // Now include the gshader as well
    shader->add_vshader_from_source(load_source("vert.glsl").c_str());
    shader->add_fshader_from_source(load_source("frag.glsl").c_str());
    shader->add_gshader_from_source(load_source("geom.glsl").c_str());
    shader->link();

    app.add_listener<ApplicationUpdateEvent>([](const ApplicationUpdateEvent&){
        update();
    });

    Window& window = app.create_window([](Window&){ draw(); });
    window.set_title("Geometry Shader Example");
    window.set_size(width, height);

    return app.run();

}
