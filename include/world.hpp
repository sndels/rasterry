#ifndef WORLD_HPP
#define WORLD_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

#include "material.hpp"
#include "mesh.hpp"
#include "texture.hpp"

struct Model;

struct Scene {
    struct Node {
        std::vector<Node*> children;
        const Mesh* mesh = nullptr;
        glm::vec3 translation = glm::vec3(0.f);
        glm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
        glm::vec3 scale = glm::vec3(1.f);
    };

    std::vector<Node*> nodes;
};

struct World {
    std::vector<Texture> textures;
    std::vector<Material> materials;
    std::vector<Mesh> meshes;
    std::vector<Scene::Node> nodes;
    std::vector<Scene> scenes;
    size_t currentScene = 0;
};

#endif // WORLD_HPP
