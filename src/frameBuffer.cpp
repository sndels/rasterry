#include "frameBuffer.hpp"

#include <algorithm>

FrameBuffer::
    FrameBuffer(const glm::uvec2& res, const glm::uvec2& outRes) :
    _res(res),
    _outRes(outRes),
    _pixels(_res.x * _res.y),
    _depth(_res.x * _res.y)
{
    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);

    // Generate texture
    glGenTextures(1, &_textureID);
    glBindTexture(GL_TEXTURE_2D, _textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, _res.x, _res.y, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Bind to fbo
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _textureID, 0);
}

FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &_fbo);
    glDeleteTextures(1, &_textureID);
}

const glm::uvec2& FrameBuffer::res() const
{
    return _res;
}

float FrameBuffer::depth(const glm::ivec2& p) const
{
    return _depth[p.y * _res.x + p.x];
}

void FrameBuffer::setPixel(const glm::ivec2& p, const Color& color)
{
    _pixels[p.y * _res.x + p.x] = color;
}

void FrameBuffer::setDepth(const glm::ivec2& p, float value)
{
    _depth[p.y * _res.x + p.x] = value;
}

void FrameBuffer::display()
{
    // Push new frame to buffer
    glBindTexture(GL_TEXTURE_2D, _textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, _res.x, _res.y, 0, GL_RGB, GL_UNSIGNED_BYTE, _pixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    // Blit to default buffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, _res.x, _res.y, 0, 0, _outRes.x, _outRes.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void FrameBuffer::clear(const Color& color)
{
    std::fill(_pixels.begin(), _pixels.end(), color);
}

void FrameBuffer::clearDepth(float value)
{
    std::fill(_depth.begin(), _depth.end(), value);
}
