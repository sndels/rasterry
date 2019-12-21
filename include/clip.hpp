#ifndef CLIP_HPP
#define CLIP_HPP

// Windows is dumb and min/max are macros
#define NOMINMAX
#include <glm/glm.hpp>
#include <array>

#include "frameBuffer.hpp"

void drawLine(const glm::vec4& p0, const glm::vec4& p1, const Color& color, FrameBuffer* fb);

// Expects non-divided clip coordinates, ccw winding
// Does backface culling
void drawTri(const std::array<glm::vec4, 3>& clipVerts, const Color& color, FrameBuffer* fb);

#endif // CLIP_HPP
