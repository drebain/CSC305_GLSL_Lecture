#include <OpenGP/types.h>
#include <OpenGP/MLogger.h>
#include <OpenGP/GL/GlfwWindow.h>

///--- Mesh rendering
#include <OpenGP/GL/Application.h>
#include <OpenGP/GL/Scene.h>
#include <OpenGP/GL/Components/WorldRenderComponent.h>
#include <OpenGP/GL/Components/TrackballComponent.h>
#include <OpenGP/GL/Components/GUICanvasComponent.h>
#include <OpenGP/SurfaceMesh/SurfaceMesh.h>
#include <OpenGP/SurfaceMesh/GL/SurfaceMeshRenderer.h>
#include <OpenGP/SphereMesh/GL/SphereMeshRenderer.h>
#include <OpenGP/GL/ImguiRenderer.h>
#include <OpenGP/GL/gl_debug.h>

#if 0

#define OPENGP_IMPLEMENT_ALL_IN_THIS_FILE
#include <OpenGP/util/implementations.h>

using namespace OpenGP;
int main(int argc, char** argv){
    std::string file = (argc > 1) ? argv[1] : "bunny.obj";

    SurfaceMesh mesh;
    bool success = mesh.read(file);
    if(!success) mFatal() << "File not found" << file;
    mesh.triangulate();
    mesh.update_face_normals();
    mesh.update_vertex_normals();

    std::string file2 = (argc > 2) ? argv[2] : "example.smo";

    SphereMesh mesh2;
    success = mesh2.read(file2);
    if(!success) mFatal() << "File not found" << file2;

    Application app;

    Scene scene;

    app.set_update_callback([&](){scene.update();});

    auto &canvas = scene.create_entity_with<GUICanvasComponent>();

    auto &obj = scene.create_entity_with<WorldRenderComponent>();
    auto &renderer = obj.set_renderer<SurfaceMeshRenderer>();
    renderer.upload_mesh(mesh);

    auto &obj2 = scene.create_entity_with<WorldRenderComponent>();
    auto &renderer2 = obj2.set_renderer<SphereMeshRenderer>();
    renderer2.upload_mesh(mesh2);

    auto &trackball = scene.create_entity_with<TrackballComponent>();
    auto &cam = trackball.get<CameraComponent>();
    trackball.center = Vec3(0, 0, 0);
    cam.get<TransformComponent>().position = Vec3(3, 3, -10);

    auto &window = app.create_window([&](Window &window){
        cam.draw();
    });

    ImGuizmo::OPERATION op = ImGuizmo::ROTATE;

    canvas.set_action([&](){

        auto &transform = obj.get<TransformComponent>();

        Mat4x4 proj = cam.get_projection();
        Mat4x4 view = cam.get_view();
        Mat4x4 model = transform.get_transformation_matrix();

        ImGuizmo::Manipulate(view.data(), proj.data(), op, ImGuizmo::WORLD, model.data());

        transform.set_transformation_matrix(model);

        ImGui::BeginMainMenuBar();
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Quit"))
                app.close();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Translate"))
                op = ImGuizmo::TRANSLATE;

            if (ImGui::MenuItem("Rotate"))
                op = ImGuizmo::ROTATE;

            if (ImGui::MenuItem("Scale"))
                op = ImGuizmo::SCALE;

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Wireframe")) {
                if (renderer.wireframe_mode == WireframeMode::Overlay) {
                    renderer.wireframe_mode == WireframeMode::None;
                } else {
                    renderer.wireframe_mode == WireframeMode::Overlay;
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            ImGui::MenuItem("There is no help");
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();

        ImGui::Begin("Test Window");
        ImGui::Checkbox("Triangle Mesh", &obj.visible);
        ImGui::Checkbox("Sphere Mesh", &obj2.visible);
        static Vec3 color(1,1,1);
        ImGui::ColorEdit3("Base Color", color.data());
        ImGui::ColorEdit3("Wire Color", renderer.wirecolor.data());
        ImGui::End();

        renderer.get_material().set_property("base_color", color);

    });

    cam.set_window(window);
    canvas.set_camera(cam);

    window.set_title("Test Window");

    return app.run();
}

#endif
