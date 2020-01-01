# Pull submodules
execute_process(COMMAND git submodule update --init --depth 1)

add_subdirectory(libgl3w)
add_subdirectory(glfw)

# GLM is header only
add_library(glm INTERFACE)
# Define as system to suppress warnings
target_include_directories(glm SYSTEM INTERFACE ${CMAKE_CURRENT_LIST_DIR}/glm)

# tinygltf is header only
add_library(tinygltf INTERFACE)
target_include_directories(tinygltf INTERFACE ${CMAKE_CURRENT_LIST_DIR}/tinygltf)

add_library(imgui STATIC "")
target_sources(imgui
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/examples/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/imgui/examples/imgui_impl_opengl3.cpp
)
target_include_directories(imgui
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/imgui
    ${CMAKE_CURRENT_LIST_DIR}/imgui/examples
)
target_link_libraries(imgui
    PUBLIC
    ${OPENGL_LIBRARIES}
    glfw
    libgl3w
)
