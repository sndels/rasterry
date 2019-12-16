# Pull submodules
execute_process(COMMAND git submodule update --init --depth 1)

add_subdirectory(libgl3w)
add_subdirectory(glfw)

# GLM is header only
add_library(glm INTERFACE)
# Define as system to suppress warnings
target_include_directories(glm SYSTEM INTERFACE ${CMAKE_CURRENT_LIST_DIR}/glm)
