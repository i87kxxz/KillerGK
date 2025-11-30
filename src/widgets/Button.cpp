/**
 * @file Button.cpp
 * @brief Button widget implementation
 */

#include "KillerGK/widgets/Button.hpp"
#include <algorithm>

namespace KillerGK {

// =============================================================================
// ButtonData - Internal data structure
// =============================================================================

struct Button::ButtonData {
    std::string text;
    std::string iconPath;
    IconPosition iconPos = IconPosition::Left;
    ButtonVariant variant = ButtonVariant::Primary;
    bool isLoading = false;
    RippleEffect rippleEffect;
    
    // State colors
    Color hoverColor = Color(0.3f, 0.5f, 0.9f, 1.0f);
    Color pressedColor = Color(0.2f, 0.4f, 0.8f, 1.0f);
    Color disabledColor = Color(0.5f, 0.5f, 0.5f, 1.0f);
    Color textColor = Color::White;
};

// =============================================================================
// Button Implementation
// =============================================================================

Button::Button() 
    : Widget()
    , m_buttonData(std::make_shared<ButtonData>()) 
{
    // Set default button styling
    backgroundColor(Color(0.25f, 0.47f, 0.85f, 1.0f));  // Primary blue
    borderRadius(4.0f);
    padding(8.0f, 16.0f);
}

Button Button::create() {
    return Button();
}

// Text
Button& Button::text(const std::string& text) {
    m_buttonData->text = text;
    return *this;
}

const std::string& Button::getText() const {
    return m_buttonData->text;
}

// Icon
Button& Button::icon(const std::string& iconPath) {
    m_buttonData->iconPath = iconPath;
    return *this;
}

const std::string& Button::getIcon() const {
    return m_buttonData->iconPath;
}

Button& Button::iconPosition(IconPosition pos) {
    m_buttonData->iconPos = pos;
    return *this;
}

IconPosition Button::getIconPosition() const {
    return m_buttonData->iconPos;
}

// Variant
Button& Button::variant(ButtonVariant var) {
    m_buttonData->variant = var;
    
    // Apply default styling based on variant
    switch (var) {
        case ButtonVariant::Primary:
            backgroundColor(Color(0.25f, 0.47f, 0.85f, 1.0f));
            m_buttonData->textColor = Color::White;
            borderWidth(0.0f);
            break;
        case ButtonVariant::Secondary:
            backgroundColor(Color(0.9f, 0.9f, 0.9f, 1.0f));
            m_buttonData->textColor = Color(0.2f, 0.2f, 0.2f, 1.0f);
            borderWidth(0.0f);
            break;
        case ButtonVariant::Outlined:
            backgroundColor(Color::Transparent);
            m_buttonData->textColor = Color(0.25f, 0.47f, 0.85f, 1.0f);
            borderWidth(1.0f);
            borderColor(Color(0.25f, 0.47f, 0.85f, 1.0f));
            break;
        case ButtonVariant::Text:
            backgroundColor(Color::Transparent);
            m_buttonData->textColor = Color(0.25f, 0.47f, 0.85f, 1.0f);
            borderWidth(0.0f);
            break;
    }
    
    return *this;
}

ButtonVariant Button::getVariant() const {
    return m_buttonData->variant;
}

// Loading
Button& Button::loading(bool isLoading) {
    m_buttonData->isLoading = isLoading;
    return *this;
}

bool Button::isLoading() const {
    return m_buttonData->isLoading;
}

// Ripple
Button& Button::ripple(bool enabled) {
    m_buttonData->rippleEffect.enabled = enabled;
    return *this;
}

bool Button::hasRipple() const {
    return m_buttonData->rippleEffect.enabled;
}

Button& Button::rippleColor(const Color& color) {
    m_buttonData->rippleEffect.color = color;
    return *this;
}

const RippleEffect& Button::getRippleEffect() const {
    return m_buttonData->rippleEffect;
}

// State colors
Button& Button::hoverColor(const Color& color) {
    m_buttonData->hoverColor = color;
    return *this;
}

const Color& Button::getHoverColor() const {
    return m_buttonData->hoverColor;
}

Button& Button::pressedColor(const Color& color) {
    m_buttonData->pressedColor = color;
    return *this;
}

const Color& Button::getPressedColor() const {
    return m_buttonData->pressedColor;
}

Button& Button::disabledColor(const Color& color) {
    m_buttonData->disabledColor = color;
    return *this;
}

const Color& Button::getDisabledColor() const {
    return m_buttonData->disabledColor;
}

Button& Button::textColor(const Color& color) {
    m_buttonData->textColor = color;
    return *this;
}

const Color& Button::getTextColor() const {
    return m_buttonData->textColor;
}

// Ripple animation
void Button::startRipple(float x, float y) {
    if (!m_buttonData->rippleEffect.enabled) {
        return;
    }
    
    m_buttonData->rippleEffect.active = true;
    m_buttonData->rippleEffect.progress = 0.0f;
    m_buttonData->rippleEffect.originX = x;
    m_buttonData->rippleEffect.originY = y;
    
    // Calculate max radius if not set
    if (m_buttonData->rippleEffect.maxRadius <= 0.0f) {
        float w = getWidth();
        float h = getHeight();
        // Max radius is the distance from origin to the farthest corner
        float maxDistX = std::max(x, w - x);
        float maxDistY = std::max(y, h - y);
        m_buttonData->rippleEffect.maxRadius = std::sqrt(maxDistX * maxDistX + maxDistY * maxDistY);
    }
}

void Button::updateRipple(float deltaTime) {
    if (!m_buttonData->rippleEffect.active) {
        return;
    }
    
    float duration = m_buttonData->rippleEffect.duration;
    if (duration <= 0.0f) {
        duration = 400.0f;
    }
    
    m_buttonData->rippleEffect.progress += deltaTime / duration;
    
    if (m_buttonData->rippleEffect.progress >= 1.0f) {
        m_buttonData->rippleEffect.active = false;
        m_buttonData->rippleEffect.progress = 0.0f;
    }
}

Color Button::getCurrentBackgroundColor() const {
    if (!isEnabled()) {
        return m_buttonData->disabledColor;
    }
    
    if (isPressed()) {
        return m_buttonData->pressedColor;
    }
    
    if (isHovered()) {
        return m_buttonData->hoverColor;
    }
    
    return getBackgroundColor();
}

} // namespace KillerGK
