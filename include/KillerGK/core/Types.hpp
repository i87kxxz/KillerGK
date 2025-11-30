/**
 * @file Types.hpp
 * @brief Core type definitions for KillerGK
 */

#pragma once

#include <cstdint>
#include <string>
#include <functional>
#include <memory>
#include <optional>
#include <variant>
#include <map>
#include <any>

namespace KillerGK {

/**
 * @struct Color
 * @brief RGBA color representation
 */
struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;

    constexpr Color() = default;
    constexpr Color(float r, float g, float b, float a = 1.0f) 
        : r(r), g(g), b(b), a(a) {}

    static Color hex(const std::string& hex);
    static Color rgb(int r, int g, int b);
    static Color rgba(int r, int g, int b, float a);
    static Color hsl(float h, float s, float l);

    [[nodiscard]] Color lighten(float amount) const;
    [[nodiscard]] Color darken(float amount) const;
    [[nodiscard]] Color withAlpha(float alpha) const;

    bool operator==(const Color& other) const = default;

    // Predefined colors
    static const Color White;
    static const Color Black;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Cyan;
    static const Color Magenta;
    static const Color Transparent;
};

/**
 * @struct Point
 * @brief 2D point representation
 */
struct Point {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Point() = default;
    constexpr Point(float x, float y) : x(x), y(y) {}

    bool operator==(const Point& other) const = default;
};

/**
 * @struct Size
 * @brief 2D size representation
 */
struct Size {
    float width = 0.0f;
    float height = 0.0f;

    constexpr Size() = default;
    constexpr Size(float w, float h) : width(w), height(h) {}

    bool operator==(const Size& other) const = default;
};

/**
 * @struct Rect
 * @brief Rectangle representation
 */
struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    constexpr Rect() = default;
    constexpr Rect(float x, float y, float w, float h) 
        : x(x), y(y), width(w), height(h) {}

    [[nodiscard]] bool contains(float px, float py) const;
    [[nodiscard]] bool contains(const Point& p) const;
    [[nodiscard]] bool intersects(const Rect& other) const;
    [[nodiscard]] Rect expand(float amount) const;

    [[nodiscard]] Point topLeft() const { return {x, y}; }
    [[nodiscard]] Point bottomRight() const { return {x + width, y + height}; }
    [[nodiscard]] Size size() const { return {width, height}; }

    bool operator==(const Rect& other) const = default;
};

} // namespace KillerGK
