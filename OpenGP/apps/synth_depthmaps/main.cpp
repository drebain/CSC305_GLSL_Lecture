#include <iostream>

#include <OpenGP/GL/GlfwWindow.h>
#include <OpenGP/GL/SyntheticDepthmap.h>
#include <OpenGP/GL/FullscreenQuad.h>
#include <OpenGP/SurfaceMesh/SurfaceMesh.h>
#include <OpenGP/SurfaceMesh/GL/SurfaceMeshDepthmapRenderer.h>
#include <OpenGP/SphereMesh/GL/SphereMeshDepthmapRenderer.h>


using namespace OpenGP;

int main(int argc, char **argv) {

    std::string filename = (argc > 1) ? argv[1] : "example.smo";

    GlfwWindow window("Test Window", 800, 600);

    SphereMesh mesh;
    mesh.read(filename);
    SphereMeshDepthmapRenderer renderer(mesh);
    renderer.init();

    FullscreenQuad fsq;

    SyntheticDepthmap depthmap(1024, 1024);

    window.set_user_update_fn([&](){

        fsq.draw_texture(depthmap.get_texture());

        depthmap.clear();

        depthmap.render(renderer, Eigen::Matrix4f::Identity(), OpenGP::lookAt(Vec3(-5, 5, -5), Vec3(0, 0, 0), Vec3(0, 1, 0)), OpenGP::perspective(40, 1, 0.1, 1000));

    });

    GlfwWindow::run();

}
