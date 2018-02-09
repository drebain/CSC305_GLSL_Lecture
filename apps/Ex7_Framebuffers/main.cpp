#include <fstream>

#include <OpenGP/GL/Application.h>
#include <OpenGP/GL/Eigen.h>
#include <OpenGP/SurfaceMesh/SurfaceMesh.h>


using namespace OpenGP;

constexpr int width = 1024;
constexpr int height = 768;

std::unique_ptr<Shader> shader;
std::unique_ptr<Shader> portal_shader;
std::unique_ptr<Shader> effect_shader;

std::unique_ptr<GPUMesh> mesh;
std::unique_ptr<GPUMesh> portal_mesh;

// These two textures will store the color result of our drawing operations, and
// allow the frames that we draw to be reused. There are two because we do not
// want to draw into a buffer that we are using as a texture
std::unique_ptr<RGBA8Texture> color_buffer_front;
std::unique_ptr<RGBA8Texture> color_buffer_back;

// We need a depth buffer with the same size as the color buffer so that we can
// do depth testing, which is required to draw a 3d scene
std::unique_ptr<D16Texture> depth_buffer;

// The framebuffer objects will reference the textures above and allow draw
// operations to render into them
std::unique_ptr<Framebuffer> framebuffer_front;
std::unique_ptr<Framebuffer> framebuffer_back;

// We will use this to display the contents of a framebuffer on the screen
// after it has been rendered to
std::unique_ptr<FullscreenQuad> fsquad;

float t = 0.0f;

void draw() {

    glViewport(0, 0, width, height);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind the framebuffer so that we draw to it instead of the window
    framebuffer_front->bind();

    // Note that glClear operates on the currently bound framebuffer only, so
    // we need to do it once for each framebuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader->bind();

    Mat4x4 model = Mat4x4::Identity();

    Vec3 eye = Vec3(5 * cos(t), 2, 5 * sin(t));
    Mat4x4 view = look_at(eye,
                          Vec3(0, 1, 0),
                          Vec3(0, 1, 0));

    float ratio = float(width) / float(height);
    Mat4x4 projection = perspective(60, ratio, 0.1, 50);

    Mat4x4 MVP = projection * view * model;
    Mat4x4 model_rotation = model.inverse().transpose();

    shader->set_uniform("model", model);
    shader->set_uniform("model_rotation", model_rotation);
    shader->set_uniform("MVP", MVP);
    shader->set_uniform("eye", eye);

    Vec3 light_dir = Vec3(cos(2*t), -1, sin(2*t)).normalized();
    shader->set_uniform("light_dir", light_dir);
    shader->set_uniform("light_color", Vec3(1.0, 0.9, 0.8));
    shader->set_uniform("ambient_light", Vec3(0.05, 0.15, 0.35));
    shader->set_uniform("bunny_color", Vec3(0.9, 0.3, 0.1));
    shader->set_uniform("bunny_specular", Vec3(0.6, 0.6, 0.6));
    shader->set_uniform("specular_power", 50.0f);

    mesh->set_attributes(*shader);
    mesh->draw();

    shader->unbind();

    portal_shader->bind();

    model << ratio, 0,       0, 0,
             0,     1/ratio, 0, 1,
             0,     0,       1, -3,
             0,     0,       0, 1;

    MVP = projection * view * model;

    portal_shader->set_uniform("MVP", MVP);

    glActiveTexture(GL_TEXTURE0);
    portal_shader->set_uniform("tex", 0);

    color_buffer_back->bind();

    portal_mesh->set_attributes(*portal_shader);
    portal_mesh->draw();

    color_buffer_back->unbind();

    portal_shader->unbind();

    // And unbind the framebuffer so we can draw to the window again
    framebuffer_front->unbind();

    // Mirror the contents of the front framebuffer onto the window
    fsquad->draw_texture(*color_buffer_front, *effect_shader);

    std::swap(framebuffer_front, framebuffer_back);
    std::swap(color_buffer_front, color_buffer_back);

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
    portal_shader = std::unique_ptr<Shader>(new Shader());
    effect_shader = std::unique_ptr<Shader>(new Shader());
    mesh = std::unique_ptr<GPUMesh>(new GPUMesh());
    portal_mesh = std::unique_ptr<GPUMesh>(new GPUMesh());
    fsquad = std::unique_ptr<FullscreenQuad>(new FullscreenQuad());

    SurfaceMesh mesh_data;
    mesh_data.read("bunny.obj");
    mesh_data.update_vertex_normals();

    mesh->init_from_mesh(mesh_data);

    std::vector<Vec3> portal_vposition = {
        Vec3(-1, -1, 0),
        Vec3(-1,  1, 0),
        Vec3(1,  -1, 0),
        Vec3(1,   1, 0)
    };

    portal_mesh->set_vbo<Vec3>("vposition", portal_vposition);

    std::vector<unsigned int> triangle_indices = {
        0, 1, 2,
        1, 2, 3
    };

    portal_mesh->set_triangles(triangle_indices);

    color_buffer_front = std::unique_ptr<RGBA8Texture>(new RGBA8Texture());
    color_buffer_back = std::unique_ptr<RGBA8Texture>(new RGBA8Texture());
    depth_buffer = std::unique_ptr<D16Texture>(new D16Texture());

    // We are not loading the textures, so we need to tell OpenGL their size
    color_buffer_front->allocate(width, height);
    color_buffer_back->allocate(width, height);
    depth_buffer->allocate(width, height);

    framebuffer_front = std::unique_ptr<Framebuffer>(new Framebuffer());
    framebuffer_back = std::unique_ptr<Framebuffer>(new Framebuffer());

    // Now we need to attach the textures to the framebuffer objects
    framebuffer_front->attach_color_texture(*color_buffer_front);
    framebuffer_front->attach_depth_texture(*depth_buffer);
    framebuffer_back->attach_color_texture(*color_buffer_back);
    framebuffer_back->attach_depth_texture(*depth_buffer);

    shader->add_vshader_from_source(load_source("vert.glsl").c_str());
    shader->add_fshader_from_source(load_source("frag.glsl").c_str());
    shader->link();

    portal_shader->add_vshader_from_source(load_source("portal_vert.glsl").c_str());
    portal_shader->add_fshader_from_source(load_source("portal_frag.glsl").c_str());
    portal_shader->link();

    effect_shader->add_vshader_from_source(load_source("effect_vert.glsl").c_str());
    effect_shader->add_fshader_from_source(load_source("effect_frag.glsl").c_str());
    effect_shader->link();

    app.add_listener<ApplicationUpdateEvent>([](const ApplicationUpdateEvent&){
        update();
    });

    Window& window = app.create_window([](Window&){ draw(); });
    window.set_title("Framebuffers Example");
    window.set_size(width, height);

    return app.run();

}
