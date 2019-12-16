#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <vector>
#include <GL/gl3w.h>

class FrameBuffer
{
public:
    FrameBuffer(uint32_t width, uint32_t height);
    ~FrameBuffer();

    uint32_t width();
    uint32_t height();
    void     resize(uint32_t width, uint32_t height);
    uint8_t*   pixelArray();
    void     display();

private:
    uint32_t             _width;
    uint32_t             _height;
    std::vector<uint8_t> _pixels;

    GLuint             _fbo;
    GLuint             _textureID;
};

#endif // FRAMEBUFFER_HPP
