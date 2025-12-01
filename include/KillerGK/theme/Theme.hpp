/**
 * @file Theme.hpp
 * @brief Theme system for KillerGK
 */

#pragma once

#include "../core/Types.hpp"
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>

namespace KillerGK {

// Forward declarations
class ThemeImpl;
class Theme;

/**
 * @brief Handle to a built theme
 */
using ThemeHandle = std::shared_ptr<ThemeImpl>;

/**
 * @enum ThemeMode
 * @brief Light or dark mode
 */
enum class ThemeMode {
    Light,
    Dark
};

/**
 * @struct ThemeColors
 * @brief Complete color palette for a theme
 */
struct ThemeColors {
    Color primary{0.25f, 0.47f, 0.85f, 1.0f};
    Color onPrimary{1.0f, 1.0f, 1.0f, 1.0f};
    Color primaryContainer{0.85f, 0.91f, 1.0f, 1.0f};
    Color onPrimaryContainer{0.0f, 0.11f, 0.35f, 1.0f};
    
    Color secondary{0.0f, 0.74f, 0.83f, 1.0f};
    Color onSecondary{1.0f, 1.0f, 1.0f, 1.0f};
    Color secondaryContainer{0.8f, 0.95f, 0.97f, 1.0f};
    Color onSecondaryContainer{0.0f, 0.2f, 0.22f, 1.0f};
    
    Color tertiary{0.49f, 0.27f, 0.56f, 1.0f};
    Color onTertiary{1.0f, 1.0f, 1.0f, 1.0f};
    
    Color error{0.96f, 0.26f, 0.21f, 1.0f};
    Color onError{1.0f, 1.0f, 1.0f, 1.0f};
    Color errorContainer{1.0f, 0.85f, 0.84f, 1.0f};
    Color onErrorContainer{0.25f, 0.0f, 0.02f, 1.0f};
    
    Color background{1.0f, 1.0f, 1.0f, 1.0f};
    Color onBackground{0.13f, 0.13f, 0.13f, 1.0f};
    
    Color surface{0.98f, 0.98f, 0.98f, 1.0f};
    Color onSurface{0.13f, 0.13f, 0.13f, 1.0f};
    Color surfaceVariant{0.9f, 0.9f, 0.92f, 1.0f};
    Color onSurfaceVariant{0.27f, 0.27f, 0.31f, 1.0f};
    
    Color outline{0.47f, 0.47f, 0.5f, 1.0f};
    Color outlineVariant{0.78f, 0.78f, 0.8f, 1.0f};
    
    Color shadow{0.0f, 0.0f, 0.0f, 0.3f};
    Color scrim{0.0f, 0.0f, 0.0f, 0.5f};
    
    Color inverseSurface{0.19f, 0.19f, 0.22f, 1.0f};
    Color inverseOnSurface{0.95f, 0.95f, 0.95f, 1.0f};
    Color inversePrimary{0.67f, 0.8f, 1.0f, 1.0f};

    bool operator==(const ThemeColors& other) const = default;
};

/**
 * @struct ThemeTypography
 * @brief Typography settings for a theme
 */
struct ThemeTypography {
    std::string fontFamily = "Roboto";
    std::string monoFontFamily = "Roboto Mono";
    
    // Display styles
    float displayLarge = 57.0f;
    float displayMedium = 45.0f;
    float displaySmall = 36.0f;
    
    // Headline styles
    float headlineLarge = 32.0f;
    float headlineMedium = 28.0f;
    float headlineSmall = 24.0f;
    
    // Title styles
    float titleLarge = 22.0f;
    float titleMedium = 16.0f;
    float titleSmall = 14.0f;
    
    // Body styles
    float bodyLarge = 16.0f;
    float bodyMedium = 14.0f;
    float bodySmall = 12.0f;
    
    // Label styles
    float labelLarge = 14.0f;
    float labelMedium = 12.0f;
    float labelSmall = 11.0f;
    
    // Line heights (multipliers)
    float lineHeightTight = 1.2f;
    float lineHeightNormal = 1.5f;
    float lineHeightRelaxed = 1.75f;
    
    // Letter spacing
    float letterSpacingTight = -0.5f;
    float letterSpacingNormal = 0.0f;
    float letterSpacingWide = 0.5f;

    bool operator==(const ThemeTypography& other) const = default;
};

/**
 * @struct ThemeSpacing
 * @brief Spacing values for a theme
 */
struct ThemeSpacing {
    float none = 0.0f;
    float xs = 4.0f;
    float sm = 8.0f;
    float md = 16.0f;
    float lg = 24.0f;
    float xl = 32.0f;
    float xxl = 48.0f;
    float xxxl = 64.0f;

    bool operator==(const ThemeSpacing& other) const = default;
};

/**
 * @struct ThemeShape
 * @brief Shape/border radius values for a theme
 */
struct ThemeShape {
    float none = 0.0f;
    float extraSmall = 4.0f;
    float small = 8.0f;
    float medium = 12.0f;
    float large = 16.0f;
    float extraLarge = 28.0f;
    float full = 9999.0f;  // Fully rounded

    bool operator==(const ThemeShape& other) const = default;
};

/**
 * @struct ThemeElevation
 * @brief Shadow/elevation values for a theme
 */
struct ThemeElevation {
    float level0 = 0.0f;
    float level1 = 1.0f;
    float level2 = 3.0f;
    float level3 = 6.0f;
    float level4 = 8.0f;
    float level5 = 12.0f;

    bool operator==(const ThemeElevation& other) const = default;
};

/**
 * @struct ThemeEffects
 * @brief Visual effects settings
 */
struct ThemeEffects {
    bool glassEffect = false;
    float glassBlur = 20.0f;
    float glassOpacity = 0.7f;
    
    bool acrylicEffect = false;
    float acrylicBlur = 30.0f;
    float acrylicNoiseOpacity = 0.02f;
    
    float shadowIntensity = 0.2f;
    bool enableAnimations = true;
    float animationDuration = 200.0f;  // ms

    bool operator==(const ThemeEffects& other) const = default;
};

/**
 * @struct ThemeTransition
 * @brief Transition settings for theme mode changes
 */
struct ThemeTransition {
    float duration = 300.0f;  // ms
    bool enabled = true;

    bool operator==(const ThemeTransition& other) const = default;
};

/**
 * @class ThemeImpl
 * @brief Internal implementation of a built theme
 */
class ThemeImpl {
public:
    ThemeColors colors;
    ThemeTypography typography;
    ThemeSpacing spacing;
    ThemeShape shape;
    ThemeElevation elevation;
    ThemeEffects effects;
    ThemeTransition transition;
    ThemeMode mode = ThemeMode::Light;
    std::string name;
    
    // Parent theme for inheritance
    std::shared_ptr<ThemeImpl> parent;
    
    // Get effective color (with inheritance)
    [[nodiscard]] Color getColor(const std::string& name) const;
    
    // Get effective spacing
    [[nodiscard]] float getSpacing(const std::string& name) const;
    
    // Get effective shape
    [[nodiscard]] float getShape(const std::string& name) const;
};

/**
 * @class Theme
 * @brief Theme builder class with fluent API
 */
class Theme {
public:
    // Preset themes
    static Theme material();
    static Theme materialDark();
    static Theme flat();
    static Theme flatDark();
    static Theme glass();
    static Theme glassDark();
    static Theme custom();
    
    // Create from existing theme (for inheritance)
    static Theme from(const ThemeHandle& parent);

    // Theme name
    Theme& name(const std::string& themeName);

    // Primary colors
    Theme& primaryColor(const Color& color);
    Theme& onPrimaryColor(const Color& color);
    Theme& primaryContainerColor(const Color& color);
    Theme& onPrimaryContainerColor(const Color& color);
    
    // Secondary colors
    Theme& secondaryColor(const Color& color);
    Theme& onSecondaryColor(const Color& color);
    Theme& secondaryContainerColor(const Color& color);
    Theme& onSecondaryContainerColor(const Color& color);
    
    // Tertiary colors
    Theme& tertiaryColor(const Color& color);
    Theme& onTertiaryColor(const Color& color);
    
    // Error colors
    Theme& errorColor(const Color& color);
    Theme& onErrorColor(const Color& color);
    Theme& errorContainerColor(const Color& color);
    Theme& onErrorContainerColor(const Color& color);
    
    // Background/Surface colors
    Theme& backgroundColor(const Color& color);
    Theme& onBackgroundColor(const Color& color);
    Theme& surfaceColor(const Color& color);
    Theme& onSurfaceColor(const Color& color);
    Theme& surfaceVariantColor(const Color& color);
    Theme& onSurfaceVariantColor(const Color& color);
    
    // Outline colors
    Theme& outlineColor(const Color& color);
    Theme& outlineVariantColor(const Color& color);
    
    // Shadow/Scrim colors
    Theme& shadowColor(const Color& color);
    Theme& scrimColor(const Color& color);
    
    // Inverse colors
    Theme& inverseSurfaceColor(const Color& color);
    Theme& inverseOnSurfaceColor(const Color& color);
    Theme& inversePrimaryColor(const Color& color);
    
    // Convenience: set all colors at once
    Theme& colors(const ThemeColors& themeColors);
    
    // Legacy color setters (for compatibility)
    Theme& textColor(const Color& color);

    // Typography
    Theme& fontFamily(const std::string& family);
    Theme& monoFontFamily(const std::string& family);
    Theme& fontSize(float size);  // Sets bodyMedium
    Theme& typography(const ThemeTypography& themeTypography);

    // Spacing
    Theme& spacing(const ThemeSpacing& themeSpacing);
    Theme& spacingScale(float scale);  // Scale all spacing values

    // Shape
    Theme& borderRadius(float radius);  // Sets medium
    Theme& shape(const ThemeShape& themeShape);

    // Elevation
    Theme& shadowIntensity(float intensity);
    Theme& elevation(const ThemeElevation& themeElevation);

    // Effects
    Theme& glassEffect(bool enabled);
    Theme& glassBlur(float blur);
    Theme& glassOpacity(float opacity);
    Theme& acrylicEffect(bool enabled);
    Theme& acrylicBlur(float blur);
    Theme& acrylicNoiseOpacity(float opacity);
    Theme& enableAnimations(bool enabled);
    Theme& animationDuration(float duration);
    Theme& effects(const ThemeEffects& themeEffects);

    // Mode
    Theme& darkMode(bool enabled);
    Theme& mode(ThemeMode themeMode);
    
    // Transition settings
    Theme& transitionDuration(float duration);
    Theme& transitionEnabled(bool enabled);
    Theme& transition(const ThemeTransition& themeTransition);

    // Build the theme
    ThemeHandle build();

    // Getters for preview/inspection
    [[nodiscard]] const ThemeColors& getColors() const;
    [[nodiscard]] const ThemeTypography& getTypography() const;
    [[nodiscard]] const ThemeSpacing& getSpacing() const;
    [[nodiscard]] const ThemeShape& getShape() const;
    [[nodiscard]] const ThemeElevation& getElevation() const;
    [[nodiscard]] const ThemeEffects& getEffects() const;
    [[nodiscard]] ThemeMode getMode() const;
    [[nodiscard]] const std::string& getName() const;
    
    // Legacy getters (for compatibility)
    [[nodiscard]] const Color& getPrimaryColor() const;
    [[nodiscard]] const Color& getSecondaryColor() const;
    [[nodiscard]] const Color& getBackgroundColor() const;
    [[nodiscard]] const Color& getSurfaceColor() const;
    [[nodiscard]] const Color& getTextColor() const;
    [[nodiscard]] bool isDarkMode() const;

private:
    Theme();
    explicit Theme(std::shared_ptr<ThemeImpl> parent);
    
    std::shared_ptr<ThemeImpl> m_impl;
};

/**
 * @class ThemeManager
 * @brief Manages theme switching and transitions
 */
class ThemeManager {
public:
    static ThemeManager& instance();
    
    // Set the current theme
    void setTheme(const ThemeHandle& theme);
    
    // Get the current theme
    [[nodiscard]] ThemeHandle currentTheme() const;
    
    // Toggle dark/light mode
    void toggleMode();
    
    // Set mode explicitly
    void setMode(ThemeMode mode);
    
    // Get current mode
    [[nodiscard]] ThemeMode currentMode() const;
    
    // Register callback for theme changes
    using ThemeChangeCallback = std::function<void(const ThemeHandle&)>;
    void onThemeChange(ThemeChangeCallback callback);
    
    // Register callback for mode changes
    using ModeChangeCallback = std::function<void(ThemeMode)>;
    void onModeChange(ModeChangeCallback callback);
    
    // Get interpolated colors during transition
    [[nodiscard]] ThemeColors getTransitionColors(float progress) const;
    
    // Check if transition is in progress
    [[nodiscard]] bool isTransitioning() const;
    
    // Get transition progress (0.0 to 1.0)
    [[nodiscard]] float transitionProgress() const;

private:
    ThemeManager();
    ~ThemeManager();
    
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

// Helper function to interpolate between two colors
Color interpolateColor(const Color& from, const Color& to, float t);

// Helper function to interpolate between two theme color sets
ThemeColors interpolateColors(const ThemeColors& from, const ThemeColors& to, float t);

} // namespace KillerGK
