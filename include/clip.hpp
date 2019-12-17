#ifndef CLIP_HPP
#define CLIP_HPP

#include <glm/glm.hpp>

#include "frameBuffer.hpp"

void drawLine(const glm::vec3& p0, const glm::vec3& p1, const Color& color, FrameBuffer* fb);

#endif // CLIP_HPP
