cmake_minimum_required(VERSION 3.4)

project(rasterry)

# Platform specific settings
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -pedantic")
    if (APPLE)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa -framework IOKit -framework CoreVideo")
    endif()
endif()

find_package(OpenGL REQUIRED)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS include/*.hpp)

add_subdirectory(ext)

include_directories(include ext/libgl3w/include ext/glfw/include)

# WIN32 tells to not build a cmd-app on windows
add_executable(rasterry WIN32
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(rasterry
    PRIVATE
    ${OPENGL_LIBRARIES}
    glfw
    glm
    libgl3w
)
