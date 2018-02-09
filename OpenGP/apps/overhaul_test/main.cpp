#include <OpenGP/AppTemplates/VirtualDepthSensor.h>

#include <OpenGP/SurfaceMesh/GL/SurfaceMeshDepthmapRenderer.h>
#include <OpenGP/GL/DepthSurfaceRenderer.h>

#define OPENGP_IMPLEMENT_ALL_IN_THIS_FILE
#include <OpenGP/util/implementations.h>


using namespace OpenGP;

class App : public VirtualDepthSensor {
public:

    App() {

        const char *file = "bunny.obj";
        SurfaceMesh mesh;
        bool success = mesh.read(file);
        if(!success) mFatal() << "File not found" << file;
        mesh.triangulate();
        mesh.update_face_normals();
        mesh.update_vertex_normals();

        auto &bunny = scene.create_entity_with<WorldRenderComponent>();
        auto &renderer = bunny.set_renderer<SurfaceMeshRenderer>();
        renderer.upload_mesh(mesh);
        auto &depth_renderer = bunny.require<DepthRenderComponent>().set_renderer<SurfaceMeshDepthmapRenderer>();
        depth_renderer.upload_mesh(mesh);

        auto &depth_surface = scene.create_entity_with<WorldRenderComponent>();
        auto &depth_surface_renderer = depth_surface.set_renderer<DepthSurfaceRenderer>();
        depth_surface_renderer.set_depth_texture(sensor->get_depthmap()->get_depth_texture());
        depth_surface_renderer.set_sensor_matrix(sensor->get<CameraComponent>().get_projection(sensor->width, sensor->height));

        bunny.get<TransformComponent>().position = Vec3(0, 0, 0);
        main_camera->get<TransformComponent>().position = Vec3(1, 1, 1);
        main_camera->get<TrackballComponent>().center = Vec3(0, 0, 0);
        sensor->get<TransformComponent>().position = Vec3(-1, 1, -1);

    }

};

int main(int argc, char **argv) {
    App app;
    return app.run();
}
