#ifndef MESH_HPP
#define MESH_HPP

#include <glm/glm.hpp>
#include <vector>

struct TriIndices {
    size_t v0 = 0;
    size_t v1 = 0;
    size_t v2 = 0;

    TriIndices() = default;
    TriIndices(size_t v0, size_t v1, size_t v2) :
        v0(v0),
        v1(v1),
        v2(v2)
    {}
};

struct Material;

struct Primitive {
    glm::vec3 min = glm::vec3(0.f);
    glm::vec3 max = glm::vec3(0.f);
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec4> tangents;
    std::vector<glm::vec2> texCoord0s;
    std::vector<TriIndices> tris;
    const Material* material = nullptr;
};

struct Mesh {
    glm::vec3 min = glm::vec3(0.f);
    glm::vec3 max = glm::vec3(0.f);
    std::vector<Primitive> primitives;
};

#endif // MESH_HPP
