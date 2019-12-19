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

    // TODO: There have been more optimized _looking_ versions of this
    //       At least many of the variables don't depend on p
    //       Also, this expects point inside the triangle
    //       -> Needs to be considered if used for 'hit' check
    inline glm::vec3 barycentric(const glm::vec2& ab, const glm::vec2& ac, const glm::vec2& ap) {
        const float bb = glm::dot(ab, ab);
        const float bc = glm::dot(ab, ac);
        const float cc = glm::dot(ac, ac);
        const float pb = glm::dot(ap, ab);
        const float pc = glm::dot(ap, ac);

        const float invDenom = 1.f / (bb * cc - bc * bc);

        const float v = (cc * pb - bc * pc) * invDenom;
        const float w = (bb * pc - bc * pb) * invDenom;
        return glm::vec3(1.f - v - w, v, w);
    }

    template<typename T>
    inline T baryInterp(const std::array<T, 3> v, const glm::vec3& bary)
    {
        return bary[0] * v[0] + bary[1] * v[1] + bary[2] * v[2];
    }

    inline bool offscreen(const std::array<glm::vec3, 3> v) {
        // In front of near plane
        if (v[0].z < 0.f && v[1].z < 0.f && v[2].z < 0.f)
            return true;

        // Behind far plane
        if (v[0].z > 1.f && v[1].z > 1.f && v[2].z > 1.f)
            return true;

        // Off right
        if (v[0].x > 1.f && v[1].x > 1.f && v[2].x > 1.f)
            return true;

        // Off left
        if (v[0].x < -1.f && v[1].x < -1.f && v[2].x < -1.f)
            return true;

        // Off top
        if (v[0].y > 1.f && v[1].y > 1.f && v[2].y > 1.f)
            return true;

        // Off bottom
        if (v[0].y < -1.f && v[1].y < -1.f && v[2].y < -1.f)
            return true;

        return false;
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

// Base form from Mileff et.al. with fixed edge directions
// Expects ccw winding, does backface culling
// https://www.uni-obuda.hu/journal/Mileff_Nehez_Dudra_63.pdf
void drawTri(const std::array<glm::vec3, 3>& verts, const Color& color, FrameBuffer* fb)
{
    // Early out if whole tri is offscreen / out of clip volume
    if (offscreen(verts))
        return;

    const glm::vec2 res(fb->res());
    const glm::vec2 halfRes(res / 2.f);

    // Corners in raster space
    const glm::vec2 a = clipToRaster(verts[0], halfRes);
    const glm::vec2 b = clipToRaster(verts[1], halfRes);
    const glm::vec2 c = clipToRaster(verts[2], halfRes);

    // Vectors from corner to corner
    const glm::vec2 ab = b - a;
    const glm::vec2 bc = c - b;
    const glm::vec2 ca = a - c;

    // Viewport clipped bounding box -> [min, max)
    const glm::vec2 vMin = glm::max(
        glm::min(a, glm::min(b, c)),
        glm::vec2(0)
    );
    const glm::vec2 vMax = glm::min(
        glm::max(a, glm::max(b, c)),
        res
    );

    for (uint32_t x = vMin.x; x < std::ceil(vMax.x); ++x) {
        for (uint32_t y = vMin.y; y < std::ceil(vMax.y); ++y) {
            // Do the half-space check
            // pdp = 0 -> parallel
            // pdp > 0 -> b is ccw from a
            // pdp < 0 -> b is cw from a
            // TODO: Are barycentric coordinates upfront faster (once they are optimized)?
            const glm::vec2 p = glm::vec2(x, y) + 0.5f;
            const float c0 = perpDotProd(ab, p - a);
            const float c1 = perpDotProd(bc, p - b);
            const float c2 = perpDotProd(ca, p - c);

            if (c0 >= 0.f && c1 >= 0.f && c2 >= 0.f) {
                const glm::vec2 ac = -ca;
                const glm::vec2 ap = p - a;

                const glm::vec3 bary = barycentric(ab, ac, ap);

                const std::array<float, 3> depths = {verts[0].z, verts[1].z, verts[2].z};
                const float depth = baryInterp(depths, bary);

                if (depth < fb->depth(p)) {
                    fb->setPixel(glm::ivec2(p), color);
                    fb->setDepth(glm::ivec2(p), depth);
                }
            }
        }
    }
}
