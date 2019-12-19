#ifndef CLIP_HPP
#define CLIP_HPP

// Windows is dumb and min/max are macros
#define NOMINMAX
#include <glm/glm.hpp>
#include <array>

#include "frameBuffer.hpp"

void drawLine(const glm::vec3& p0, const glm::vec3& p1, const Color& color, FrameBuffer* fb);
void drawTri(const std::array<glm::vec3, 3>& verts, const Color& color, FrameBuffer* fb);

#endif // CLIP_HPP
