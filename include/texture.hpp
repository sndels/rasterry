#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <glm/glm.hpp>
#include <tiny_gltf.h>

#include "color.hpp"

class Texture
{
public:
    Texture(const tinygltf::Image& image);

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // Nearest sampling
    Color sample(const glm::vec2& uv) const;

private:
    glm::uvec2 pixelCoord(const glm::vec2& uv) const;

    glm::ivec2 _res;
    int _component;
    std::vector<uint8_t> _pixels;
};

#endif // TEXTURE_HPP
