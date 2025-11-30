/**
 * @file Theme.hpp
 * @brief Theme system for KillerGK
 */

#pragma once

#include "../core/Types.hpp"
#include <string>
#include <memory>

namespace KillerGK {

/**
 * @brief Handle to a built theme
 */
using ThemeHandle = std::shared_ptr<class ThemeImpl>;

/**
 * @class Theme
 * @brief Theme builder class
 */
class Theme {
public:
    // Preset themes
    static Theme material();
    static Theme flat();
    static Theme glass();
    static Theme custom();

    // Colors
    Theme& primaryColor(const Color& color);
    Theme& secondaryColor(const Color& color);
    Theme& backgroundColor(const Color& color);
    Theme& surfaceColor(const Color& color);
    Theme& textColor(const Color& color);
    Theme& errorColor(const Color& color);

    // Typography
    Theme& fontFamily(const std::string& family);
    Theme& fontSize(float size);

    // Styling
    Theme& borderRadius(float radius);
    Theme& shadowIntensity(float intensity);

    // Mode and effects
    Theme& darkMode(bool enabled);
    Theme& glassEffect(bool enabled);
    Theme& acrylicEffect(bool enabled);

    ThemeHandle build();

    // Getters
    [[nodiscard]] const Color& getPrimaryColor() const;
    [[nodiscard]] const Color& getSecondaryColor() const;
    [[nodiscard]] const Color& getBackgroundColor() const;
    [[nodiscard]] const Color& getSurfaceColor() const;
    [[nodiscard]] const Color& getTextColor() const;
    [[nodiscard]] bool isDarkMode() const;

private:
    Theme();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

} // namespace KillerGK
