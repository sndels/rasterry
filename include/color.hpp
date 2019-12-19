#ifndef COLOR_HPP
#define COLOR_HPP

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    Color() : r(0), g(0), b(0) {}
    Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
    Color(uint8_t c) : r(c), g(c), b(c) {}
};

#endif // COLOR_HPP
