#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <glm/glm.hpp>

class Texture;

struct Material {
    const Texture* baseColor = nullptr;
    const Texture* metallicRoughness = nullptr;
    const Texture* normal = nullptr;
    glm::vec4 baseColorFactor = glm::vec4(1.f);
    float metallicFactor = 1.f;
    float roughnessFactor = 1.f;
};

#endif // MATERIAL_HPP
