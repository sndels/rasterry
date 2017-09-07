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
    float*   pixelArray();
    void     display();

private:
    uint32_t           _width;
    uint32_t           _height;
    std::vector<float> _pixels;

    GLuint             _fbo;
    GLuint             _textureID;
};

#endif // FRAMEBUFFER_HPP
