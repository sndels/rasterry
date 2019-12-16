#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <GL/gl3w.h>
#include <glm/glm.hpp>
#include <vector>

#include "color.hpp"

class FrameBuffer
{
public:
    FrameBuffer(const glm::uvec2& res, const glm::uvec2& outRes);
    ~FrameBuffer();

    const glm::uvec2& res() const;
    void setPixel(const glm::uvec2& p, const Color& color);
    void display();
    void clear(const Color& color);

private:
    glm::uvec2 _res;
    glm::uvec2 _outRes;
    std::vector<Color> _pixels;

    GLuint _fbo;
    GLuint _textureID;
};

#endif // FRAMEBUFFER_HPP
