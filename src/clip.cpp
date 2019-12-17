#include "clip.hpp"

namespace {
    inline glm::ivec2 clipToRaster(const glm::vec3& clipCoord, const glm::vec2& halfRes)
    {
        return glm::ivec2((glm::vec2(clipCoord) + 1.f) * halfRes);
    }
}

// Adapted from https://en.wikipedia.org/wiki/Bresenham's_line_algorithm
void drawLine(const glm::vec3& p0, const glm::vec3& p1, const Color& color, FrameBuffer* fb)
{
    // TODO: Verify details, clamp lines?

    const glm::vec2 halfRes(fb->res() / glm::uvec2(2));
    const glm::ivec2 p0r = clipToRaster(p0, halfRes);
    const glm::ivec2 p1r = clipToRaster(p1, halfRes);

    const int32_t dx = abs(p0r.x - p1r.x);
    const int32_t sx = p0r.x < p1r.x ? 1 : -1;
    const int32_t dy = -abs(p0r.y - p1r.y);
    const int32_t sy = p0r.y < p1r.y ? 1 : -1;

    int32_t err = dx + dy;
    for (int32_t x = p0r.x, y = p0r.y; !(x == p1r.x && y == p1r.y);) {
        fb->setPixel(glm::uvec2(x, y), color);

        const int32_t e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
    }
}
