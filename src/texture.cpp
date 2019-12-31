#include "texture.hpp"

Texture::Texture(const tinygltf::Image& image) :
    _res(image.width, image.height),
    _component(image.component),
    _pixels(image.image)
{
    if (_res.x <= 0 || _res.y <= 0)
        throw std::runtime_error("Texture with bad dimensions");
    if (_component <= 0 || _component > 4)
        throw std::runtime_error("Texture with bad components");
}

Texture::Texture(const Texture&& other) :
    _res(other._res),
    _component(other._component),
    _pixels(other._pixels)
{ }

Texture& Texture::operator=(const Texture&& other)
{
    if (this != &other) {
        Texture texture;
        texture._res = other._res;
        texture._component = other._component;
        texture._pixels = other._pixels;
    }
    return *this;
}

Color Texture::sample(const glm::vec2& uv) const
{
    const glm::uvec2 coord = pixelCoord(uv);
    return Color(
        _pixels[coord.y * _res.x + coord.x],
        _pixels[coord.y * _res.x + coord.x + 1],
        _pixels[coord.y * _res.x + coord.x + 2]
    );
}

glm::uvec2 Texture::pixelCoord(const glm::vec2& uv) const
{
    assert(uv.x >= 0.f && uv.x <= 1.f && uv.y >= 0.f && uv.y <= 1.f);
    // TODO: Wrapping
    return glm::clamp(
        glm::uvec2(uv.x *_res.x + uv.y * _res.y),
        glm::uvec2(0, 0),
        glm::uvec2(_res - 1)
    );
}
