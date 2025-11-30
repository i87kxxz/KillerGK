/**
 * @file Widget.cpp
 * @brief Base widget implementation with Builder Pattern
 */

#include "KillerGK/widgets/Widget.hpp"
#include <limits>
#include <sstream>
#include <algorithm>

namespace KillerGK {

// =============================================================================
// WidgetData - Internal data structure
// =============================================================================

struct Widget::WidgetData {
    // Identity
    std::string id;

    // Size
    float width = 0.0f;
    float height = 0.0f;
    float minWidth = 0.0f;
    float maxWidth = std::numeric_limits<float>::max();
    float minHeight = 0.0f;
    float maxHeight = std::numeric_limits<float>::max();

    // Spacing
    Spacing margin;
    Spacing padding;

    // Visibility and state
    bool visible = true;
    bool enabled = true;
    bool focused = false;
    bool hovered = false;
    bool pressed = false;
    std::string tooltip;

    // Styling
    Color backgroundColor;
    float borderRadius = 0.0f;
    float borderWidth = 0.0f;
    Color borderColor;
    Shadow shadow;
    float opacity = 1.0f;
    float blurRadius = 0.0f;

    // Animation
    std::vector<Transition> transitions;

    // Event callbacks
    std::function<void()> onClickCallback;
    std::function<void(bool)> onHoverCallback;
    std::function<void(bool)> onFocusCallback;
    std::map<EventType, std::function<void(WidgetEvent&)>> eventCallbacks;

    // Hierarchy
    Widget* parent = nullptr;
    std::vector<Widget*> children;

    // Custom properties
    std::map<std::string, std::any> customProperties;
};

// =============================================================================
// Widget Implementation
// =============================================================================

Widget::Widget() : m_data(std::make_shared<WidgetData>()) {}

Widget Widget::create() {
    return Widget();
}

// Identity
Widget& Widget::id(const std::string& id) {
    m_data->id = id;
    return *this;
}

const std::string& Widget::getId() const {
    return m_data->id;
}


// Size Properties
Widget& Widget::width(float value) {
    m_data->width = value;
    return *this;
}

Widget& Widget::height(float value) {
    m_data->height = value;
    return *this;
}

Widget& Widget::minWidth(float value) {
    m_data->minWidth = value;
    return *this;
}

Widget& Widget::maxWidth(float value) {
    m_data->maxWidth = value;
    return *this;
}

Widget& Widget::minHeight(float value) {
    m_data->minHeight = value;
    return *this;
}

Widget& Widget::maxHeight(float value) {
    m_data->maxHeight = value;
    return *this;
}

float Widget::getWidth() const {
    return m_data->width;
}

float Widget::getHeight() const {
    return m_data->height;
}

float Widget::getMinWidth() const {
    return m_data->minWidth;
}

float Widget::getMaxWidth() const {
    return m_data->maxWidth;
}

float Widget::getMinHeight() const {
    return m_data->minHeight;
}

float Widget::getMaxHeight() const {
    return m_data->maxHeight;
}

// Spacing Properties
Widget& Widget::margin(float all) {
    m_data->margin = Spacing(all);
    return *this;
}

Widget& Widget::margin(float vertical, float horizontal) {
    m_data->margin = Spacing(vertical, horizontal);
    return *this;
}

Widget& Widget::margin(float top, float right, float bottom, float left) {
    m_data->margin = Spacing(top, right, bottom, left);
    return *this;
}

Widget& Widget::padding(float all) {
    m_data->padding = Spacing(all);
    return *this;
}

Widget& Widget::padding(float vertical, float horizontal) {
    m_data->padding = Spacing(vertical, horizontal);
    return *this;
}

Widget& Widget::padding(float top, float right, float bottom, float left) {
    m_data->padding = Spacing(top, right, bottom, left);
    return *this;
}

const Spacing& Widget::getMargin() const {
    return m_data->margin;
}

const Spacing& Widget::getPadding() const {
    return m_data->padding;
}

// Visibility and State
Widget& Widget::visible(bool value) {
    m_data->visible = value;
    return *this;
}

Widget& Widget::enabled(bool value) {
    m_data->enabled = value;
    return *this;
}

Widget& Widget::tooltip(const std::string& text) {
    m_data->tooltip = text;
    return *this;
}

bool Widget::isVisible() const {
    return m_data->visible;
}

bool Widget::isEnabled() const {
    return m_data->enabled;
}

const std::string& Widget::getTooltip() const {
    return m_data->tooltip;
}

bool Widget::isFocused() const {
    return m_data->focused;
}

bool Widget::isHovered() const {
    return m_data->hovered;
}

bool Widget::isPressed() const {
    return m_data->pressed;
}


// Styling Properties
Widget& Widget::backgroundColor(const Color& color) {
    m_data->backgroundColor = color;
    return *this;
}

Widget& Widget::borderRadius(float radius) {
    m_data->borderRadius = radius;
    return *this;
}

Widget& Widget::borderWidth(float width) {
    m_data->borderWidth = width;
    return *this;
}

Widget& Widget::borderColor(const Color& color) {
    m_data->borderColor = color;
    return *this;
}

Widget& Widget::shadow(float blur, float offsetX, float offsetY, const Color& color) {
    m_data->shadow = Shadow(blur, offsetX, offsetY, color);
    return *this;
}

Widget& Widget::opacity(float value) {
    m_data->opacity = value;
    return *this;
}

Widget& Widget::blur(float radius) {
    m_data->blurRadius = radius;
    return *this;
}

const Color& Widget::getBackgroundColor() const {
    return m_data->backgroundColor;
}

float Widget::getBorderRadius() const {
    return m_data->borderRadius;
}

float Widget::getBorderWidth() const {
    return m_data->borderWidth;
}

const Color& Widget::getBorderColor() const {
    return m_data->borderColor;
}

const Shadow& Widget::getShadow() const {
    return m_data->shadow;
}

float Widget::getOpacity() const {
    return m_data->opacity;
}

float Widget::getBlur() const {
    return m_data->blurRadius;
}

// Animation
Widget& Widget::animate(Animation& /*anim*/) {
    // Animation integration will be implemented in Phase 6
    return *this;
}

Widget& Widget::transition(Property prop, float duration) {
    m_data->transitions.push_back(Transition(prop, duration));
    return *this;
}

const std::vector<Transition>& Widget::getTransitions() const {
    return m_data->transitions;
}

// Event Callbacks
Widget& Widget::onClick(std::function<void()> callback) {
    m_data->onClickCallback = std::move(callback);
    return *this;
}

Widget& Widget::onHover(std::function<void(bool)> callback) {
    m_data->onHoverCallback = std::move(callback);
    return *this;
}

Widget& Widget::onFocus(std::function<void(bool)> callback) {
    m_data->onFocusCallback = std::move(callback);
    return *this;
}

Widget& Widget::onEvent(EventType type, std::function<void(WidgetEvent&)> callback) {
    m_data->eventCallbacks[type] = std::move(callback);
    return *this;
}


// Event Dispatch
bool Widget::dispatchEvent(WidgetEvent& event) {
    if (!m_data->enabled) {
        return false;
    }

    event.target = this;

    // Check for specific event callback
    auto it = m_data->eventCallbacks.find(event.type);
    if (it != m_data->eventCallbacks.end() && it->second) {
        it->second(event);
        if (event.handled) {
            return true;
        }
    }

    // Handle built-in event types
    switch (event.type) {
        case EventType::Click:
            if (m_data->onClickCallback) {
                m_data->onClickCallback();
                event.handled = true;
            }
            break;
        case EventType::Hover:
            if (m_data->onHoverCallback) {
                m_data->onHoverCallback(m_data->hovered);
                event.handled = true;
            }
            break;
        case EventType::Focus:
            if (m_data->onFocusCallback) {
                m_data->onFocusCallback(m_data->focused);
                event.handled = true;
            }
            break;
        default:
            break;
    }

    // Event bubbling to parent
    if (event.bubbles && !event.handled && m_data->parent) {
        return m_data->parent->dispatchEvent(event);
    }

    return event.handled;
}

void Widget::triggerClick() {
    if (!m_data->enabled) {
        return;
    }

    WidgetEvent event;
    event.type = EventType::Click;
    dispatchEvent(event);
}

void Widget::setHovered(bool hovered) {
    if (m_data->hovered != hovered) {
        m_data->hovered = hovered;
        if (m_data->onHoverCallback) {
            m_data->onHoverCallback(hovered);
        }
        WidgetEvent event;
        event.type = EventType::Hover;
        dispatchEvent(event);
    }
}

void Widget::setFocused(bool focused) {
    if (m_data->focused != focused) {
        m_data->focused = focused;
        if (m_data->onFocusCallback) {
            m_data->onFocusCallback(focused);
        }
        WidgetEvent event;
        event.type = EventType::Focus;
        dispatchEvent(event);
    }
}

void Widget::setPressed(bool pressed) {
    m_data->pressed = pressed;
}

// Hierarchy
Widget& Widget::parent(Widget* p) {
    m_data->parent = p;
    return *this;
}

Widget* Widget::getParent() const {
    return m_data->parent;
}

const std::vector<Widget*>& Widget::getChildren() const {
    return m_data->children;
}

void Widget::addChild(Widget* child) {
    if (child) {
        m_data->children.push_back(child);
        child->parent(this);
    }
}

void Widget::removeChild(Widget* child) {
    if (child) {
        auto it = std::find(m_data->children.begin(), m_data->children.end(), child);
        if (it != m_data->children.end()) {
            (*it)->parent(nullptr);
            m_data->children.erase(it);
        }
    }
}

// Property access
Widget& Widget::setPropertyFloat(const std::string& name, float value) {
    m_data->customProperties[name] = value;
    return *this;
}

Widget& Widget::setPropertyInt(const std::string& name, int value) {
    m_data->customProperties[name] = value;
    return *this;
}

Widget& Widget::setPropertyBool(const std::string& name, bool value) {
    m_data->customProperties[name] = value;
    return *this;
}

Widget& Widget::setPropertyString(const std::string& name, const std::string& value) {
    m_data->customProperties[name] = value;
    return *this;
}

float Widget::getPropertyFloat(const std::string& name, float defaultValue) const {
    auto it = m_data->customProperties.find(name);
    if (it != m_data->customProperties.end()) {
        try {
            return std::any_cast<float>(it->second);
        } catch (const std::bad_any_cast&) {
            return defaultValue;
        }
    }
    return defaultValue;
}

int Widget::getPropertyInt(const std::string& name, int defaultValue) const {
    auto it = m_data->customProperties.find(name);
    if (it != m_data->customProperties.end()) {
        try {
            return std::any_cast<int>(it->second);
        } catch (const std::bad_any_cast&) {
            return defaultValue;
        }
    }
    return defaultValue;
}

bool Widget::getPropertyBool(const std::string& name, bool defaultValue) const {
    auto it = m_data->customProperties.find(name);
    if (it != m_data->customProperties.end()) {
        try {
            return std::any_cast<bool>(it->second);
        } catch (const std::bad_any_cast&) {
            return defaultValue;
        }
    }
    return defaultValue;
}

std::string Widget::getPropertyString(const std::string& name, const std::string& defaultValue) const {
    auto it = m_data->customProperties.find(name);
    if (it != m_data->customProperties.end()) {
        try {
            return std::any_cast<std::string>(it->second);
        } catch (const std::bad_any_cast&) {
            return defaultValue;
        }
    }
    return defaultValue;
}

bool Widget::hasProperty(const std::string& name) const {
    return m_data->customProperties.find(name) != m_data->customProperties.end();
}


// State Management
WidgetState Widget::getState() const {
    WidgetState state;
    state.id = m_data->id;
    state.visible = m_data->visible;
    state.enabled = m_data->enabled;
    state.focused = m_data->focused;
    state.hovered = m_data->hovered;
    state.pressed = m_data->pressed;
    state.bounds = Rect(0, 0, m_data->width, m_data->height);
    state.properties = m_data->customProperties;
    
    // Store standard properties
    state.properties["width"] = m_data->width;
    state.properties["height"] = m_data->height;
    state.properties["opacity"] = m_data->opacity;
    state.properties["borderRadius"] = m_data->borderRadius;
    state.properties["borderWidth"] = m_data->borderWidth;
    state.properties["blurRadius"] = m_data->blurRadius;
    
    return state;
}

void Widget::setState(const WidgetState& state) {
    m_data->id = state.id;
    m_data->visible = state.visible;
    m_data->enabled = state.enabled;
    m_data->focused = state.focused;
    m_data->hovered = state.hovered;
    m_data->pressed = state.pressed;
    m_data->width = state.bounds.width;
    m_data->height = state.bounds.height;
    
    // Restore standard properties if present
    if (state.properties.count("opacity")) {
        try {
            m_data->opacity = std::any_cast<float>(state.properties.at("opacity"));
        } catch (...) {}
    }
    if (state.properties.count("borderRadius")) {
        try {
            m_data->borderRadius = std::any_cast<float>(state.properties.at("borderRadius"));
        } catch (...) {}
    }
    if (state.properties.count("borderWidth")) {
        try {
            m_data->borderWidth = std::any_cast<float>(state.properties.at("borderWidth"));
        } catch (...) {}
    }
    if (state.properties.count("blurRadius")) {
        try {
            m_data->blurRadius = std::any_cast<float>(state.properties.at("blurRadius"));
        } catch (...) {}
    }
    
    // Copy custom properties (excluding standard ones)
    for (const auto& [key, value] : state.properties) {
        if (key != "width" && key != "height" && key != "opacity" && 
            key != "borderRadius" && key != "borderWidth" && key != "blurRadius") {
            m_data->customProperties[key] = value;
        }
    }
}

// =============================================================================
// WidgetState Implementation
// =============================================================================

std::string WidgetState::toJson() const {
    std::ostringstream oss;
    oss << "{";
    oss << "\"id\":\"" << id << "\",";
    oss << "\"visible\":" << (visible ? "true" : "false") << ",";
    oss << "\"enabled\":" << (enabled ? "true" : "false") << ",";
    oss << "\"focused\":" << (focused ? "true" : "false") << ",";
    oss << "\"hovered\":" << (hovered ? "true" : "false") << ",";
    oss << "\"pressed\":" << (pressed ? "true" : "false") << ",";
    oss << "\"bounds\":{";
    oss << "\"x\":" << bounds.x << ",";
    oss << "\"y\":" << bounds.y << ",";
    oss << "\"width\":" << bounds.width << ",";
    oss << "\"height\":" << bounds.height;
    oss << "}";
    
    // Serialize numeric properties from the properties map
    for (const auto& [key, value] : properties) {
        try {
            if (value.type() == typeid(float)) {
                oss << ",\"" << key << "\":" << std::any_cast<float>(value);
            } else if (value.type() == typeid(int)) {
                oss << ",\"" << key << "\":" << std::any_cast<int>(value);
            } else if (value.type() == typeid(bool)) {
                oss << ",\"" << key << "\":" << (std::any_cast<bool>(value) ? "true" : "false");
            } else if (value.type() == typeid(std::string)) {
                oss << ",\"" << key << "\":\"" << std::any_cast<std::string>(value) << "\"";
            }
        } catch (...) {
            // Skip properties that can't be serialized
        }
    }
    
    oss << "}";
    return oss.str();
}


// Helper function to skip whitespace
static size_t skipWhitespace(const std::string& json, size_t pos) {
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || 
           json[pos] == '\n' || json[pos] == '\r')) {
        ++pos;
    }
    return pos;
}

// Helper function to parse a string value
static std::string parseString(const std::string& json, size_t& pos) {
    pos = skipWhitespace(json, pos);
    if (pos >= json.size() || json[pos] != '"') {
        return "";
    }
    ++pos; // Skip opening quote
    
    std::string result;
    while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\' && pos + 1 < json.size()) {
            ++pos;
            switch (json[pos]) {
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                case 'r': result += '\r'; break;
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                default: result += json[pos]; break;
            }
        } else {
            result += json[pos];
        }
        ++pos;
    }
    if (pos < json.size()) ++pos; // Skip closing quote
    return result;
}

// Helper function to parse a number
static float parseNumber(const std::string& json, size_t& pos) {
    pos = skipWhitespace(json, pos);
    size_t start = pos;
    
    if (pos < json.size() && json[pos] == '-') ++pos;
    while (pos < json.size() && (std::isdigit(json[pos]) || json[pos] == '.')) {
        ++pos;
    }
    
    if (start == pos) return 0.0f;
    return std::stof(json.substr(start, pos - start));
}

// Helper function to parse a boolean
static bool parseBool(const std::string& json, size_t& pos) {
    pos = skipWhitespace(json, pos);
    if (json.substr(pos, 4) == "true") {
        pos += 4;
        return true;
    } else if (json.substr(pos, 5) == "false") {
        pos += 5;
        return false;
    }
    return false;
}

WidgetState WidgetState::fromJson(const std::string& json) {
    WidgetState state;
    size_t pos = 0;
    
    pos = skipWhitespace(json, pos);
    if (pos >= json.size() || json[pos] != '{') {
        return state;
    }
    ++pos;
    
    while (pos < json.size() && json[pos] != '}') {
        pos = skipWhitespace(json, pos);
        if (json[pos] == ',') {
            ++pos;
            continue;
        }
        
        // Parse key
        std::string key = parseString(json, pos);
        if (key.empty()) break;
        
        pos = skipWhitespace(json, pos);
        if (pos >= json.size() || json[pos] != ':') break;
        ++pos;
        pos = skipWhitespace(json, pos);
        
        // Parse value based on key
        if (key == "id") {
            state.id = parseString(json, pos);
        } else if (key == "visible") {
            state.visible = parseBool(json, pos);
        } else if (key == "enabled") {
            state.enabled = parseBool(json, pos);
        } else if (key == "focused") {
            state.focused = parseBool(json, pos);
        } else if (key == "hovered") {
            state.hovered = parseBool(json, pos);
        } else if (key == "pressed") {
            state.pressed = parseBool(json, pos);
        } else if (key == "bounds") {
            // Parse bounds object
            pos = skipWhitespace(json, pos);
            if (json[pos] == '{') {
                ++pos;
                while (pos < json.size() && json[pos] != '}') {
                    pos = skipWhitespace(json, pos);
                    if (json[pos] == ',') { ++pos; continue; }
                    
                    std::string boundsKey = parseString(json, pos);
                    pos = skipWhitespace(json, pos);
                    if (json[pos] == ':') ++pos;
                    
                    float value = parseNumber(json, pos);
                    if (boundsKey == "x") state.bounds.x = value;
                    else if (boundsKey == "y") state.bounds.y = value;
                    else if (boundsKey == "width") state.bounds.width = value;
                    else if (boundsKey == "height") state.bounds.height = value;
                }
                if (pos < json.size()) ++pos; // Skip '}'
            }
        } else {
            // Parse as property - check type
            pos = skipWhitespace(json, pos);
            if (json[pos] == '"') {
                state.properties[key] = parseString(json, pos);
            } else if (json[pos] == 't' || json[pos] == 'f') {
                state.properties[key] = parseBool(json, pos);
            } else {
                state.properties[key] = parseNumber(json, pos);
            }
        }
    }
    
    return state;
}

bool WidgetState::operator==(const WidgetState& other) const {
    // Compare basic fields
    if (id != other.id || visible != other.visible || enabled != other.enabled ||
        focused != other.focused || hovered != other.hovered || pressed != other.pressed) {
        return false;
    }
    
    // Compare bounds
    if (bounds.x != other.bounds.x || bounds.y != other.bounds.y ||
        bounds.width != other.bounds.width || bounds.height != other.bounds.height) {
        return false;
    }
    
    // Compare properties count
    if (properties.size() != other.properties.size()) {
        return false;
    }
    
    // Compare property keys (values comparison is complex due to std::any)
    for (const auto& [key, value] : properties) {
        if (other.properties.find(key) == other.properties.end()) {
            return false;
        }
    }
    
    return true;
}

} // namespace KillerGK
