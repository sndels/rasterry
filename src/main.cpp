#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif // _WIN32

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "frameBuffer.hpp"
#include "clip.hpp"
#include "loader.hpp"
#include "timer.hpp"

using std::cout;
using std::cerr;
using std::endl;

void drawModel(const Model& model, const Color& color, FrameBuffer* fb)
{
    for (const auto& tri : model.tris) {
        const glm::vec3 v0 = model.verts[tri.v0] * 10.f + glm::vec3(0.2f, -1.f, 0.f);
        const glm::vec3 v1 = model.verts[tri.v1] * 10.f + glm::vec3(0.2f, -1.f, 0.f);
        const glm::vec3 v2 = model.verts[tri.v2] * 10.f + glm::vec3(0.2f, -1.f, 0.f);
        drawLine(v0, v1, color, fb);
        drawLine(v1, v2, color, fb);
        drawLine(v2, v0, color, fb);
    }
}

namespace {
    const static char* WINDOW_TITLE = "rasterry";
    glm::uvec2 RES(640, 480);
    uint32_t OUTPUT_SCALE = 2;
    glm::uvec2 OUTPUT_RES = RES * OUTPUT_SCALE;
}

void keyCallback(GLFWwindow* window, int32_t key, int32_t scancode, int32_t action,
                 int32_t mods)
{
    (void) scancode;
    (void) mods;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

static void errorCallback(int error, const char* description)
{
    cerr << "GLFW error " << error << ": " << description << endl;
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

    // Init buffer
    FrameBuffer fb(RES, OUTPUT_RES);

    const Color white(255, 255, 255);
    const Color red(255, 0, 0);

    // Load model
    Model model = loadOBJ(RES_DIRECTORY "obj/bunny.obj");

    Timer t;
    while (!glfwWindowShouldClose(windowPtr)) {
        glfwPollEvents();

        t.reset();
        fb.clear(Color(0, 0, 0));
        float clearTime = t.getMillis();

        t.reset();
        drawModel(model, white, &fb);
        float drawTime = t.getMillis();

        t.reset();
        fb.display();
        glfwSwapBuffers(windowPtr);
        float swapTime = t.getMillis();

        printf(
            "\rclear %.2fms draw %.2fms swap %.2fms          ",
            clearTime, drawTime, swapTime
        );
    }

    glfwDestroyWindow(windowPtr);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
