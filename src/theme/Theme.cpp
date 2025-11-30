/**
 * @file Theme.cpp
 * @brief Theme system implementation (stub)
 */

#include "KillerGK/theme/Theme.hpp"

namespace KillerGK {

struct Theme::Impl {
    Color primaryColor{0.25f, 0.47f, 0.85f, 1.0f};    // Material Blue
    Color secondaryColor{0.0f, 0.74f, 0.83f, 1.0f};   // Teal
    Color backgroundColor{1.0f, 1.0f, 1.0f, 1.0f};
    Color surfaceColor{0.98f, 0.98f, 0.98f, 1.0f};
    Color textColor{0.13f, 0.13f, 0.13f, 1.0f};
    Color errorColor{0.96f, 0.26f, 0.21f, 1.0f};
    std::string fontFamily = "Roboto";
    float fontSize = 14.0f;
    float borderRadius = 4.0f;
    float shadowIntensity = 0.2f;
    bool darkMode = false;
    bool glassEffect = false;
    bool acrylicEffect = false;
};

Theme::Theme() : m_impl(std::make_shared<Impl>()) {}

Theme Theme::material() {
    Theme theme;
    // Material Design 3 defaults are already set
    return theme;
}

Theme Theme::flat() {
    Theme theme;
    theme.m_impl->borderRadius = 0.0f;
    theme.m_impl->shadowIntensity = 0.0f;
    return theme;
}

Theme Theme::glass() {
    Theme theme;
    theme.m_impl->glassEffect = true;
    theme.m_impl->backgroundColor = Color(1.0f, 1.0f, 1.0f, 0.7f);
    theme.m_impl->surfaceColor = Color(1.0f, 1.0f, 1.0f, 0.5f);
    return theme;
}

Theme Theme::custom() {
    return Theme();
}

Theme& Theme::primaryColor(const Color& color) {
    m_impl->primaryColor = color;
    return *this;
}

Theme& Theme::secondaryColor(const Color& color) {
    m_impl->secondaryColor = color;
    return *this;
}

Theme& Theme::backgroundColor(const Color& color) {
    m_impl->backgroundColor = color;
    return *this;
}

Theme& Theme::surfaceColor(const Color& color) {
    m_impl->surfaceColor = color;
    return *this;
}

Theme& Theme::textColor(const Color& color) {
    m_impl->textColor = color;
    return *this;
}

Theme& Theme::errorColor(const Color& color) {
    m_impl->errorColor = color;
    return *this;
}

Theme& Theme::fontFamily(const std::string& family) {
    m_impl->fontFamily = family;
    return *this;
}

Theme& Theme::fontSize(float size) {
    m_impl->fontSize = size;
    return *this;
}

Theme& Theme::borderRadius(float radius) {
    m_impl->borderRadius = radius;
    return *this;
}

Theme& Theme::shadowIntensity(float intensity) {
    m_impl->shadowIntensity = intensity;
    return *this;
}

Theme& Theme::darkMode(bool enabled) {
    m_impl->darkMode = enabled;
    if (enabled) {
        m_impl->backgroundColor = Color(0.12f, 0.12f, 0.12f, 1.0f);
        m_impl->surfaceColor = Color(0.18f, 0.18f, 0.18f, 1.0f);
        m_impl->textColor = Color(1.0f, 1.0f, 1.0f, 0.87f);
    }
    return *this;
}

Theme& Theme::glassEffect(bool enabled) {
    m_impl->glassEffect = enabled;
    return *this;
}

Theme& Theme::acrylicEffect(bool enabled) {
    m_impl->acrylicEffect = enabled;
    return *this;
}

ThemeHandle Theme::build() {
    // TODO: Implement
    return nullptr;
}

const Color& Theme::getPrimaryColor() const {
    return m_impl->primaryColor;
}

const Color& Theme::getSecondaryColor() const {
    return m_impl->secondaryColor;
}

const Color& Theme::getBackgroundColor() const {
    return m_impl->backgroundColor;
}

const Color& Theme::getSurfaceColor() const {
    return m_impl->surfaceColor;
}

const Color& Theme::getTextColor() const {
    return m_impl->textColor;
}

bool Theme::isDarkMode() const {
    return m_impl->darkMode;
}

} // namespace KillerGK
