/**
 * @file Button.hpp
 * @brief Button widget for KillerGK with Builder Pattern API
 */

#pragma once

#include "Widget.hpp"
#include "../core/Types.hpp"
#include <string>
#include <functional>
#include <memory>

namespace KillerGK {

/**
 * @enum ButtonVariant
 * @brief Visual style variants for buttons
 */
enum class ButtonVariant {
    Primary,    ///< Primary action button with filled background
    Secondary,  ///< Secondary action button with lighter styling
    Outlined,   ///< Button with border only, no fill
    Text        ///< Text-only button with no border or fill
};

/**
 * @enum IconPosition
 * @brief Position of icon relative to button text
 */
enum class IconPosition {
    Left,   ///< Icon on the left of text
    Right,  ///< Icon on the right of text
    Top,    ///< Icon above text
    Bottom  ///< Icon below text
};

/**
 * @struct RippleEffect
 * @brief Configuration for button ripple animation effect
 */
struct RippleEffect {
    bool enabled = true;
    Color color = Color(1.0f, 1.0f, 1.0f, 0.3f);
    float duration = 400.0f;  // milliseconds
    float maxRadius = 0.0f;   // 0 = auto-calculate based on button size
    
    // Animation state
    bool active = false;
    float progress = 0.0f;
    float originX = 0.0f;
    float originY = 0.0f;
};

/**
 * @class Button
 * @brief Button widget with text, icon, and various visual states
 * 
 * Supports multiple variants (Primary, Secondary, Outlined, Text),
 * icons with configurable positions, loading states, and ripple effects.
 * 
 * Example:
 * @code
 * auto button = Button::create()
 *     .text("Click Me")
 *     .variant(ButtonVariant::Primary)
 *     .icon("icons/check.png")
 *     .iconPosition(IconPosition::Left)
 *     .ripple(true)
 *     .onClick([]() { std::cout << "Button clicked!"; });
 * @endcode
 */
class Button : public Widget {
public:
    virtual ~Button() = default;

    /**
     * @brief Create a new Button instance
     * @return New Button with default properties
     */
    static Button create();

    // =========================================================================
    // Button-specific Properties
    // =========================================================================

    /**
     * @brief Set button text
     * @param text Text to display
     * @return Reference to this button for chaining
     */
    Button& text(const std::string& text);

    /**
     * @brief Get button text
     * @return Current button text
     */
    [[nodiscard]] const std::string& getText() const;

    /**
     * @brief Set button icon path
     * @param iconPath Path to icon image
     * @return Reference to this button for chaining
     */
    Button& icon(const std::string& iconPath);

    /**
     * @brief Get button icon path
     * @return Current icon path
     */
    [[nodiscard]] const std::string& getIcon() const;

    /**
     * @brief Set icon position relative to text
     * @param pos Icon position
     * @return Reference to this button for chaining
     */
    Button& iconPosition(IconPosition pos);

    /**
     * @brief Get icon position
     * @return Current icon position
     */
    [[nodiscard]] IconPosition getIconPosition() const;

    /**
     * @brief Set button visual variant
     * @param var Button variant
     * @return Reference to this button for chaining
     */
    Button& variant(ButtonVariant var);

    /**
     * @brief Get button variant
     * @return Current button variant
     */
    [[nodiscard]] ButtonVariant getVariant() const;

    /**
     * @brief Set loading state
     * @param isLoading Whether button is in loading state
     * @return Reference to this button for chaining
     */
    Button& loading(bool isLoading);

    /**
     * @brief Check if button is in loading state
     * @return true if loading
     */
    [[nodiscard]] bool isLoading() const;

    /**
     * @brief Enable or disable ripple effect
     * @param enabled Whether ripple is enabled
     * @return Reference to this button for chaining
     */
    Button& ripple(bool enabled);

    /**
     * @brief Check if ripple effect is enabled
     * @return true if ripple is enabled
     */
    [[nodiscard]] bool hasRipple() const;

    /**
     * @brief Set ripple effect color
     * @param color Ripple color
     * @return Reference to this button for chaining
     */
    Button& rippleColor(const Color& color);

    /**
     * @brief Get ripple effect configuration
     * @return Current ripple configuration
     */
    [[nodiscard]] const RippleEffect& getRippleEffect() const;

    // =========================================================================
    // State Colors
    // =========================================================================

    /**
     * @brief Set hover state background color
     * @param color Hover color
     * @return Reference to this button for chaining
     */
    Button& hoverColor(const Color& color);

    /**
     * @brief Get hover state background color
     * @return Hover color
     */
    [[nodiscard]] const Color& getHoverColor() const;

    /**
     * @brief Set pressed state background color
     * @param color Pressed color
     * @return Reference to this button for chaining
     */
    Button& pressedColor(const Color& color);

    /**
     * @brief Get pressed state background color
     * @return Pressed color
     */
    [[nodiscard]] const Color& getPressedColor() const;

    /**
     * @brief Set disabled state background color
     * @param color Disabled color
     * @return Reference to this button for chaining
     */
    Button& disabledColor(const Color& color);

    /**
     * @brief Get disabled state background color
     * @return Disabled color
     */
    [[nodiscard]] const Color& getDisabledColor() const;

    /**
     * @brief Set text color
     * @param color Text color
     * @return Reference to this button for chaining
     */
    Button& textColor(const Color& color);

    /**
     * @brief Get text color
     * @return Text color
     */
    [[nodiscard]] const Color& getTextColor() const;

    // =========================================================================
    // Ripple Animation
    // =========================================================================

    /**
     * @brief Start ripple animation at specified position
     * @param x X coordinate of ripple origin
     * @param y Y coordinate of ripple origin
     */
    void startRipple(float x, float y);

    /**
     * @brief Update ripple animation
     * @param deltaTime Time since last update in milliseconds
     */
    void updateRipple(float deltaTime);

    /**
     * @brief Get current effective background color based on state
     * @return Current background color considering hover/pressed/disabled states
     */
    [[nodiscard]] Color getCurrentBackgroundColor() const;

protected:
    Button();

    struct ButtonData;
    std::shared_ptr<ButtonData> m_buttonData;
};

} // namespace KillerGK
