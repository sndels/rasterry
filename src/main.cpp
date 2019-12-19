#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

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

    const glm::vec3 LIGHT_DIR(-1, -1, 1);

    const Color white(255, 255, 255);
    const Color red(255, 0, 0);

    void drawModel(const Model& model, const glm::mat4& modelToWorld, const Camera& camera, FrameBuffer* fb)
    {
        const glm::mat4 modelToClip = camera.worldToClip() * modelToWorld;
        for (const auto& tri : model.tris) {
            const glm::vec3 v0 = model.verts[tri.v0];
            const glm::vec3 v1 = model.verts[tri.v1];
            const glm::vec3 v2 = model.verts[tri.v2];

            const glm::vec3 n = glm::normalize(glm::cross(v1 - v0, v2 - v0));
            const float NoL = glm::dot(n, -LIGHT_DIR);
            const Color shade(255 * std::max(NoL, 0.f));


            const std::array<glm::vec3, 3> clipVerts = [&]() {
                const glm::vec4 v0Clip = modelToClip * glm::vec4(v0, 1.f);
                const glm::vec4 v1Clip = modelToClip * glm::vec4(v1, 1.f);
                const glm::vec4 v2Clip = modelToClip * glm::vec4(v2, 1.f);
                return std::array<glm::vec3, 3>{
                    glm::vec3(v0Clip / v0Clip.w),
                    glm::vec3(v1Clip / v1Clip.w),
                    glm::vec3(v2Clip / v2Clip.w)
                };
            }();
            drawTri(clipVerts, shade, fb);
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

    // Init buffer
    FrameBuffer fb(RES, OUTPUT_RES);

    // Do the scene
    Camera camera;
    camera.lookAt(
        glm::vec3(0.f, 0.f, -7.5f),
        glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(0.f, 1.f, 0.f)
    );
    camera.perspective(glm::radians(59.f), float(RES.x) / RES.y, 0.1f, 50.f);

    Model model = loadOBJ(RES_DIRECTORY "obj/bunny.obj");
    // The bunny is small, off-center and left-handed
    const glm::mat4 modelToWorld =
        glm::translate(
            glm::scale(
                glm::mat4(
                    -1.f, 0.f,  0.f, 0.f,
                     0.f, 1.f,  0.f, 0.f,
                     0.f, 0.f, -1.f, 0.f,
                     0.f, 0.f,  0.f, 1.f
                ),
                glm::vec3(30.f)
            ),
            glm::vec3(0.015f, -0.1f, 0.f)
        );

    Timer t;
    Timer gt;
    while (!glfwWindowShouldClose(windowPtr)) {
        glfwPollEvents();

        // camera.lookAt(
        //     glm::vec3(0.f, 0.f, -gt.getSeconds()),
        //     glm::vec3(0.f, 0.f, 0.f),
        //     glm::vec3(0.f, 1.f, 0.f)
        // );

        t.reset();
        fb.clear(Color(0, 0, 0));
        float clearTime = t.getMillis();

        t.reset();
        drawModel(model, modelToWorld, camera, &fb);
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
