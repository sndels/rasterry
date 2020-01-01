#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <unordered_set>

#include "camera.hpp"
#include "clip.hpp"
#include "frameBuffer.hpp"
#include "loader.hpp"
#include "timer.hpp"

using std::cout;
using std::cerr;
using std::endl;

namespace {
    const static char* WINDOW_TITLE = "rasterry";
    glm::uvec2 RES(640, 480);
    uint32_t OUTPUT_SCALE = 2;
    glm::uvec2 OUTPUT_RES = RES * OUTPUT_SCALE;

    const glm::vec3 LIGHT_DIR = glm::normalize(glm::vec3(-1.f, -1.f, -2.f));

    const Color white(255, 255, 255);
    const Color red(255, 0, 0);

    void drawMesh(const Mesh& mesh, const glm::mat4& modelToWorld, const Camera& camera, FrameBuffer* fb)
    {
        for (const auto& primitive : mesh.primitives) {
            // This is basically a "vertex shader"
            for (const auto& tri : primitive.tris) {
                const glm::vec4 p0World = modelToWorld * glm::vec4(primitive.positions[tri.v0], 1.f);
                const glm::vec4 p1World = modelToWorld * glm::vec4(primitive.positions[tri.v1], 1.f);
                const glm::vec4 p2World = modelToWorld * glm::vec4(primitive.positions[tri.v2], 1.f);

                const glm::vec3 n = glm::normalize(glm::cross(
                    glm::vec3(p1World - p0World),
                    glm::vec3(p2World - p0World)
                ));
                const float NoL = glm::dot(n, -LIGHT_DIR);
                const Color shade(255 * NoL);

                const std::array<glm::vec4, 3> clipVerts = [&](){
                    return std::array<glm::vec4, 3>{
                        camera.worldToClip() * p0World,
                        camera.worldToClip() * p1World,
                        camera.worldToClip() * p2World
                    };
                }();

                drawTri(clipVerts, shade, fb);
            }
        }
    }

    void drawWorld(const World& world, const Camera& camera, FrameBuffer *fb)
    {
        // Go through scene graph using DFS while keeping track of stacked transform
        std::vector<glm::mat4> parentTransforms({ glm::mat4(1.f) });
        std::unordered_set<Scene::Node*> visited;
        std::vector<Scene::Node*> nodeStack = world.scenes[world.currentScene].nodes;
        while (!nodeStack.empty()) {
            const auto node = nodeStack.back();
            if (visited.find(node) != visited.end()) {
                nodeStack.pop_back();
                parentTransforms.pop_back();
            } else {
                visited.emplace(node);
                nodeStack.insert(nodeStack.end(), node->children.begin(), node->children.end());

                const glm::mat4 transform =
                    parentTransforms.back() *
                    glm::translate(glm::mat4(1.f), node->translation) *
                    glm::mat4_cast(node->rotation) *
                    glm::scale(glm::mat4(1.f), node->scale);

                if (node->mesh != nullptr)
                    drawMesh(*node->mesh, transform, camera, fb);

                parentTransforms.push_back(std::move(transform));
            }
        }
    }

    void keyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action,
                    int32_t mods)
    {
        (void) scancode;
        (void) mods;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
    }

    void errorCallback(int error, const char* description)
    {
        cerr << "GLFW error " << error << ": " << description << endl;
    }
}

int main()
{
    // Init GLFW-context
    glfwSetErrorCallback(errorCallback);
    if (!glfwInit()) exit(EXIT_FAILURE);

    // Set desired context hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // Create the window
    GLFWwindow* windowPtr;
    windowPtr = glfwCreateWindow(OUTPUT_RES.x, OUTPUT_RES.y, WINDOW_TITLE, NULL, NULL);
    if (!windowPtr) {
        glfwTerminate();
        cerr << "Error creating GLFW-window!" << endl;
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(windowPtr);

    // Init GL
    if (gl3wInit()) {
        glfwDestroyWindow(windowPtr);
        glfwTerminate();
        cerr << "Error initializing GL3W!" << endl;
        exit(EXIT_FAILURE);
    }

    // Set vsync on
    glfwSwapInterval(1);

    // Init GL settings
    glViewport(0, 0, OUTPUT_RES.x, OUTPUT_RES.y);
    glClearColor(0.f, 0.f, 0.f, 1.f);

    GLenum error = glGetError();
    if(error != GL_NO_ERROR) {
        glfwDestroyWindow(windowPtr);
        glfwTerminate();
        cerr << "Error initializing GL!" << endl;
        exit(EXIT_FAILURE);
    }

    // Set glfw-callbacks, these will pass to imgui's callbacks if overridden
    glfwSetKeyCallback(windowPtr, keyCallback);

    // Init imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(windowPtr, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    ImGuiWindowFlags mainWindowFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize;

    // Init buffer
    FrameBuffer fb(RES, OUTPUT_RES);

    // Do the scene
    Camera camera;
    camera.lookAt(
        glm::vec3(0.f, 50.f, 100.f),
        glm::vec3(0.f, 25.f, 0.f),
        glm::vec3(0.f, 1.f, 0.f)
    );
    camera.perspective(glm::radians(59.f), float(RES.x) / RES.y, 0.1f, 500.f);

    World world = loadGLTF(RES_DIRECTORY "res/the_noble_craftsman/scene.gltf");

    Mesh bunny = loadOBJ(RES_DIRECTORY "res/bunny.obj");
    // Scale and center bunny
    const float size = glm::compMax(bunny.max - bunny.min);
    const glm::vec3 offset = -(bunny.min + (bunny.max - bunny.min) / 2.f);
    const glm::mat4 bunnyToWorld =
        glm::translate(
            glm::scale(
                glm::mat4(
                    -1.f, 0.f,  0.f, 0.f,
                     0.f, 1.f,  0.f, 0.f,
                     0.f, 0.f, -1.f, 0.f,
                     0.f, 0.f,  0.f, 1.f
                ),
                glm::vec3(5.f / size) // magic scale for default camera position
            ),
            offset
        );

    Timer t;
    Timer gt;
    while (!glfwWindowShouldClose(windowPtr)) {
        glfwPollEvents();

        // Init imgui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // camera.lookAt(
        //     glm::vec3(0.f, 0.f, -gt.getSeconds()),
        //     glm::vec3(0.f, 0.f, 0.f),
        //     glm::vec3(0.f, 1.f, 0.f)
        // );

        // Setup frame buffer
        t.reset();
        fb.clearDepth(1.f);
        fb.clear(Color(0, 0, 0));
        float clearTime = t.getMillis();

        t.reset();
        // drawMesh(bunny, bunnyToWorld, camera, &fb);
        drawWorld(world, camera, &fb);
        float drawTime = t.getMillis();

        t.reset();
        fb.display();
        float displayTime = t.getMillis();

        // Draw profiler
        {
            ImGui::Begin("MainWindow", nullptr, mainWindowFlags);

            ImGui::Text(
                "clear %.2fms draw %.2fms display %.2fms",
                clearTime, drawTime, displayTime
            );
            ImGui::Text("avg frame %.2fms", 1000.f / ImGui::GetIO().Framerate);

            ImGui::End();
        }

        // Render imgui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(windowPtr);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(windowPtr);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}
