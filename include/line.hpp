#ifndef LINE_HPP
#define LINE_HPP

#include <glm/glm.hpp>

#include "frameBuffer.hpp"

void drawLine(const glm::ivec2& p0, const glm::ivec2& p1, const Color& color, FrameBuffer* fb);

#endif // LINE_HPP
