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

add_subdirectory(ext)
add_subdirectory(include)
add_subdirectory(src)

include_directories(include ext/libgl3w/include ext/glfw/include)

# Set absolute path to res directory
add_definitions(-DRES_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/")

add_executable(rasterry
    ${RASTERRY_SOURCES}
    ${RASTERRY_HEADERS}
)

target_include_directories(rasterry
    PRIVATE
    ${RASTERRY_INCLUDE_DIR}
)

target_link_libraries(rasterry
    PRIVATE
    ${OPENGL_LIBRARIES}
    glfw
    glm
    libgl3w
)
