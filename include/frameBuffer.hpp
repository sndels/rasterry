#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <GL/gl3w.h>
#include <glm/glm.hpp>
#include <vector>

#include "color.hpp"

class FrameBuffer
{
public:
    FrameBuffer(uint32_t width, uint32_t height);
    ~FrameBuffer();

    uint32_t width();
    uint32_t height();
    void     resize(uint32_t width, uint32_t height);
    void     setPixel(const glm::uvec2& p, const Color& color);
    void     display();

private:
    uint32_t             _width;
    uint32_t             _height;
    std::vector<Color> _pixels;

    GLuint             _fbo;
    GLuint             _textureID;
};

#endif // FRAMEBUFFER_HPP
