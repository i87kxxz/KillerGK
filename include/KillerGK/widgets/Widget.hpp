/**
 * @file Widget.hpp
 * @brief Base Widget class for KillerGK with Builder Pattern API
 */

#pragma once

#include "../core/Types.hpp"
#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <map>
#include <any>

namespace KillerGK {

// Forward declarations
class Animation;
class Widget;

/**
 * @enum Property
 * @brief Animatable widget properties
 */
enum class Property {
    X, Y, Width, Height,
    Opacity, Rotation, Scale,
    BackgroundColorR, BackgroundColorG, BackgroundColorB, BackgroundColorA,
    BorderRadius, BorderWidth,
    MarginTop, MarginRight, MarginBottom, MarginLeft,
    PaddingTop, PaddingRight, PaddingBottom, PaddingLeft
};

/**
 * @struct Spacing
 * @brief Represents margin or padding values
 */
struct Spacing {
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
    float left = 0.0f;

    constexpr Spacing() = default;
    constexpr Spacing(float all) : top(all), right(all), bottom(all), left(all) {}
    constexpr Spacing(float vertical, float horizontal) 
        : top(vertical), right(horizontal), bottom(vertical), left(horizontal) {}
    constexpr Spacing(float t, float r, float b, float l) 
        : top(t), right(r), bottom(b), left(l) {}

    bool operator==(const Spacing& other) const = default;
};

/**
 * @struct Shadow
 * @brief Shadow effect configuration
 */
struct Shadow {
    float blur = 0.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    Color color;

    constexpr Shadow() = default;
    constexpr Shadow(float b, float ox, float oy, const Color& c)
        : blur(b), offsetX(ox), offsetY(oy), color(c) {}

    bool operator==(const Shadow& other) const = default;
};


/**
 * @struct Transition
 * @brief Animation transition configuration for a property
 */
struct Transition {
    Property property;
    float duration = 0.0f;  // milliseconds

    constexpr Transition() : property(Property::Opacity) {}
    constexpr Transition(Property p, float d) : property(p), duration(d) {}

    bool operator==(const Transition& other) const = default;
};

/**
 * @struct WidgetState
 * @brief Serializable widget state for persistence
 */
struct WidgetState {
    std::string id;
    bool visible = true;
    bool enabled = true;
    bool focused = false;
    bool hovered = false;
    bool pressed = false;
    Rect bounds;
    std::map<std::string, std::any> properties;

    /**
     * @brief Serialize widget state to JSON string
     * @return JSON representation of the state
     */
    [[nodiscard]] std::string toJson() const;

    /**
     * @brief Deserialize widget state from JSON string
     * @param json JSON string to parse
     * @return Parsed WidgetState
     */
    static WidgetState fromJson(const std::string& json);

    bool operator==(const WidgetState& other) const;
};

/**
 * @enum EventType
 * @brief Types of widget events
 */
enum class EventType {
    Click,
    Hover,
    Focus,
    KeyPress,
    MouseMove,
    MouseDown,
    MouseUp
};

/**
 * @struct WidgetEvent
 * @brief Event data for widget callbacks
 */
struct WidgetEvent {
    EventType type;
    Widget* target = nullptr;
    bool handled = false;
    bool bubbles = true;

    // Mouse event data
    float mouseX = 0.0f;
    float mouseY = 0.0f;
    int button = 0;

    // Keyboard event data
    int keyCode = 0;
    bool shift = false;
    bool ctrl = false;
    bool alt = false;

    /**
     * @brief Stop event propagation
     */
    void stopPropagation() { bubbles = false; }

    /**
     * @brief Mark event as handled
     */
    void preventDefault() { handled = true; }
};


/**
 * @class Widget
 * @brief Base widget class using Builder Pattern
 * 
 * All widgets inherit from this class and use the fluent Builder Pattern
 * for configuration. Properties can be chained together for concise setup.
 * 
 * Example:
 * @code
 * auto widget = Widget::create()
 *     .id("myWidget")
 *     .width(100)
 *     .height(50)
 *     .backgroundColor(Color::Blue)
 *     .onClick([]() { std::cout << "Clicked!"; });
 * @endcode
 */
class Widget {
public:
    virtual ~Widget() = default;

    /**
     * @brief Create a new Widget instance
     * @return New Widget with default properties
     */
    static Widget create();

    // =========================================================================
    // Identity
    // =========================================================================

    Widget& id(const std::string& id);
    [[nodiscard]] const std::string& getId() const;

    // =========================================================================
    // Size Properties
    // =========================================================================

    Widget& width(float value);
    Widget& height(float value);
    Widget& minWidth(float value);
    Widget& maxWidth(float value);
    Widget& minHeight(float value);
    Widget& maxHeight(float value);

    [[nodiscard]] float getWidth() const;
    [[nodiscard]] float getHeight() const;
    [[nodiscard]] float getMinWidth() const;
    [[nodiscard]] float getMaxWidth() const;
    [[nodiscard]] float getMinHeight() const;
    [[nodiscard]] float getMaxHeight() const;

    // =========================================================================
    // Spacing Properties
    // =========================================================================

    Widget& margin(float all);
    Widget& margin(float vertical, float horizontal);
    Widget& margin(float top, float right, float bottom, float left);
    Widget& padding(float all);
    Widget& padding(float vertical, float horizontal);
    Widget& padding(float top, float right, float bottom, float left);

    [[nodiscard]] const Spacing& getMargin() const;
    [[nodiscard]] const Spacing& getPadding() const;

    // =========================================================================
    // Visibility and State
    // =========================================================================

    Widget& visible(bool value);
    Widget& enabled(bool value);
    Widget& tooltip(const std::string& text);

    [[nodiscard]] bool isVisible() const;
    [[nodiscard]] bool isEnabled() const;
    [[nodiscard]] const std::string& getTooltip() const;
    [[nodiscard]] bool isFocused() const;
    [[nodiscard]] bool isHovered() const;
    [[nodiscard]] bool isPressed() const;


    // =========================================================================
    // Styling Properties
    // =========================================================================

    Widget& backgroundColor(const Color& color);
    Widget& borderRadius(float radius);
    Widget& borderWidth(float width);
    Widget& borderColor(const Color& color);
    Widget& shadow(float blur, float offsetX, float offsetY, const Color& color);
    Widget& opacity(float value);
    Widget& blur(float radius);

    [[nodiscard]] const Color& getBackgroundColor() const;
    [[nodiscard]] float getBorderRadius() const;
    [[nodiscard]] float getBorderWidth() const;
    [[nodiscard]] const Color& getBorderColor() const;
    [[nodiscard]] const Shadow& getShadow() const;
    [[nodiscard]] float getOpacity() const;
    [[nodiscard]] float getBlur() const;

    // =========================================================================
    // Animation
    // =========================================================================

    Widget& animate(Animation& anim);
    Widget& transition(Property prop, float duration);

    [[nodiscard]] const std::vector<Transition>& getTransitions() const;

    // =========================================================================
    // Event Callbacks
    // =========================================================================

    Widget& onClick(std::function<void()> callback);
    Widget& onHover(std::function<void(bool)> callback);
    Widget& onFocus(std::function<void(bool)> callback);
    Widget& onEvent(EventType type, std::function<void(WidgetEvent&)> callback);

    // =========================================================================
    // Event Dispatch
    // =========================================================================

    /**
     * @brief Dispatch an event to this widget
     * @param event The event to dispatch
     * @return true if event was handled
     */
    bool dispatchEvent(WidgetEvent& event);

    /**
     * @brief Trigger click event
     */
    void triggerClick();

    /**
     * @brief Set hover state
     * @param hovered Whether widget is hovered
     */
    void setHovered(bool hovered);

    /**
     * @brief Set focus state
     * @param focused Whether widget is focused
     */
    void setFocused(bool focused);

    /**
     * @brief Set pressed state
     * @param pressed Whether widget is pressed
     */
    void setPressed(bool pressed);

    // =========================================================================
    // Hierarchy
    // =========================================================================

    Widget& parent(Widget* p);
    [[nodiscard]] Widget* getParent() const;
    [[nodiscard]] const std::vector<Widget*>& getChildren() const;
    void addChild(Widget* child);
    void removeChild(Widget* child);

    // =========================================================================
    // State Management
    // =========================================================================

    /**
     * @brief Get current widget state for serialization
     * @return Current state
     */
    [[nodiscard]] WidgetState getState() const;

    /**
     * @brief Restore widget state from serialized data
     * @param state State to restore
     */
    void setState(const WidgetState& state);

    // =========================================================================
    // Generic Property Access
    // =========================================================================

    /**
     * @brief Set a custom property value (float)
     * @param name Property name
     * @param value Property value
     */
    Widget& setPropertyFloat(const std::string& name, float value);

    /**
     * @brief Set a custom property value (int)
     * @param name Property name
     * @param value Property value
     */
    Widget& setPropertyInt(const std::string& name, int value);

    /**
     * @brief Set a custom property value (bool)
     * @param name Property name
     * @param value Property value
     */
    Widget& setPropertyBool(const std::string& name, bool value);

    /**
     * @brief Set a custom property value (string)
     * @param name Property name
     * @param value Property value
     */
    Widget& setPropertyString(const std::string& name, const std::string& value);

    /**
     * @brief Get a custom property value (float)
     * @param name Property name
     * @param defaultValue Default value if property not found
     * @return Property value or default
     */
    [[nodiscard]] float getPropertyFloat(const std::string& name, float defaultValue = 0.0f) const;

    /**
     * @brief Get a custom property value (int)
     * @param name Property name
     * @param defaultValue Default value if property not found
     * @return Property value or default
     */
    [[nodiscard]] int getPropertyInt(const std::string& name, int defaultValue = 0) const;

    /**
     * @brief Get a custom property value (bool)
     * @param name Property name
     * @param defaultValue Default value if property not found
     * @return Property value or default
     */
    [[nodiscard]] bool getPropertyBool(const std::string& name, bool defaultValue = false) const;

    /**
     * @brief Get a custom property value (string)
     * @param name Property name
     * @param defaultValue Default value if property not found
     * @return Property value or default
     */
    [[nodiscard]] std::string getPropertyString(const std::string& name, const std::string& defaultValue = "") const;

    /**
     * @brief Check if a custom property exists
     * @param name Property name
     * @return true if property exists
     */
    [[nodiscard]] bool hasProperty(const std::string& name) const;

protected:
    Widget();

    struct WidgetData;
    std::shared_ptr<WidgetData> m_data;
};

} // namespace KillerGK
