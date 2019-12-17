#ifndef MODEL_HPP
#define MODEL_HPP

#include <glm/glm.hpp>
#include <vector>

struct TriIndices {
    size_t v0;
    size_t v1;
    size_t v2;
};

struct Model {
    glm::vec3 min;
    glm::vec3 max;
    std::vector<glm::vec3> verts;
    std::vector<TriIndices> tris;
};

#endif // MODEL_HPP
