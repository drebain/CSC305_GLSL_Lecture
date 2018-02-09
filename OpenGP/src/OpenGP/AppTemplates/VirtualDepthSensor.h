// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <OpenGP/GL/Application.h>
#include <OpenGP/GL/Scene.h>
#include <OpenGP/GL/Window.h>
#include <OpenGP/GL/SyntheticDepthmap.h>
#include <OpenGP/SurfaceMesh/GL/SurfaceMeshRenderer.h>
#include <OpenGP/RGBD/Stream.h>

#include <OpenGP/GL/Components/TrackballComponent.h>
#include <OpenGP/GL/Components/GUICanvasComponent.h>
#include <OpenGP/GL/Components/GizmoComponent.h>
#include <OpenGP/GL/Components/DepthRenderComponent.h>

//=============================================================================
namespace OpenGP {
//=============================================================================

class DepthSensorComponent : public Component {
private:

    std::unique_ptr<SyntheticDepthmap> depthmap;
    std::unique_ptr<Framebuffer> color;
    std::unique_ptr<RGB8Texture> color_texture;

    FullscreenQuad fsq;

    Image<uint16_t> depth_buffer;
    void *depth_data;
    Image<Eigen::Matrix<uint8_t, 3, 1>> color_buffer;
    void *color_data;

public:

    int width = 640, height = 480;

    void init() {
        require<GizmoComponent>();
        require<CameraComponent>();
        auto &renderer = require<WorldRenderComponent>().set_renderer<SurfaceMeshRenderer>();
        GPUMesh &mesh = renderer.get_gpu_mesh();

        std::vector<Vec3> vpoint;

        vpoint.push_back(Vec3(0, 0, 0));
        vpoint.push_back(Vec3(1, 1, 1));
        vpoint.push_back(Vec3(1, -1, 1));
        vpoint.push_back(Vec3(-1, -1, 1));
        vpoint.push_back(Vec3(-1, 1, 1));

        std::vector<unsigned int> triangles;

        triangles.insert(triangles.end(), { 0, 1, 2 });
        triangles.insert(triangles.end(), { 0, 2, 3 });
        triangles.insert(triangles.end(), { 0, 3, 4 });
        triangles.insert(triangles.end(), { 0, 4, 1 });

        mesh.set_vpoint(vpoint);
        mesh.set_triangles(triangles);

        renderer.wirecolor = Vec3(1, 1, 1);
        renderer.wireframe_mode = WireframeMode::WiresOnly;

        init_framebuffers();

    }

    void init_framebuffers() {
        depthmap.reset();
        depthmap = std::unique_ptr<SyntheticDepthmap>(new SyntheticDepthmap(width, height));

        depth_buffer.resize(height, width);
        depth_data = depth_buffer.data();

        color_texture.reset();
        color_texture = std::unique_ptr<RGB8Texture>(new RGB8Texture());
        color_texture->allocate(width, height);

        color_buffer.resize(height, width);
        color_data = color_buffer.data();

        color.reset();
        color = std::unique_ptr<Framebuffer>(new Framebuffer());
        color->attach_color_texture(*color_texture);
        color->attach_depth_texture(depthmap->get_depth_texture());
    }

    void update() {

        auto &transform = get<TransformComponent>();
        auto &camera = get<CameraComponent>();

        float aspect = (float)width / (float)height;
        float y = std::tan(camera.vfov * M_PI / 360);
        transform.set_scale(0.2 * Vec3(y * aspect * 1.01, y * 1.01, 1));

        depthmap->clear();

        Mat4x4 view = camera.get_view();
        Mat4x4 projection = camera.get_projection(width, height);

        for (auto &drc : get_scene().all_of_type<DepthRenderComponent>()) {

            auto &renderer = drc.get_renderer();

            Mat4x4 model = drc.get<TransformComponent>().get_transformation_matrix();

            depthmap->render(renderer, model, view, projection);

        }

        color->bind();

        get<CameraComponent>().draw(width, height);

        color->unbind();

    }

    void draw_depthmap() {

        fsq.draw_texture(depthmap->get_texture());

    }

    SyntheticDepthmap* get_depthmap() {
        return depthmap.get();
    }

    void draw_color() {

        fsq.draw_texture(*color_texture);

    }

    RGB8Texture* get_color_texture() {
        return color_texture.get();
    }

    SensorDevice get_device() {

        StreamIntrinsics color_intrinsics, depth_intrinsics; // TODO
        StreamExtrinsics color_extrinsics, depth_extrinsics; // TODO
        float depth_scale;
        float framerate;

        std::unordered_map<std::string, SensorStream> streams;
        streams.emplace(
            std::piecewise_construct,
            std::forward_as_tuple("COLOR"),
            std::forward_as_tuple("COLOR", &color_data, color_intrinsics, color_extrinsics, framerate)
        );
        streams.emplace(
            std::piecewise_construct,
            std::forward_as_tuple("DEPTH"),
            std::forward_as_tuple("DEPTH", &depth_data, depth_intrinsics, depth_extrinsics, framerate)
        );

        return SensorDevice(depth_scale, streams, [this](bool block){

            get_color_texture()->download(color_buffer);
            get_depthmap()->get_depth_texture().download(depth_buffer);

            return true;

        });

    }

};

class VirtualDepthSensor {
protected:

    // Must be first
    Application app;

    Scene scene;

    CameraComponent *main_camera;

    DepthSensorComponent *sensor;

    GUICanvasComponent *canvas;

    Window *scene_window;
    Window *depthmap_window;

public:

    bool display_color = false;

    VirtualDepthSensor() {

        app.add_listener<ApplicationUpdateEvent>([&](const ApplicationUpdateEvent&){ scene.update(); });

        main_camera = &scene.create_entity_with<TrackballComponent>().get<CameraComponent>();

        sensor = &scene.create_entity_with<DepthSensorComponent>();
        auto *gizmo = &sensor->get<GizmoComponent>();

        canvas = &scene.create_entity_with<GUICanvasComponent>();
        canvas->add_listener<GUIElementDrawEvent>([this](const GUIElementDrawEvent &e){ on_gui(e); });
        canvas->add_listener<GUIElementDrawEvent>([gizmo](const GUIElementDrawEvent &e){ gizmo->on_gui(e); });

        scene_window = &app.create_window([&](Window &window){
            main_camera->draw();
        });
        scene_window->add_listener<WindowCloseEvent>([&](const WindowCloseEvent &){ app.close(); });
        scene_window->set_title("Scene");

        main_camera->set_window(*scene_window);
        canvas->set_camera(*main_camera);

        depthmap_window = &app.create_window([&](Window &window){
            if (display_color) {
                sensor->draw_color();
            } else {
                sensor->draw_depthmap();
            }
        });
        depthmap_window->add_listener<WindowCloseEvent>([&](const WindowCloseEvent &){ app.close(); });
        depthmap_window->add_listener<KeyEvent>([this](const KeyEvent &e){
            if (e.key == GLFW_KEY_C && e.released) display_color = !display_color;
        });
        depthmap_window->set_title("Sensor Output");

    }

    ~VirtualDepthSensor() {}

    void on_gui(const GUIElementDrawEvent &event) {

        ImGui::BeginMainMenuBar();
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Quit"))
                app.close();
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();

    }

    int run() {
        return app.run();
    }

};

//=============================================================================
} // OpenGP::
//=============================================================================
