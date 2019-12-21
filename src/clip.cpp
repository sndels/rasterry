#include "clip.hpp"

namespace {
    inline glm::vec4 perspectiveDiv(const glm::vec4& clipP)
    {
        const float invW = 1.f / clipP.w;
        return glm::vec4(glm::vec3(clipP) * invW, invW);
    }

    inline glm::ivec2 NDCToFrag(const glm::vec4& ndcP, const glm::vec2& halfRes)
    {
        return glm::ivec2((glm::vec2(ndcP) + 1.f) * halfRes);
    }

    //  0 -> c is on edge a b
    // < 0 -> c is ccw from a
    // > 0 -> c is cw from a
    inline float edgeFunc(const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
    {
        return (c.y - a.y) * (b.x - a.x) - (c.x - a.x) * (b.y - a.y);
    }

    template<typename T>
    inline T baryInterp(const std::array<T, 3> values, const glm::vec3& bary)
    {
        return bary[0] * values[0] + bary[1] * values[1] + bary[2] * values[2];
    }

    inline bool outsideClip(const glm::vec4& clipP)
    {
        if (clipP.x < -clipP.w || clipP.x > clipP.w)
            return true;
        if (clipP.y < -clipP.w || clipP.y > clipP.w)
            return true;
        if (clipP.z < -clipP.w || clipP.z > clipP.w)
            return true;

        return false;
    }
}

// Adapted from https://en.wikipedia.org/wiki/Bresenham's_line_algorithm
void drawLine(const glm::vec4& clipP0, const glm::vec4& clipP1, const Color& color, FrameBuffer* fb)
{
    const glm::vec4 ndcP0 = perspectiveDiv(clipP0);
    const glm::vec4 ndcP1 = perspectiveDiv(clipP1);

    // TODO: Clamp lines, handle depth

    const glm::vec2 halfRes(fb->res() / glm::uvec2(2));
    const glm::ivec2 windowP0 = NDCToFrag(ndcP0, halfRes);
    const glm::ivec2 windowP1 = NDCToFrag(ndcP1, halfRes);

    const int32_t dx = abs(windowP0.x - windowP1.x);
    const int32_t sx = windowP0.x < windowP1.x ? 1 : -1;
    const int32_t dy = -abs(windowP0.y - windowP1.y);
    const int32_t sy = windowP0.y < windowP1.y ? 1 : -1;

    for (int32_t x = windowP0.x, y = windowP0.y, err = dx + dy;
         !(x == windowP1.x && y == windowP1.y);) {
        fb->setPixel(glm::ivec2(x, y), color);

        const int32_t windowE2 = 2 * err;
        if (windowE2 >= dy) {
            err += dy;
            x += sx;
        }
        if (windowE2 <= dx) {
            err += dx;
            y += sy;
        }
    }
}

void drawTri(const std::array<glm::vec4, 3>& clipVerts, const Color& color, FrameBuffer* fb)
{
    // Rough clipping
    if (outsideClip(clipVerts[0]) && outsideClip(clipVerts[1]) && outsideClip(clipVerts[2]))
        return;

    // NDC convention (clip.xyz / clip.w, 1 / clip.w)
    const glm::vec4 ndcV0 = perspectiveDiv(clipVerts[0]);
    const glm::vec4 ndcV1 = perspectiveDiv(clipVerts[1]);
    const glm::vec4 ndcV2 = perspectiveDiv(clipVerts[2]);

    // Interpolated per-fragment
    const std::array<float, 3> ndcDepths = {
        ndcV0.z,
        ndcV1.z,
        ndcV2.z
    };

    // Viewport transformation
    // Window coordinates bottom-left (0,0), top-right (res.x, res.y)
    const glm::vec2 res(fb->res());
    const glm::vec2 halfRes(res / 2.f);
    const glm::vec2 windowV0 = NDCToFrag(ndcV0, halfRes);
    const glm::vec2 windowV1 = NDCToFrag(ndcV1, halfRes);
    const glm::vec2 windowV2 = NDCToFrag(ndcV2, halfRes);

    // Used to enforce top-left rule
    const glm::vec2 windowE0 = windowV2 - windowV1;
    const glm::vec2 windowE1 = windowV0 - windowV2;
    const glm::vec2 windowE2 = windowV1 - windowV0;

    // (Double) tri area for barycentric coordinates
    const float area = edgeFunc(windowV0, windowV1, windowV2);

    // Viewport clipped bounding box -> [min, max)
    const glm::vec2 vMin = glm::max(
        glm::min(windowV0, glm::min(windowV1, windowV2)),
        glm::vec2(0)
    );
    const glm::vec2 vMax = glm::min(
        glm::max(windowV0, glm::max(windowV1, windowV2)),
        res
    );

    // Check and draw all fragments inside bounding box
    for (uint32_t x = vMin.x; x < std::ceil(vMax.x); ++x) {
        for (uint32_t y = vMin.y; y < std::ceil(vMax.y); ++y) {
            // Use pixel center as usual
            const glm::vec2 windowP = glm::vec2(x, y) + 0.5f;
            const glm::vec3 w(
                edgeFunc(windowV1, windowV2, windowP),
                edgeFunc(windowV2, windowV0, windowP),
                edgeFunc(windowV0, windowV1, windowP)
            );

            // Half-space check with top-left rule
            bool overlaps = true;
            overlaps &= w.x == 0 ? ((windowE0.y == 0 && windowE0.x > 0) || windowE0.y > 0) : (w.x > 0);
            overlaps &= w.y == 0 ? ((windowE1.y == 0 && windowE1.x > 0) || windowE1.y > 0) : (w.y > 0);
            overlaps &= w.z == 0 ? ((windowE2.y == 0 && windowE2.x > 0) || windowE2.y > 0) : (w.z > 0);

            if (overlaps) {
                // All attributes are interpolated with perspective corrected barys
                const glm::vec3 windowBary(w.x / area, w.y / area, w.z / area);
                const glm::vec3 correctedBary = [&](){
                    const glm::vec3 bary(
                        windowBary.x * ndcV0.w,
                        windowBary.y * ndcV1.w,
                        windowBary.z * ndcV2.w
                    );
                    return bary / (bary.x + bary.y + bary.z);
                }();

                // This makes depth non-linear, though it matches what OpenGL does
                const float depth = baryInterp(ndcDepths, windowBary);

                if (depth < fb->depth(windowP)) {
                    fb->setPixel(glm::ivec2(windowP), color);
                    fb->setDepth(glm::ivec2(windowP), depth);
                }
            }
        }
    }
}
