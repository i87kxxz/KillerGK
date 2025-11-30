/**
 * @file Types.cpp
 * @brief Implementation of core types
 */

#include "KillerGK/core/Types.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace KillerGK {

// Predefined colors
const Color Color::White{1.0f, 1.0f, 1.0f, 1.0f};
const Color Color::Black{0.0f, 0.0f, 0.0f, 1.0f};
const Color Color::Red{1.0f, 0.0f, 0.0f, 1.0f};
const Color Color::Green{0.0f, 1.0f, 0.0f, 1.0f};
const Color Color::Blue{0.0f, 0.0f, 1.0f, 1.0f};
const Color Color::Yellow{1.0f, 1.0f, 0.0f, 1.0f};
const Color Color::Cyan{0.0f, 1.0f, 1.0f, 1.0f};
const Color Color::Magenta{1.0f, 0.0f, 1.0f, 1.0f};
const Color Color::Transparent{0.0f, 0.0f, 0.0f, 0.0f};

Color Color::hex(const std::string& hex) {
    std::string h = hex;
    if (!h.empty() && h[0] == '#') {
        h = h.substr(1);
    }

    if (h.length() == 3) {
        // Expand shorthand (e.g., "FFF" -> "FFFFFF")
        h = std::string{h[0], h[0], h[1], h[1], h[2], h[2]};
    }

    if (h.length() != 6 && h.length() != 8) {
        return Color::Black;
    }

    auto parseHex = [](const std::string& s, size_t pos) -> int {
        return std::stoi(s.substr(pos, 2), nullptr, 16);
    };

    float r = parseHex(h, 0) / 255.0f;
    float g = parseHex(h, 2) / 255.0f;
    float b = parseHex(h, 4) / 255.0f;
    float a = h.length() == 8 ? parseHex(h, 6) / 255.0f : 1.0f;

    return Color{r, g, b, a};
}

Color Color::rgb(int r, int g, int b) {
    return Color{
        std::clamp(r, 0, 255) / 255.0f,
        std::clamp(g, 0, 255) / 255.0f,
        std::clamp(b, 0, 255) / 255.0f,
        1.0f
    };
}

Color Color::rgba(int r, int g, int b, float a) {
    return Color{
        std::clamp(r, 0, 255) / 255.0f,
        std::clamp(g, 0, 255) / 255.0f,
        std::clamp(b, 0, 255) / 255.0f,
        std::clamp(a, 0.0f, 1.0f)
    };
}

Color Color::hsl(float h, float s, float l) {
    h = std::fmod(h, 360.0f);
    if (h < 0) h += 360.0f;
    s = std::clamp(s, 0.0f, 1.0f);
    l = std::clamp(l, 0.0f, 1.0f);

    float c = (1.0f - std::abs(2.0f * l - 1.0f)) * s;
    float x = c * (1.0f - std::abs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = l - c / 2.0f;

    float r, g, b;
    if (h < 60) { r = c; g = x; b = 0; }
    else if (h < 120) { r = x; g = c; b = 0; }
    else if (h < 180) { r = 0; g = c; b = x; }
    else if (h < 240) { r = 0; g = x; b = c; }
    else if (h < 300) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }

    return Color{r + m, g + m, b + m, 1.0f};
}

Color Color::lighten(float amount) const {
    return Color{
        std::clamp(r + amount, 0.0f, 1.0f),
        std::clamp(g + amount, 0.0f, 1.0f),
        std::clamp(b + amount, 0.0f, 1.0f),
        a
    };
}

Color Color::darken(float amount) const {
    return lighten(-amount);
}

Color Color::withAlpha(float alpha) const {
    return Color{r, g, b, std::clamp(alpha, 0.0f, 1.0f)};
}

// Rect implementation
bool Rect::contains(float px, float py) const {
    return px >= x && px <= x + width && py >= y && py <= y + height;
}

bool Rect::contains(const Point& p) const {
    return contains(p.x, p.y);
}

bool Rect::intersects(const Rect& other) const {
    return !(x + width < other.x || other.x + other.width < x ||
             y + height < other.y || other.y + other.height < y);
}

Rect Rect::expand(float amount) const {
    return Rect{
        x - amount,
        y - amount,
        width + 2 * amount,
        height + 2 * amount
    };
}

} // namespace KillerGK
