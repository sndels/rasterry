#include "clip.hpp"

namespace {
    inline glm::ivec2 clipToRaster(const glm::vec3& clipCoord, const glm::vec2& halfRes)
    {
        return glm::ivec2((glm::vec2(clipCoord) + 1.f) * halfRes);
    }

    inline float perpDotProd(const glm::vec2& a, const glm::vec2& b)
    {
        return a.x * b.y - a.y * b.x;
    }
}

// Adapted from https://en.wikipedia.org/wiki/Bresenham's_line_algorithm
void drawLine(const glm::vec3& p0, const glm::vec3& p1, const Color& color, FrameBuffer* fb)
{
    // TODO: Clamp lines, handle depth

    const glm::vec2 halfRes(fb->res() / glm::uvec2(2));
    const glm::ivec2 p0r = clipToRaster(p0, halfRes);
    const glm::ivec2 p1r = clipToRaster(p1, halfRes);

    const int32_t dx = abs(p0r.x - p1r.x);
    const int32_t sx = p0r.x < p1r.x ? 1 : -1;
    const int32_t dy = -abs(p0r.y - p1r.y);
    const int32_t sy = p0r.y < p1r.y ? 1 : -1;

    for (int32_t x = p0r.x, y = p0r.y, err = dx + dy;
         !(x == p1r.x && y == p1r.y);) {
        fb->setPixel(glm::ivec2(x, y), color);

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

// From Mileff et.al. with fixed edge directions
// https://www.uni-obuda.hu/journal/Mileff_Nehez_Dudra_63.pdf
void drawTri(const std::array<glm::vec3, 3>& verts, const Color& color, FrameBuffer* fb)
{
    const glm::ivec2 res = fb->res();
    const glm::vec2 halfRes(res / glm::ivec2(2));

    // Corners in raster space
    const glm::ivec2 a = clipToRaster(verts[0], halfRes);
    const glm::ivec2 b = clipToRaster(verts[1], halfRes);
    const glm::ivec2 c = clipToRaster(verts[2], halfRes);

    // Vectors from corner to corner
    const glm::ivec2 ab = b - a;
    const glm::ivec2 bc = c - b;
    const glm::ivec2 ca = a - c;

    // Clipped bounding box -> [min, max)
    const glm::ivec2 vMin = glm::max(
        glm::min(a, glm::min(b, c)),
        glm::ivec2(0)
    );
    const glm::ivec2 vMax = glm::min(
        glm::max(a, glm::max(b, c)),
        glm::ivec2(res)
    );

    for (uint32_t x = vMin.x; x < vMax.x; ++x) {
        for (uint32_t y = vMin.y; y < vMax.y; ++y) {
            // Do the half-space check
            // pdp = 0 -> parallel
            // pdp > 0 -> b is ccw from a
            // pdp < 0 -> b is cw from a
            const glm::ivec2 p(x, y);
            const float c0 = perpDotProd(ab, p - a);
            const float c1 = perpDotProd(bc, p - b);
            const float c2 = perpDotProd(ca, p - c);

            if (c0 >= 0 && c1 >= 0 && c2 >= 0)
                fb->setPixel(p, color);
        }
    }
}
