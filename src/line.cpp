#include "line.hpp"

// Adapted from https://en.wikipedia.org/wiki/Bresenham's_line_algorithm
void drawLine(const glm::ivec2& p0, const glm::ivec2& p1, const Color& color, FrameBuffer* fb)
{
    // TODO: Verify, clamp lines?

    const int32_t dx = abs(p0.x - p1.x);
    const int32_t sx = p0.x < p1.x ? 1 : -1;
    const int32_t dy = -abs(p0.y - p1.y);
    const int32_t sy = p0.y < p1.y ? 1 : -1;

    int32_t err = dx + dy;
    for (int32_t x = p0.x, y = p0.y; !(x == p1.x && y == p1.y);) {
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
