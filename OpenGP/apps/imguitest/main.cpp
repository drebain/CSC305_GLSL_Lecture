#include <OpenGP/types.h>
#include <OpenGP/MLogger.h>

///--- Mesh rendering
#include <OpenGP/GL/Application.h>
#include <OpenGP/GL/ImguiRenderer.h>

#define OPENGP_IMPLEMENT_ALL_IN_THIS_FILE
#include <OpenGP/util/implementations.h>

using namespace OpenGP;
int main(int /*argc*/, char** /*argv*/){

    Application app;

    ImguiRenderer imrenderer;

    auto &window = app.create_window([&](Window &window){

        int width, height;
        std::tie(width, height) = window.get_size();

        imrenderer.begin_frame(width, height);

        ImGui::BeginMainMenuBar();
        ImGui::MenuItem("File");
        ImGui::MenuItem("Edit");
        ImGui::MenuItem("View");
        ImGui::MenuItem("Help");
        ImGui::EndMainMenuBar();

        ImGui::Begin("Test Window 1");
        ImGui::Text("This is a test imgui window");
        ImGui::End();

        glClearColor(0.15f, 0.15f, 0.15f, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        imrenderer.end_frame();

    });
    window.set_title("imgui Test");

    auto &window2 = app.create_window([&](Window &window){

        int width, height;
        std::tie(width, height) = window.get_size();

        imrenderer.begin_frame(width, height);

        ImGui::BeginMainMenuBar();
        ImGui::MenuItem("File");
        ImGui::MenuItem("Edit");
        ImGui::MenuItem("View");
        ImGui::MenuItem("Help");
        ImGui::EndMainMenuBar();

        ImGui::Begin("Test Window 2");
        ImGui::Text("This is another test imgui window");
        ImGui::End();

        glClearColor(0.15f, 0.15f, 0.15f, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        imrenderer.end_frame();

    });
    window2.set_title("imgui Test 2");

    return app.run();
}
