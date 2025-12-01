/**
 * @file Theme.cpp
 * @brief Theme system implementation
 */

#include "KillerGK/theme/Theme.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

namespace KillerGK {

// ============================================================================
// Color Interpolation Helpers
// ============================================================================

Color interpolateColor(const Color& from, const Color& to, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return Color(
        from.r + (to.r - from.r) * t,
        from.g + (to.g - from.g) * t,
        from.b + (to.b - from.b) * t,
        from.a + (to.a - from.a) * t
    );
}

ThemeColors interpolateColors(const ThemeColors& from, const ThemeColors& to, float t) {
    ThemeColors result;
    result.primary = interpolateColor(from.primary, to.primary, t);
    result.onPrimary = interpolateColor(from.onPrimary, to.onPrimary, t);
    result.primaryContainer = interpolateColor(from.primaryContainer, to.primaryContainer, t);
    result.onPrimaryContainer = interpolateColor(from.onPrimaryContainer, to.onPrimaryContainer, t);
    result.secondary = interpolateColor(from.secondary, to.secondary, t);
    result.onSecondary = interpolateColor(from.onSecondary, to.onSecondary, t);
    result.secondaryContainer = interpolateColor(from.secondaryContainer, to.secondaryContainer, t);
    result.onSecondaryContainer = interpolateColor(from.onSecondaryContainer, to.onSecondaryContainer, t);
    result.tertiary = interpolateColor(from.tertiary, to.tertiary, t);
    result.onTertiary = interpolateColor(from.onTertiary, to.onTertiary, t);
    result.error = interpolateColor(from.error, to.error, t);
    result.onError = interpolateColor(from.onError, to.onError, t);
    result.errorContainer = interpolateColor(from.errorContainer, to.errorContainer, t);
    result.onErrorContainer = interpolateColor(from.onErrorContainer, to.onErrorContainer, t);
    result.background = interpolateColor(from.background, to.background, t);
    result.onBackground = interpolateColor(from.onBackground, to.onBackground, t);
    result.surface = interpolateColor(from.surface, to.surface, t);
    result.onSurface = interpolateColor(from.onSurface, to.onSurface, t);
    result.surfaceVariant = interpolateColor(from.surfaceVariant, to.surfaceVariant, t);
    result.onSurfaceVariant = interpolateColor(from.onSurfaceVariant, to.onSurfaceVariant, t);
    result.outline = interpolateColor(from.outline, to.outline, t);
    result.outlineVariant = interpolateColor(from.outlineVariant, to.outlineVariant, t);
    result.shadow = interpolateColor(from.shadow, to.shadow, t);
    result.scrim = interpolateColor(from.scrim, to.scrim, t);
    result.inverseSurface = interpolateColor(from.inverseSurface, to.inverseSurface, t);
    result.inverseOnSurface = interpolateColor(from.inverseOnSurface, to.inverseOnSurface, t);
    result.inversePrimary = interpolateColor(from.inversePrimary, to.inversePrimary, t);
    return result;
}

// ============================================================================
// ThemeImpl Implementation
// ============================================================================

Color ThemeImpl::getColor(const std::string& colorName) const {
    // Map color names to values
    if (colorName == "primary") return colors.primary;
    if (colorName == "onPrimary") return colors.onPrimary;
    if (colorName == "primaryContainer") return colors.primaryContainer;
    if (colorName == "onPrimaryContainer") return colors.onPrimaryContainer;
    if (colorName == "secondary") return colors.secondary;
    if (colorName == "onSecondary") return colors.onSecondary;
    if (colorName == "secondaryContainer") return colors.secondaryContainer;
    if (colorName == "onSecondaryContainer") return colors.onSecondaryContainer;
    if (colorName == "tertiary") return colors.tertiary;
    if (colorName == "onTertiary") return colors.onTertiary;
    if (colorName == "error") return colors.error;
    if (colorName == "onError") return colors.onError;
    if (colorName == "errorContainer") return colors.errorContainer;
    if (colorName == "onErrorContainer") return colors.onErrorContainer;
    if (colorName == "background") return colors.background;
    if (colorName == "onBackground") return colors.onBackground;
    if (colorName == "surface") return colors.surface;
    if (colorName == "onSurface") return colors.onSurface;
    if (colorName == "surfaceVariant") return colors.surfaceVariant;
    if (colorName == "onSurfaceVariant") return colors.onSurfaceVariant;
    if (colorName == "outline") return colors.outline;
    if (colorName == "outlineVariant") return colors.outlineVariant;
    if (colorName == "shadow") return colors.shadow;
    if (colorName == "scrim") return colors.scrim;
    if (colorName == "inverseSurface") return colors.inverseSurface;
    if (colorName == "inverseOnSurface") return colors.inverseOnSurface;
    if (colorName == "inversePrimary") return colors.inversePrimary;
    
    // Fall back to parent if available
    if (parent) {
        return parent->getColor(colorName);
    }
    
    return Color::Black;
}

float ThemeImpl::getSpacing(const std::string& spacingName) const {
    if (spacingName == "none") return spacing.none;
    if (spacingName == "xs") return spacing.xs;
    if (spacingName == "sm") return spacing.sm;
    if (spacingName == "md") return spacing.md;
    if (spacingName == "lg") return spacing.lg;
    if (spacingName == "xl") return spacing.xl;
    if (spacingName == "xxl") return spacing.xxl;
    if (spacingName == "xxxl") return spacing.xxxl;
    
    if (parent) {
        return parent->getSpacing(spacingName);
    }
    
    return 0.0f;
}

float ThemeImpl::getShape(const std::string& shapeName) const {
    if (shapeName == "none") return shape.none;
    if (shapeName == "extraSmall") return shape.extraSmall;
    if (shapeName == "small") return shape.small;
    if (shapeName == "medium") return shape.medium;
    if (shapeName == "large") return shape.large;
    if (shapeName == "extraLarge") return shape.extraLarge;
    if (shapeName == "full") return shape.full;
    
    if (parent) {
        return parent->getShape(shapeName);
    }
    
    return 0.0f;
}

// ============================================================================
// Theme Builder Implementation
// ============================================================================

Theme::Theme() : m_impl(std::make_shared<ThemeImpl>()) {}

Theme::Theme(std::shared_ptr<ThemeImpl> parent) : m_impl(std::make_shared<ThemeImpl>()) {
    m_impl->parent = parent;
    // Copy parent values as defaults
    if (parent) {
        m_impl->colors = parent->colors;
        m_impl->typography = parent->typography;
        m_impl->spacing = parent->spacing;
        m_impl->shape = parent->shape;
        m_impl->elevation = parent->elevation;
        m_impl->effects = parent->effects;
        m_impl->transition = parent->transition;
        m_impl->mode = parent->mode;
    }
}

// ============================================================================
// Preset Themes - Material Design 3
// ============================================================================

Theme Theme::material() {
    Theme theme;
    theme.m_impl->name = "Material";
    theme.m_impl->mode = ThemeMode::Light;
    
    // Material Design 3 Light colors
    theme.m_impl->colors.primary = Color(0.40f, 0.31f, 0.64f, 1.0f);  // Purple
    theme.m_impl->colors.onPrimary = Color(1.0f, 1.0f, 1.0f, 1.0f);
    theme.m_impl->colors.primaryContainer = Color(0.91f, 0.85f, 1.0f, 1.0f);
    theme.m_impl->colors.onPrimaryContainer = Color(0.15f, 0.05f, 0.35f, 1.0f);
    
    theme.m_impl->colors.secondary = Color(0.38f, 0.36f, 0.44f, 1.0f);
    theme.m_impl->colors.onSecondary = Color(1.0f, 1.0f, 1.0f, 1.0f);
    theme.m_impl->colors.secondaryContainer = Color(0.91f, 0.88f, 0.96f, 1.0f);
    theme.m_impl->colors.onSecondaryContainer = Color(0.14f, 0.12f, 0.19f, 1.0f);
    
    theme.m_impl->colors.tertiary = Color(0.49f, 0.33f, 0.38f, 1.0f);
    theme.m_impl->colors.onTertiary = Color(1.0f, 1.0f, 1.0f, 1.0f);
    
    theme.m_impl->colors.error = Color(0.73f, 0.16f, 0.16f, 1.0f);
    theme.m_impl->colors.onError = Color(1.0f, 1.0f, 1.0f, 1.0f);
    theme.m_impl->colors.errorContainer = Color(0.98f, 0.85f, 0.85f, 1.0f);
    theme.m_impl->colors.onErrorContainer = Color(0.25f, 0.0f, 0.02f, 1.0f);
    
    theme.m_impl->colors.background = Color(1.0f, 0.98f, 1.0f, 1.0f);
    theme.m_impl->colors.onBackground = Color(0.11f, 0.11f, 0.13f, 1.0f);
    theme.m_impl->colors.surface = Color(1.0f, 0.98f, 1.0f, 1.0f);
    theme.m_impl->colors.onSurface = Color(0.11f, 0.11f, 0.13f, 1.0f);
    theme.m_impl->colors.surfaceVariant = Color(0.9f, 0.88f, 0.93f, 1.0f);
    theme.m_impl->colors.onSurfaceVariant = Color(0.28f, 0.27f, 0.31f, 1.0f);
    
    theme.m_impl->colors.outline = Color(0.47f, 0.45f, 0.5f, 1.0f);
    theme.m_impl->colors.outlineVariant = Color(0.78f, 0.76f, 0.81f, 1.0f);
    
    // Material typography
    theme.m_impl->typography.fontFamily = "Roboto";
    theme.m_impl->typography.monoFontFamily = "Roboto Mono";
    
    // Material shape
    theme.m_impl->shape.extraSmall = 4.0f;
    theme.m_impl->shape.small = 8.0f;
    theme.m_impl->shape.medium = 12.0f;
    theme.m_impl->shape.large = 16.0f;
    theme.m_impl->shape.extraLarge = 28.0f;
    
    return theme;
}

Theme Theme::materialDark() {
    Theme theme;
    theme.m_impl->name = "Material Dark";
    theme.m_impl->mode = ThemeMode::Dark;
    
    // Material Design 3 Dark colors
    theme.m_impl->colors.primary = Color(0.82f, 0.77f, 1.0f, 1.0f);
    theme.m_impl->colors.onPrimary = Color(0.24f, 0.14f, 0.44f, 1.0f);
    theme.m_impl->colors.primaryContainer = Color(0.32f, 0.22f, 0.54f, 1.0f);
    theme.m_impl->colors.onPrimaryContainer = Color(0.91f, 0.85f, 1.0f, 1.0f);
    
    theme.m_impl->colors.secondary = Color(0.8f, 0.78f, 0.86f, 1.0f);
    theme.m_impl->colors.onSecondary = Color(0.2f, 0.18f, 0.25f, 1.0f);
    theme.m_impl->colors.secondaryContainer = Color(0.29f, 0.27f, 0.34f, 1.0f);
    theme.m_impl->colors.onSecondaryContainer = Color(0.91f, 0.88f, 0.96f, 1.0f);
    
    theme.m_impl->colors.tertiary = Color(0.93f, 0.73f, 0.79f, 1.0f);
    theme.m_impl->colors.onTertiary = Color(0.29f, 0.14f, 0.19f, 1.0f);
    
    theme.m_impl->colors.error = Color(1.0f, 0.71f, 0.68f, 1.0f);
    theme.m_impl->colors.onError = Color(0.41f, 0.0f, 0.04f, 1.0f);
    theme.m_impl->colors.errorContainer = Color(0.57f, 0.07f, 0.1f, 1.0f);
    theme.m_impl->colors.onErrorContainer = Color(1.0f, 0.85f, 0.84f, 1.0f);
    
    theme.m_impl->colors.background = Color(0.07f, 0.07f, 0.09f, 1.0f);
    theme.m_impl->colors.onBackground = Color(0.9f, 0.89f, 0.93f, 1.0f);
    theme.m_impl->colors.surface = Color(0.07f, 0.07f, 0.09f, 1.0f);
    theme.m_impl->colors.onSurface = Color(0.9f, 0.89f, 0.93f, 1.0f);
    theme.m_impl->colors.surfaceVariant = Color(0.28f, 0.27f, 0.31f, 1.0f);
    theme.m_impl->colors.onSurfaceVariant = Color(0.78f, 0.76f, 0.81f, 1.0f);
    
    theme.m_impl->colors.outline = Color(0.57f, 0.55f, 0.6f, 1.0f);
    theme.m_impl->colors.outlineVariant = Color(0.28f, 0.27f, 0.31f, 1.0f);
    
    theme.m_impl->colors.inverseSurface = Color(0.9f, 0.89f, 0.93f, 1.0f);
    theme.m_impl->colors.inverseOnSurface = Color(0.19f, 0.18f, 0.21f, 1.0f);
    theme.m_impl->colors.inversePrimary = Color(0.40f, 0.31f, 0.64f, 1.0f);
    
    // Same typography and shape as light
    theme.m_impl->typography.fontFamily = "Roboto";
    theme.m_impl->typography.monoFontFamily = "Roboto Mono";
    theme.m_impl->shape.medium = 12.0f;
    
    return theme;
}

// ============================================================================
// Preset Themes - Flat/Modern
// ============================================================================

Theme Theme::flat() {
    Theme theme;
    theme.m_impl->name = "Flat";
    theme.m_impl->mode = ThemeMode::Light;
    
    // Clean, minimal colors
    theme.m_impl->colors.primary = Color(0.2f, 0.6f, 0.86f, 1.0f);  // Blue
    theme.m_impl->colors.onPrimary = Color(1.0f, 1.0f, 1.0f, 1.0f);
    theme.m_impl->colors.primaryContainer = Color(0.85f, 0.93f, 0.98f, 1.0f);
    theme.m_impl->colors.onPrimaryContainer = Color(0.05f, 0.2f, 0.35f, 1.0f);
    
    theme.m_impl->colors.secondary = Color(0.18f, 0.8f, 0.44f, 1.0f);  // Green
    theme.m_impl->colors.onSecondary = Color(1.0f, 1.0f, 1.0f, 1.0f);
    
    theme.m_impl->colors.error = Color(0.91f, 0.3f, 0.24f, 1.0f);  // Red
    theme.m_impl->colors.onError = Color(1.0f, 1.0f, 1.0f, 1.0f);
    
    theme.m_impl->colors.background = Color(0.98f, 0.98f, 0.98f, 1.0f);
    theme.m_impl->colors.onBackground = Color(0.2f, 0.2f, 0.2f, 1.0f);
    theme.m_impl->colors.surface = Color(1.0f, 1.0f, 1.0f, 1.0f);
    theme.m_impl->colors.onSurface = Color(0.2f, 0.2f, 0.2f, 1.0f);
    theme.m_impl->colors.surfaceVariant = Color(0.95f, 0.95f, 0.95f, 1.0f);
    theme.m_impl->colors.onSurfaceVariant = Color(0.4f, 0.4f, 0.4f, 1.0f);
    
    theme.m_impl->colors.outline = Color(0.75f, 0.75f, 0.75f, 1.0f);
    theme.m_impl->colors.outlineVariant = Color(0.9f, 0.9f, 0.9f, 1.0f);
    
    // Clean typography
    theme.m_impl->typography.fontFamily = "Inter";
    theme.m_impl->typography.monoFontFamily = "JetBrains Mono";
    
    // No rounded corners for flat design
    theme.m_impl->shape.none = 0.0f;
    theme.m_impl->shape.extraSmall = 2.0f;
    theme.m_impl->shape.small = 4.0f;
    theme.m_impl->shape.medium = 4.0f;
    theme.m_impl->shape.large = 4.0f;
    theme.m_impl->shape.extraLarge = 8.0f;
    
    // No shadows for flat design
    theme.m_impl->effects.shadowIntensity = 0.0f;
    
    return theme;
}

Theme Theme::flatDark() {
    Theme theme;
    theme.m_impl->name = "Flat Dark";
    theme.m_impl->mode = ThemeMode::Dark;
    
    // Dark flat colors
    theme.m_impl->colors.primary = Color(0.35f, 0.7f, 0.9f, 1.0f);
    theme.m_impl->colors.onPrimary = Color(0.0f, 0.15f, 0.25f, 1.0f);
    theme.m_impl->colors.primaryContainer = Color(0.1f, 0.3f, 0.45f, 1.0f);
    theme.m_impl->colors.onPrimaryContainer = Color(0.85f, 0.93f, 0.98f, 1.0f);
    
    theme.m_impl->colors.secondary = Color(0.3f, 0.85f, 0.55f, 1.0f);
    theme.m_impl->colors.onSecondary = Color(0.0f, 0.2f, 0.1f, 1.0f);
    
    theme.m_impl->colors.error = Color(1.0f, 0.5f, 0.45f, 1.0f);
    theme.m_impl->colors.onError = Color(0.3f, 0.0f, 0.0f, 1.0f);
    
    theme.m_impl->colors.background = Color(0.1f, 0.1f, 0.1f, 1.0f);
    theme.m_impl->colors.onBackground = Color(0.9f, 0.9f, 0.9f, 1.0f);
    theme.m_impl->colors.surface = Color(0.15f, 0.15f, 0.15f, 1.0f);
    theme.m_impl->colors.onSurface = Color(0.9f, 0.9f, 0.9f, 1.0f);
    theme.m_impl->colors.surfaceVariant = Color(0.2f, 0.2f, 0.2f, 1.0f);
    theme.m_impl->colors.onSurfaceVariant = Color(0.7f, 0.7f, 0.7f, 1.0f);
    
    theme.m_impl->colors.outline = Color(0.4f, 0.4f, 0.4f, 1.0f);
    theme.m_impl->colors.outlineVariant = Color(0.25f, 0.25f, 0.25f, 1.0f);
    
    theme.m_impl->typography.fontFamily = "Inter";
    theme.m_impl->typography.monoFontFamily = "JetBrains Mono";
    
    theme.m_impl->shape.medium = 4.0f;
    theme.m_impl->effects.shadowIntensity = 0.0f;
    
    return theme;
}

// ============================================================================
// Preset Themes - Glass
// ============================================================================

Theme Theme::glass() {
    Theme theme;
    theme.m_impl->name = "Glass";
    theme.m_impl->mode = ThemeMode::Light;
    
    // Glassmorphism colors with transparency
    theme.m_impl->colors.primary = Color(0.4f, 0.35f, 0.9f, 1.0f);
    theme.m_impl->colors.onPrimary = Color(1.0f, 1.0f, 1.0f, 1.0f);
    theme.m_impl->colors.primaryContainer = Color(0.4f, 0.35f, 0.9f, 0.15f);
    theme.m_impl->colors.onPrimaryContainer = Color(0.2f, 0.15f, 0.5f, 1.0f);
    
    theme.m_impl->colors.secondary = Color(0.0f, 0.8f, 0.7f, 1.0f);
    theme.m_impl->colors.onSecondary = Color(1.0f, 1.0f, 1.0f, 1.0f);
    
    theme.m_impl->colors.background = Color(0.95f, 0.95f, 0.98f, 0.8f);
    theme.m_impl->colors.onBackground = Color(0.1f, 0.1f, 0.15f, 1.0f);
    theme.m_impl->colors.surface = Color(1.0f, 1.0f, 1.0f, 0.6f);
    theme.m_impl->colors.onSurface = Color(0.1f, 0.1f, 0.15f, 1.0f);
    theme.m_impl->colors.surfaceVariant = Color(1.0f, 1.0f, 1.0f, 0.4f);
    theme.m_impl->colors.onSurfaceVariant = Color(0.3f, 0.3f, 0.35f, 1.0f);
    
    theme.m_impl->colors.outline = Color(1.0f, 1.0f, 1.0f, 0.3f);
    theme.m_impl->colors.outlineVariant = Color(1.0f, 1.0f, 1.0f, 0.15f);
    
    theme.m_impl->typography.fontFamily = "SF Pro Display";
    
    // Rounded corners for glass effect
    theme.m_impl->shape.small = 12.0f;
    theme.m_impl->shape.medium = 16.0f;
    theme.m_impl->shape.large = 24.0f;
    theme.m_impl->shape.extraLarge = 32.0f;
    
    // Enable glass effect
    theme.m_impl->effects.glassEffect = true;
    theme.m_impl->effects.glassBlur = 20.0f;
    theme.m_impl->effects.glassOpacity = 0.7f;
    theme.m_impl->effects.shadowIntensity = 0.15f;
    
    return theme;
}

Theme Theme::glassDark() {
    Theme theme;
    theme.m_impl->name = "Glass Dark";
    theme.m_impl->mode = ThemeMode::Dark;
    
    theme.m_impl->colors.primary = Color(0.6f, 0.55f, 1.0f, 1.0f);
    theme.m_impl->colors.onPrimary = Color(0.1f, 0.05f, 0.3f, 1.0f);
    theme.m_impl->colors.primaryContainer = Color(0.6f, 0.55f, 1.0f, 0.2f);
    theme.m_impl->colors.onPrimaryContainer = Color(0.85f, 0.82f, 1.0f, 1.0f);
    
    theme.m_impl->colors.secondary = Color(0.2f, 0.9f, 0.8f, 1.0f);
    theme.m_impl->colors.onSecondary = Color(0.0f, 0.2f, 0.18f, 1.0f);
    
    theme.m_impl->colors.background = Color(0.05f, 0.05f, 0.1f, 0.9f);
    theme.m_impl->colors.onBackground = Color(0.95f, 0.95f, 0.98f, 1.0f);
    theme.m_impl->colors.surface = Color(0.1f, 0.1f, 0.15f, 0.7f);
    theme.m_impl->colors.onSurface = Color(0.95f, 0.95f, 0.98f, 1.0f);
    theme.m_impl->colors.surfaceVariant = Color(0.15f, 0.15f, 0.2f, 0.5f);
    theme.m_impl->colors.onSurfaceVariant = Color(0.8f, 0.8f, 0.85f, 1.0f);
    
    theme.m_impl->colors.outline = Color(1.0f, 1.0f, 1.0f, 0.2f);
    theme.m_impl->colors.outlineVariant = Color(1.0f, 1.0f, 1.0f, 0.1f);
    
    theme.m_impl->typography.fontFamily = "SF Pro Display";
    
    theme.m_impl->shape.medium = 16.0f;
    
    theme.m_impl->effects.glassEffect = true;
    theme.m_impl->effects.glassBlur = 25.0f;
    theme.m_impl->effects.glassOpacity = 0.6f;
    
    return theme;
}

Theme Theme::custom() {
    return Theme();
}

Theme Theme::from(const ThemeHandle& parent) {
    return Theme(parent);
}

// ============================================================================
// Theme Builder Methods - Name
// ============================================================================

Theme& Theme::name(const std::string& themeName) {
    m_impl->name = themeName;
    return *this;
}

// ============================================================================
// Theme Builder Methods - Primary Colors
// ============================================================================

Theme& Theme::primaryColor(const Color& color) {
    m_impl->colors.primary = color;
    return *this;
}

Theme& Theme::onPrimaryColor(const Color& color) {
    m_impl->colors.onPrimary = color;
    return *this;
}

Theme& Theme::primaryContainerColor(const Color& color) {
    m_impl->colors.primaryContainer = color;
    return *this;
}

Theme& Theme::onPrimaryContainerColor(const Color& color) {
    m_impl->colors.onPrimaryContainer = color;
    return *this;
}

// ============================================================================
// Theme Builder Methods - Secondary Colors
// ============================================================================

Theme& Theme::secondaryColor(const Color& color) {
    m_impl->colors.secondary = color;
    return *this;
}

Theme& Theme::onSecondaryColor(const Color& color) {
    m_impl->colors.onSecondary = color;
    return *this;
}

Theme& Theme::secondaryContainerColor(const Color& color) {
    m_impl->colors.secondaryContainer = color;
    return *this;
}

Theme& Theme::onSecondaryContainerColor(const Color& color) {
    m_impl->colors.onSecondaryContainer = color;
    return *this;
}

// ============================================================================
// Theme Builder Methods - Tertiary Colors
// ============================================================================

Theme& Theme::tertiaryColor(const Color& color) {
    m_impl->colors.tertiary = color;
    return *this;
}

Theme& Theme::onTertiaryColor(const Color& color) {
    m_impl->colors.onTertiary = color;
    return *this;
}

// ============================================================================
// Theme Builder Methods - Error Colors
// ============================================================================

Theme& Theme::errorColor(const Color& color) {
    m_impl->colors.error = color;
    return *this;
}

Theme& Theme::onErrorColor(const Color& color) {
    m_impl->colors.onError = color;
    return *this;
}

Theme& Theme::errorContainerColor(const Color& color) {
    m_impl->colors.errorContainer = color;
    return *this;
}

Theme& Theme::onErrorContainerColor(const Color& color) {
    m_impl->colors.onErrorContainer = color;
    return *this;
}

// ============================================================================
// Theme Builder Methods - Background/Surface Colors
// ============================================================================

Theme& Theme::backgroundColor(const Color& color) {
    m_impl->colors.background = color;
    return *this;
}

Theme& Theme::onBackgroundColor(const Color& color) {
    m_impl->colors.onBackground = color;
    return *this;
}

Theme& Theme::surfaceColor(const Color& color) {
    m_impl->colors.surface = color;
    return *this;
}

Theme& Theme::onSurfaceColor(const Color& color) {
    m_impl->colors.onSurface = color;
    return *this;
}

Theme& Theme::surfaceVariantColor(const Color& color) {
    m_impl->colors.surfaceVariant = color;
    return *this;
}

Theme& Theme::onSurfaceVariantColor(const Color& color) {
    m_impl->colors.onSurfaceVariant = color;
    return *this;
}

// ============================================================================
// Theme Builder Methods - Outline Colors
// ============================================================================

Theme& Theme::outlineColor(const Color& color) {
    m_impl->colors.outline = color;
    return *this;
}

Theme& Theme::outlineVariantColor(const Color& color) {
    m_impl->colors.outlineVariant = color;
    return *this;
}

// ============================================================================
// Theme Builder Methods - Shadow/Scrim Colors
// ============================================================================

Theme& Theme::shadowColor(const Color& color) {
    m_impl->colors.shadow = color;
    return *this;
}

Theme& Theme::scrimColor(const Color& color) {
    m_impl->colors.scrim = color;
    return *this;
}

// ============================================================================
// Theme Builder Methods - Inverse Colors
// ============================================================================

Theme& Theme::inverseSurfaceColor(const Color& color) {
    m_impl->colors.inverseSurface = color;
    return *this;
}

Theme& Theme::inverseOnSurfaceColor(const Color& color) {
    m_impl->colors.inverseOnSurface = color;
    return *this;
}

Theme& Theme::inversePrimaryColor(const Color& color) {
    m_impl->colors.inversePrimary = color;
    return *this;
}

Theme& Theme::colors(const ThemeColors& themeColors) {
    m_impl->colors = themeColors;
    return *this;
}

Theme& Theme::textColor(const Color& color) {
    m_impl->colors.onSurface = color;
    m_impl->colors.onBackground = color;
    return *this;
}

// ============================================================================
// Theme Builder Methods - Typography
// ============================================================================

Theme& Theme::fontFamily(const std::string& family) {
    m_impl->typography.fontFamily = family;
    return *this;
}

Theme& Theme::monoFontFamily(const std::string& family) {
    m_impl->typography.monoFontFamily = family;
    return *this;
}

Theme& Theme::fontSize(float size) {
    m_impl->typography.bodyMedium = size;
    return *this;
}

Theme& Theme::typography(const ThemeTypography& themeTypography) {
    m_impl->typography = themeTypography;
    return *this;
}

// ============================================================================
// Theme Builder Methods - Spacing
// ============================================================================

Theme& Theme::spacing(const ThemeSpacing& themeSpacing) {
    m_impl->spacing = themeSpacing;
    return *this;
}

Theme& Theme::spacingScale(float scale) {
    m_impl->spacing.xs *= scale;
    m_impl->spacing.sm *= scale;
    m_impl->spacing.md *= scale;
    m_impl->spacing.lg *= scale;
    m_impl->spacing.xl *= scale;
    m_impl->spacing.xxl *= scale;
    m_impl->spacing.xxxl *= scale;
    return *this;
}

// ============================================================================
// Theme Builder Methods - Shape
// ============================================================================

Theme& Theme::borderRadius(float radius) {
    m_impl->shape.medium = radius;
    return *this;
}

Theme& Theme::shape(const ThemeShape& themeShape) {
    m_impl->shape = themeShape;
    return *this;
}

// ============================================================================
// Theme Builder Methods - Elevation
// ============================================================================

Theme& Theme::shadowIntensity(float intensity) {
    m_impl->effects.shadowIntensity = std::clamp(intensity, 0.0f, 1.0f);
    return *this;
}

Theme& Theme::elevation(const ThemeElevation& themeElevation) {
    m_impl->elevation = themeElevation;
    return *this;
}

// ============================================================================
// Theme Builder Methods - Effects
// ============================================================================

Theme& Theme::glassEffect(bool enabled) {
    m_impl->effects.glassEffect = enabled;
    return *this;
}

Theme& Theme::glassBlur(float blur) {
    m_impl->effects.glassBlur = std::max(0.0f, blur);
    return *this;
}

Theme& Theme::glassOpacity(float opacity) {
    m_impl->effects.glassOpacity = std::clamp(opacity, 0.0f, 1.0f);
    return *this;
}

Theme& Theme::acrylicEffect(bool enabled) {
    m_impl->effects.acrylicEffect = enabled;
    return *this;
}

Theme& Theme::acrylicBlur(float blur) {
    m_impl->effects.acrylicBlur = std::max(0.0f, blur);
    return *this;
}

Theme& Theme::acrylicNoiseOpacity(float opacity) {
    m_impl->effects.acrylicNoiseOpacity = std::clamp(opacity, 0.0f, 1.0f);
    return *this;
}

Theme& Theme::enableAnimations(bool enabled) {
    m_impl->effects.enableAnimations = enabled;
    return *this;
}

Theme& Theme::animationDuration(float duration) {
    m_impl->effects.animationDuration = std::max(0.0f, duration);
    return *this;
}

Theme& Theme::effects(const ThemeEffects& themeEffects) {
    m_impl->effects = themeEffects;
    return *this;
}

// ============================================================================
// Theme Builder Methods - Mode
// ============================================================================

Theme& Theme::darkMode(bool enabled) {
    m_impl->mode = enabled ? ThemeMode::Dark : ThemeMode::Light;
    
    // Auto-adjust colors if switching modes on a preset theme
    if (enabled && m_impl->colors.background.r > 0.5f) {
        // Switch to dark colors
        m_impl->colors.background = Color(0.12f, 0.12f, 0.12f, m_impl->colors.background.a);
        m_impl->colors.onBackground = Color(0.9f, 0.9f, 0.9f, 1.0f);
        m_impl->colors.surface = Color(0.18f, 0.18f, 0.18f, m_impl->colors.surface.a);
        m_impl->colors.onSurface = Color(0.9f, 0.9f, 0.9f, 1.0f);
        m_impl->colors.surfaceVariant = Color(0.25f, 0.25f, 0.28f, m_impl->colors.surfaceVariant.a);
        m_impl->colors.onSurfaceVariant = Color(0.78f, 0.78f, 0.8f, 1.0f);
    } else if (!enabled && m_impl->colors.background.r < 0.5f) {
        // Switch to light colors
        m_impl->colors.background = Color(1.0f, 1.0f, 1.0f, m_impl->colors.background.a);
        m_impl->colors.onBackground = Color(0.13f, 0.13f, 0.13f, 1.0f);
        m_impl->colors.surface = Color(0.98f, 0.98f, 0.98f, m_impl->colors.surface.a);
        m_impl->colors.onSurface = Color(0.13f, 0.13f, 0.13f, 1.0f);
        m_impl->colors.surfaceVariant = Color(0.9f, 0.9f, 0.92f, m_impl->colors.surfaceVariant.a);
        m_impl->colors.onSurfaceVariant = Color(0.27f, 0.27f, 0.31f, 1.0f);
    }
    
    return *this;
}

Theme& Theme::mode(ThemeMode themeMode) {
    return darkMode(themeMode == ThemeMode::Dark);
}

// ============================================================================
// Theme Builder Methods - Transition
// ============================================================================

Theme& Theme::transitionDuration(float duration) {
    m_impl->transition.duration = std::max(0.0f, duration);
    return *this;
}

Theme& Theme::transitionEnabled(bool enabled) {
    m_impl->transition.enabled = enabled;
    return *this;
}

Theme& Theme::transition(const ThemeTransition& themeTransition) {
    m_impl->transition = themeTransition;
    return *this;
}

// ============================================================================
// Theme Builder - Build
// ============================================================================

ThemeHandle Theme::build() {
    return m_impl;
}

// ============================================================================
// Theme Getters
// ============================================================================

const ThemeColors& Theme::getColors() const {
    return m_impl->colors;
}

const ThemeTypography& Theme::getTypography() const {
    return m_impl->typography;
}

const ThemeSpacing& Theme::getSpacing() const {
    return m_impl->spacing;
}

const ThemeShape& Theme::getShape() const {
    return m_impl->shape;
}

const ThemeElevation& Theme::getElevation() const {
    return m_impl->elevation;
}

const ThemeEffects& Theme::getEffects() const {
    return m_impl->effects;
}

ThemeMode Theme::getMode() const {
    return m_impl->mode;
}

const std::string& Theme::getName() const {
    return m_impl->name;
}

// Legacy getters
const Color& Theme::getPrimaryColor() const {
    return m_impl->colors.primary;
}

const Color& Theme::getSecondaryColor() const {
    return m_impl->colors.secondary;
}

const Color& Theme::getBackgroundColor() const {
    return m_impl->colors.background;
}

const Color& Theme::getSurfaceColor() const {
    return m_impl->colors.surface;
}

const Color& Theme::getTextColor() const {
    return m_impl->colors.onSurface;
}

bool Theme::isDarkMode() const {
    return m_impl->mode == ThemeMode::Dark;
}

// ============================================================================
// ThemeManager Implementation
// ============================================================================

struct ThemeManager::Impl {
    ThemeHandle currentTheme;
    ThemeHandle previousTheme;
    ThemeMode currentMode = ThemeMode::Light;
    
    std::vector<ThemeChangeCallback> themeCallbacks;
    std::vector<ModeChangeCallback> modeCallbacks;
    
    bool transitioning = false;
    float transitionProgress = 0.0f;
    ThemeColors transitionFromColors;
    ThemeColors transitionToColors;
};

ThemeManager::ThemeManager() : m_impl(std::make_unique<Impl>()) {
    // Initialize with Material theme
    m_impl->currentTheme = Theme::material().build();
}

ThemeManager::~ThemeManager() = default;

ThemeManager& ThemeManager::instance() {
    static ThemeManager instance;
    return instance;
}

void ThemeManager::setTheme(const ThemeHandle& theme) {
    if (!theme) return;
    
    m_impl->previousTheme = m_impl->currentTheme;
    m_impl->currentTheme = theme;
    m_impl->currentMode = theme->mode;
    
    // Start transition if enabled
    if (m_impl->previousTheme && theme->transition.enabled) {
        m_impl->transitioning = true;
        m_impl->transitionProgress = 0.0f;
        m_impl->transitionFromColors = m_impl->previousTheme->colors;
        m_impl->transitionToColors = theme->colors;
    }
    
    // Notify callbacks
    for (const auto& callback : m_impl->themeCallbacks) {
        callback(theme);
    }
}

ThemeHandle ThemeManager::currentTheme() const {
    return m_impl->currentTheme;
}

void ThemeManager::toggleMode() {
    setMode(m_impl->currentMode == ThemeMode::Light ? ThemeMode::Dark : ThemeMode::Light);
}

void ThemeManager::setMode(ThemeMode mode) {
    if (m_impl->currentMode == mode) return;
    
    m_impl->currentMode = mode;
    
    // Notify mode callbacks
    for (const auto& callback : m_impl->modeCallbacks) {
        callback(mode);
    }
}

ThemeMode ThemeManager::currentMode() const {
    return m_impl->currentMode;
}

void ThemeManager::onThemeChange(ThemeChangeCallback callback) {
    m_impl->themeCallbacks.push_back(std::move(callback));
}

void ThemeManager::onModeChange(ModeChangeCallback callback) {
    m_impl->modeCallbacks.push_back(std::move(callback));
}

ThemeColors ThemeManager::getTransitionColors(float progress) const {
    if (!m_impl->transitioning) {
        return m_impl->currentTheme ? m_impl->currentTheme->colors : ThemeColors{};
    }
    return interpolateColors(m_impl->transitionFromColors, m_impl->transitionToColors, progress);
}

bool ThemeManager::isTransitioning() const {
    return m_impl->transitioning;
}

float ThemeManager::transitionProgress() const {
    return m_impl->transitionProgress;
}

} // namespace KillerGK
