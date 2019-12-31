#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <glm/glm.hpp>
#include <tiny_gltf.h>

#include "color.hpp"

class Texture
{
public:
    Texture() = default;
    Texture(const tinygltf::Image& image);

    Texture(const Texture&) = delete;
    Texture(const Texture&& other);
    Texture& operator=(const Texture&) = delete;
    Texture& operator=(const Texture&& other);

    // Nearest sampling
    Color sample(const glm::vec2& uv) const;

private:
    glm::uvec2 pixelCoord(const glm::vec2& uv) const;

    glm::ivec2 _res = glm::ivec2(0,0);
    int _component = 0;
    std::vector<uint8_t> _pixels;
};

#endif // TEXTURE_HPP
