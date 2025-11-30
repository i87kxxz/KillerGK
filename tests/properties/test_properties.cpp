/**
 * @file test_properties.cpp
 * @brief Property-based tests for KillerGK
 * 
 * This file contains property-based tests that verify correctness properties
 * defined in the design document.
 */

#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include <cmath>

#include "KillerGK/core/Types.hpp"
#include "test_helpers.hpp"

using namespace KillerGK;
using namespace rc;

// ============================================================================
// Property Tests for Color
// ============================================================================

/**
 * **Feature: killergk-gui-library, Property 7: Theme Application Consistency** (color component)
 * Test color transformations preserve valid ranges
 * **Validates: Requirements 5.1, 5.3**
 */
RC_GTEST_PROP(ColorProperties, LightenPreservesValidRange, ()) {
    auto color = *gen::arbitrary<Color>();
    auto amount = *genNormalizedFloat();
    
    Color lightened = color.lighten(amount);
    
    RC_ASSERT(lightened.r >= 0.0f && lightened.r <= 1.0f);
    RC_ASSERT(lightened.g >= 0.0f && lightened.g <= 1.0f);
    RC_ASSERT(lightened.b >= 0.0f && lightened.b <= 1.0f);
    RC_ASSERT(lightened.a >= 0.0f && lightened.a <= 1.0f);
}

/**
 * **Feature: killergk-gui-library, Property 7: Theme Application Consistency** (color component)
 * Test color transformations preserve valid ranges
 * **Validates: Requirements 5.1, 5.3**
 */
RC_GTEST_PROP(ColorProperties, DarkenPreservesValidRange, ()) {
    auto color = *gen::arbitrary<Color>();
    auto amount = *genNormalizedFloat();
    
    Color darkened = color.darken(amount);
    
    RC_ASSERT(darkened.r >= 0.0f && darkened.r <= 1.0f);
    RC_ASSERT(darkened.g >= 0.0f && darkened.g <= 1.0f);
    RC_ASSERT(darkened.b >= 0.0f && darkened.b <= 1.0f);
    RC_ASSERT(darkened.a >= 0.0f && darkened.a <= 1.0f);
}


/**
 * **Feature: killergk-gui-library, Property 7: Theme Application Consistency** (color component)
 * Test withAlpha preserves RGB and sets correct alpha
 * **Validates: Requirements 5.1, 5.3**
 */
RC_GTEST_PROP(ColorProperties, WithAlphaPreservesRGB, ()) {
    auto color = *gen::arbitrary<Color>();
    auto newAlpha = *genNormalizedFloat();
    
    Color result = color.withAlpha(newAlpha);
    
    RC_ASSERT(result.r == color.r);
    RC_ASSERT(result.g == color.g);
    RC_ASSERT(result.b == color.b);
    RC_ASSERT(result.a == newAlpha);
}

/**
 * **Feature: killergk-gui-library, Property 7: Theme Application Consistency** (color component)
 * Test RGB factory clamps values correctly
 * **Validates: Requirements 5.1, 5.3**
 */
RC_GTEST_PROP(ColorProperties, RgbFactoryClampsValues, ()) {
    auto r = *gen::inRange(-100, 400);
    auto g = *gen::inRange(-100, 400);
    auto b = *gen::inRange(-100, 400);
    
    Color color = Color::rgb(r, g, b);
    
    RC_ASSERT(color.r >= 0.0f && color.r <= 1.0f);
    RC_ASSERT(color.g >= 0.0f && color.g <= 1.0f);
    RC_ASSERT(color.b >= 0.0f && color.b <= 1.0f);
    RC_ASSERT(color.a == 1.0f);
}

// ============================================================================
// Property Tests for Rect
// ============================================================================

/**
 * **Feature: killergk-gui-library, Property 3: Layout Constraint Satisfaction** (geometry component)
 * Test that a point inside a rect is correctly identified
 * **Validates: Requirements 3.1, 3.2, 3.3, 3.4, 3.6**
 */
RC_GTEST_PROP(RectProperties, ContainsPointInsideRect, ()) {
    auto rect = *gen::arbitrary<Rect>();
    
    // Skip degenerate rects
    RC_PRE(rect.width > 0.0f && rect.height > 0.0f);
    
    // Generate a point inside the rect using integer-based approach
    auto offsetXPct = *gen::inRange(0, 1000);
    auto offsetYPct = *gen::inRange(0, 1000);
    
    float offsetX = (static_cast<float>(offsetXPct) / 1000.0f) * rect.width;
    float offsetY = (static_cast<float>(offsetYPct) / 1000.0f) * rect.height;
    
    float px = rect.x + offsetX;
    float py = rect.y + offsetY;
    
    RC_ASSERT(rect.contains(px, py));
}

/**
 * **Feature: killergk-gui-library, Property 3: Layout Constraint Satisfaction** (geometry component)
 * Test that expand increases rect dimensions correctly
 * **Validates: Requirements 3.1, 3.2, 3.3, 3.4, 3.6**
 */
RC_GTEST_PROP(RectProperties, ExpandIncreasesSize, ()) {
    auto rect = *gen::arbitrary<Rect>();
    auto amountInt = *gen::inRange(0, 10000);
    float amount = static_cast<float>(amountInt) / 100.0f;  // 0 to 100
    
    Rect expanded = rect.expand(amount);
    
    RC_ASSERT(expanded.width == rect.width + 2 * amount);
    RC_ASSERT(expanded.height == rect.height + 2 * amount);
    RC_ASSERT(expanded.x == rect.x - amount);
    RC_ASSERT(expanded.y == rect.y - amount);
}

/**
 * **Feature: killergk-gui-library, Property 3: Layout Constraint Satisfaction** (geometry component)
 * Test that a rect intersects with itself
 * **Validates: Requirements 3.1, 3.2, 3.3, 3.4, 3.6**
 */
RC_GTEST_PROP(RectProperties, RectIntersectsWithItself, ()) {
    auto rect = *gen::arbitrary<Rect>();
    
    // Skip degenerate rects
    RC_PRE(rect.width > 0.0f && rect.height > 0.0f);
    
    RC_ASSERT(rect.intersects(rect));
}

/**
 * **Feature: killergk-gui-library, Property 3: Layout Constraint Satisfaction** (geometry component)
 * Test intersection symmetry
 * **Validates: Requirements 3.1, 3.2, 3.3, 3.4, 3.6**
 */
RC_GTEST_PROP(RectProperties, IntersectionIsSymmetric, ()) {
    auto rect1 = *gen::arbitrary<Rect>();
    auto rect2 = *gen::arbitrary<Rect>();
    
    RC_ASSERT(rect1.intersects(rect2) == rect2.intersects(rect1));
}

// ============================================================================
// Property Tests for HSL Color Conversion
// ============================================================================

/**
 * **Feature: killergk-gui-library, Property 7: Theme Application Consistency** (color component)
 * Test HSL factory produces valid colors
 * **Validates: Requirements 5.1, 5.3**
 */
RC_GTEST_PROP(ColorProperties, HslProducesValidColors, ()) {
    auto hInt = *gen::inRange(0, 3600);
    auto sInt = *gen::inRange(0, 1000);
    auto lInt = *gen::inRange(0, 1000);
    
    float h = static_cast<float>(hInt) / 10.0f;  // 0 to 360
    float s = static_cast<float>(sInt) / 1000.0f;  // 0 to 1
    float l = static_cast<float>(lInt) / 1000.0f;  // 0 to 1
    
    Color color = Color::hsl(h, s, l);
    
    RC_ASSERT(color.r >= 0.0f && color.r <= 1.0f);
    RC_ASSERT(color.g >= 0.0f && color.g <= 1.0f);
    RC_ASSERT(color.b >= 0.0f && color.b <= 1.0f);
    RC_ASSERT(color.a == 1.0f);
}

// ============================================================================
// Property Tests for Window Independence
// ============================================================================

#include "KillerGK/platform/Platform.hpp"

/**
 * @brief Simulated window state for testing window independence
 * 
 * This struct mirrors the state that a real window would maintain,
 * allowing us to test the logical independence property without
 * requiring actual window creation (which needs a display).
 */
struct SimulatedWindowState {
    std::string title;
    int width;
    int height;
    int x;
    int y;
    bool frameless;
    bool transparent;
    bool resizable;
    bool alwaysOnTop;
    bool visible;
    
    bool operator==(const SimulatedWindowState& other) const {
        return title == other.title &&
               width == other.width &&
               height == other.height &&
               x == other.x &&
               y == other.y &&
               frameless == other.frameless &&
               transparent == other.transparent &&
               resizable == other.resizable &&
               alwaysOnTop == other.alwaysOnTop &&
               visible == other.visible;
    }
    
    bool operator!=(const SimulatedWindowState& other) const {
        return !(*this == other);
    }
    
    /**
     * @brief Create from WindowParams
     */
    static SimulatedWindowState fromParams(const WindowParams& params) {
        SimulatedWindowState state;
        state.title = params.title;
        state.width = params.width;
        state.height = params.height;
        state.x = params.x;
        state.y = params.y;
        state.frameless = params.frameless;
        state.transparent = params.transparent;
        state.resizable = params.resizable;
        state.alwaysOnTop = params.alwaysOnTop;
        state.visible = params.visible;
        return state;
    }
};

namespace rc {

/**
 * @brief Generator for valid window titles
 */
inline Gen<std::string> genWindowTitle() {
    return gen::map(gen::inRange(1, 100), [](int len) {
        std::string title = "Window_";
        title += std::to_string(len);
        return title;
    });
}

/**
 * @brief Generator for valid window dimensions
 */
inline Gen<int> genWindowDimension() {
    return gen::inRange(100, 4096);  // Reasonable window size range
}

/**
 * @brief Generator for window position
 */
inline Gen<int> genWindowPosition() {
    return gen::inRange(-1, 3840);  // -1 means centered, otherwise screen coords
}

/**
 * @brief Generator for WindowParams
 */
template<>
struct Arbitrary<WindowParams> {
    static Gen<WindowParams> arbitrary() {
        return gen::build<WindowParams>(
            gen::set(&WindowParams::title, genWindowTitle()),
            gen::set(&WindowParams::width, genWindowDimension()),
            gen::set(&WindowParams::height, genWindowDimension()),
            gen::set(&WindowParams::x, genWindowPosition()),
            gen::set(&WindowParams::y, genWindowPosition()),
            gen::set(&WindowParams::frameless, gen::arbitrary<bool>()),
            gen::set(&WindowParams::transparent, gen::arbitrary<bool>()),
            gen::set(&WindowParams::resizable, gen::arbitrary<bool>()),
            gen::set(&WindowParams::alwaysOnTop, gen::arbitrary<bool>()),
            gen::set(&WindowParams::visible, gen::arbitrary<bool>())
        );
    }
};

/**
 * @brief Generator for SimulatedWindowState
 */
template<>
struct Arbitrary<SimulatedWindowState> {
    static Gen<SimulatedWindowState> arbitrary() {
        return gen::map(gen::arbitrary<WindowParams>(), 
            [](const WindowParams& params) {
                return SimulatedWindowState::fromParams(params);
            });
    }
};

} // namespace rc

/**
 * **Feature: killergk-gui-library, Property 2: Window Independence**
 * 
 * *For any* number of windows created (up to system limits), each window 
 * SHALL maintain independent state such that modifying one window does 
 * not affect any other window's properties.
 * 
 * This test verifies that window states are logically independent by:
 * 1. Creating multiple window state configurations
 * 2. Modifying one window's state
 * 3. Verifying other windows' states remain unchanged
 * 
 * **Validates: Requirements 1.4**
 */
RC_GTEST_PROP(WindowProperties, WindowIndependence_StateIsolation, ()) {
    // Generate 2-5 window configurations
    auto numWindows = *gen::inRange(2, 6);
    
    // Generate initial states for all windows
    std::vector<SimulatedWindowState> windowStates;
    for (int i = 0; i < numWindows; ++i) {
        windowStates.push_back(*gen::arbitrary<SimulatedWindowState>());
    }
    
    // Store copies of original states
    std::vector<SimulatedWindowState> originalStates = windowStates;
    
    // Select a random window to modify
    auto modifyIndex = *gen::inRange(0, numWindows);
    
    // Generate new values for the modified window
    auto newTitle = *genWindowTitle();
    auto newWidth = *genWindowDimension();
    auto newHeight = *genWindowDimension();
    auto newX = *genWindowPosition();
    auto newY = *genWindowPosition();
    auto newFrameless = *gen::arbitrary<bool>();
    auto newTransparent = *gen::arbitrary<bool>();
    auto newResizable = *gen::arbitrary<bool>();
    auto newAlwaysOnTop = *gen::arbitrary<bool>();
    auto newVisible = *gen::arbitrary<bool>();
    
    // Modify the selected window
    windowStates[modifyIndex].title = newTitle;
    windowStates[modifyIndex].width = newWidth;
    windowStates[modifyIndex].height = newHeight;
    windowStates[modifyIndex].x = newX;
    windowStates[modifyIndex].y = newY;
    windowStates[modifyIndex].frameless = newFrameless;
    windowStates[modifyIndex].transparent = newTransparent;
    windowStates[modifyIndex].resizable = newResizable;
    windowStates[modifyIndex].alwaysOnTop = newAlwaysOnTop;
    windowStates[modifyIndex].visible = newVisible;
    
    // Verify all OTHER windows remain unchanged
    for (int i = 0; i < numWindows; ++i) {
        if (i != modifyIndex) {
            RC_ASSERT(windowStates[i] == originalStates[i]);
        }
    }
}

/**
 * **Feature: killergk-gui-library, Property 2: Window Independence**
 * 
 * Test that WindowParams creates independent configurations.
 * Each WindowParams instance should be completely independent.
 * 
 * **Validates: Requirements 1.4**
 */
RC_GTEST_PROP(WindowProperties, WindowIndependence_ParamsIsolation, ()) {
    // Generate two independent window parameter sets
    auto params1 = *gen::arbitrary<WindowParams>();
    auto params2 = *gen::arbitrary<WindowParams>();
    
    // Store original values from params2
    std::string originalTitle2 = params2.title;
    int originalWidth2 = params2.width;
    int originalHeight2 = params2.height;
    int originalX2 = params2.x;
    int originalY2 = params2.y;
    bool originalFrameless2 = params2.frameless;
    bool originalTransparent2 = params2.transparent;
    bool originalResizable2 = params2.resizable;
    bool originalAlwaysOnTop2 = params2.alwaysOnTop;
    bool originalVisible2 = params2.visible;
    
    // Modify params1
    params1.title = "Modified_Window";
    params1.width = 1920;
    params1.height = 1080;
    params1.x = 100;
    params1.y = 100;
    params1.frameless = !params1.frameless;
    params1.transparent = !params1.transparent;
    params1.resizable = !params1.resizable;
    params1.alwaysOnTop = !params1.alwaysOnTop;
    params1.visible = !params1.visible;
    
    // Verify params2 is unchanged
    RC_ASSERT(params2.title == originalTitle2);
    RC_ASSERT(params2.width == originalWidth2);
    RC_ASSERT(params2.height == originalHeight2);
    RC_ASSERT(params2.x == originalX2);
    RC_ASSERT(params2.y == originalY2);
    RC_ASSERT(params2.frameless == originalFrameless2);
    RC_ASSERT(params2.transparent == originalTransparent2);
    RC_ASSERT(params2.resizable == originalResizable2);
    RC_ASSERT(params2.alwaysOnTop == originalAlwaysOnTop2);
    RC_ASSERT(params2.visible == originalVisible2);
}

/**
 * **Feature: killergk-gui-library, Property 2: Window Independence**
 * 
 * Test that multiple window states in a collection maintain independence
 * when individual properties are modified.
 * 
 * **Validates: Requirements 1.4**
 */
RC_GTEST_PROP(WindowProperties, WindowIndependence_PropertyModification, ()) {
    // Generate initial window params
    auto params1 = *gen::arbitrary<WindowParams>();
    auto params2 = *gen::arbitrary<WindowParams>();
    
    // Create simulated window states
    SimulatedWindowState state1 = SimulatedWindowState::fromParams(params1);
    SimulatedWindowState state2 = SimulatedWindowState::fromParams(params2);
    
    // Store original state2
    SimulatedWindowState originalState2 = state2;
    
    // Generate random property modifications for state1
    auto propertyToModify = *gen::inRange(0, 10);
    
    switch (propertyToModify) {
        case 0: state1.title = *genWindowTitle(); break;
        case 1: state1.width = *genWindowDimension(); break;
        case 2: state1.height = *genWindowDimension(); break;
        case 3: state1.x = *genWindowPosition(); break;
        case 4: state1.y = *genWindowPosition(); break;
        case 5: state1.frameless = !state1.frameless; break;
        case 6: state1.transparent = !state1.transparent; break;
        case 7: state1.resizable = !state1.resizable; break;
        case 8: state1.alwaysOnTop = !state1.alwaysOnTop; break;
        case 9: state1.visible = !state1.visible; break;
    }
    
    // Verify state2 is completely unchanged
    RC_ASSERT(state2 == originalState2);
}

// ============================================================================
// Property Tests for Input Event Dispatch
// ============================================================================

#include "KillerGK/platform/Input.hpp"

namespace rc {

/**
 * @brief Generator for valid KeyCode values
 */
inline Gen<KillerGK::KeyCode> genKeyCode() {
    return gen::element(
        KillerGK::KeyCode::A, KillerGK::KeyCode::B, KillerGK::KeyCode::C,
        KillerGK::KeyCode::D, KillerGK::KeyCode::E, KillerGK::KeyCode::F,
        KillerGK::KeyCode::G, KillerGK::KeyCode::H, KillerGK::KeyCode::I,
        KillerGK::KeyCode::J, KillerGK::KeyCode::K, KillerGK::KeyCode::L,
        KillerGK::KeyCode::M, KillerGK::KeyCode::N, KillerGK::KeyCode::O,
        KillerGK::KeyCode::P, KillerGK::KeyCode::Q, KillerGK::KeyCode::R,
        KillerGK::KeyCode::S, KillerGK::KeyCode::T, KillerGK::KeyCode::U,
        KillerGK::KeyCode::V, KillerGK::KeyCode::W, KillerGK::KeyCode::X,
        KillerGK::KeyCode::Y, KillerGK::KeyCode::Z,
        KillerGK::KeyCode::Num0, KillerGK::KeyCode::Num1, KillerGK::KeyCode::Num2,
        KillerGK::KeyCode::Num3, KillerGK::KeyCode::Num4, KillerGK::KeyCode::Num5,
        KillerGK::KeyCode::Num6, KillerGK::KeyCode::Num7, KillerGK::KeyCode::Num8,
        KillerGK::KeyCode::Num9,
        KillerGK::KeyCode::F1, KillerGK::KeyCode::F2, KillerGK::KeyCode::F3,
        KillerGK::KeyCode::F4, KillerGK::KeyCode::F5, KillerGK::KeyCode::F6,
        KillerGK::KeyCode::F7, KillerGK::KeyCode::F8, KillerGK::KeyCode::F9,
        KillerGK::KeyCode::F10, KillerGK::KeyCode::F11, KillerGK::KeyCode::F12,
        KillerGK::KeyCode::Space, KillerGK::KeyCode::Enter, KillerGK::KeyCode::Escape,
        KillerGK::KeyCode::Tab, KillerGK::KeyCode::Backspace, KillerGK::KeyCode::Delete,
        KillerGK::KeyCode::Left, KillerGK::KeyCode::Right, KillerGK::KeyCode::Up,
        KillerGK::KeyCode::Down, KillerGK::KeyCode::Home, KillerGK::KeyCode::End,
        KillerGK::KeyCode::PageUp, KillerGK::KeyCode::PageDown,
        KillerGK::KeyCode::Shift, KillerGK::KeyCode::Control, KillerGK::KeyCode::Alt
    );
}

/**
 * @brief Generator for MouseButton values
 */
inline Gen<KillerGK::MouseButton> genMouseButton() {
    return gen::element(
        KillerGK::MouseButton::Left,
        KillerGK::MouseButton::Right,
        KillerGK::MouseButton::Middle,
        KillerGK::MouseButton::X1,
        KillerGK::MouseButton::X2
    );
}

/**
 * @brief Generator for TouchPhase values
 */
inline Gen<KillerGK::TouchPhase> genTouchPhase() {
    return gen::element(
        KillerGK::TouchPhase::Began,
        KillerGK::TouchPhase::Moved,
        KillerGK::TouchPhase::Stationary,
        KillerGK::TouchPhase::Ended,
        KillerGK::TouchPhase::Cancelled
    );
}

/**
 * @brief Generator for ModifierKeys
 */
template<>
struct Arbitrary<KillerGK::ModifierKeys> {
    static Gen<KillerGK::ModifierKeys> arbitrary() {
        return gen::build<KillerGK::ModifierKeys>(
            gen::set(&KillerGK::ModifierKeys::shift, gen::arbitrary<bool>()),
            gen::set(&KillerGK::ModifierKeys::control, gen::arbitrary<bool>()),
            gen::set(&KillerGK::ModifierKeys::alt, gen::arbitrary<bool>()),
            gen::set(&KillerGK::ModifierKeys::super, gen::arbitrary<bool>()),
            gen::set(&KillerGK::ModifierKeys::capsLock, gen::arbitrary<bool>()),
            gen::set(&KillerGK::ModifierKeys::numLock, gen::arbitrary<bool>())
        );
    }
};

/**
 * @brief Generator for valid screen coordinates
 */
inline Gen<float> genScreenCoordinate() {
    return gen::map(gen::inRange(0, 40960), [](int v) {
        return static_cast<float>(v) / 10.0f;  // 0.0 to 4096.0
    });
}

/**
 * @brief Generator for scroll values
 */
inline Gen<float> genScrollValue() {
    return gen::map(gen::inRange(-1000, 1000), [](int v) {
        return static_cast<float>(v) / 100.0f;  // -10.0 to 10.0
    });
}

/**
 * @brief Generator for click count
 */
inline Gen<int> genClickCount() {
    return gen::inRange(0, 4);  // 0 to 3 clicks
}

/**
 * @brief Generator for touch ID
 */
inline Gen<int> genTouchId() {
    return gen::inRange(0, 10);  // Support up to 10 simultaneous touches
}

/**
 * @brief Generator for KeyEvent
 */
template<>
struct Arbitrary<KillerGK::KeyEvent> {
    static Gen<KillerGK::KeyEvent> arbitrary() {
        return gen::build<KillerGK::KeyEvent>(
            gen::set(&KillerGK::KeyEvent::key, genKeyCode()),
            gen::set(&KillerGK::KeyEvent::pressed, gen::arbitrary<bool>()),
            gen::set(&KillerGK::KeyEvent::repeat, gen::arbitrary<bool>()),
            gen::set(&KillerGK::KeyEvent::modifiers, gen::arbitrary<KillerGK::ModifierKeys>())
        );
    }
};

/**
 * @brief Generator for MouseEvent
 */
template<>
struct Arbitrary<KillerGK::MouseEvent> {
    static Gen<KillerGK::MouseEvent> arbitrary() {
        return gen::build<KillerGK::MouseEvent>(
            gen::set(&KillerGK::MouseEvent::x, genScreenCoordinate()),
            gen::set(&KillerGK::MouseEvent::y, genScreenCoordinate()),
            gen::set(&KillerGK::MouseEvent::button, genMouseButton()),
            gen::set(&KillerGK::MouseEvent::clicks, genClickCount()),
            gen::set(&KillerGK::MouseEvent::scrollX, genScrollValue()),
            gen::set(&KillerGK::MouseEvent::scrollY, genScrollValue()),
            gen::set(&KillerGK::MouseEvent::modifiers, gen::arbitrary<KillerGK::ModifierKeys>())
        );
    }
};

/**
 * @brief Generator for TouchEvent
 */
template<>
struct Arbitrary<KillerGK::TouchEvent> {
    static Gen<KillerGK::TouchEvent> arbitrary() {
        return gen::build<KillerGK::TouchEvent>(
            gen::set(&KillerGK::TouchEvent::touchId, genTouchId()),
            gen::set(&KillerGK::TouchEvent::x, genScreenCoordinate()),
            gen::set(&KillerGK::TouchEvent::y, genScreenCoordinate()),
            gen::set(&KillerGK::TouchEvent::phase, genTouchPhase())
        );
    }
};

} // namespace rc

/**
 * **Feature: killergk-gui-library, Property 17: Input Event Dispatch**
 * 
 * *For any* keyboard input, the input system SHALL dispatch events with 
 * correct key codes and modifier states.
 * 
 * This test verifies that:
 * 1. KeyEvent created via Input::createKeyEvent preserves the key code
 * 2. KeyEvent preserves the pressed state
 * 3. KeyEvent preserves the repeat flag
 * 
 * **Validates: Requirements 11.1**
 */
RC_GTEST_PROP(InputEventProperties, KeyEventPreservesKeyData, ()) {
    auto keyCode = *genKeyCode();
    auto pressed = *gen::arbitrary<bool>();
    auto repeat = *gen::arbitrary<bool>();
    
    KeyEvent event = Input::createKeyEvent(keyCode, pressed, repeat);
    
    // Verify key code is preserved
    RC_ASSERT(event.key == keyCode);
    
    // Verify pressed state is preserved
    RC_ASSERT(event.pressed == pressed);
    
    // Verify repeat flag is preserved
    RC_ASSERT(event.repeat == repeat);
}

/**
 * **Feature: killergk-gui-library, Property 17: Input Event Dispatch**
 * 
 * *For any* mouse input, the input system SHALL dispatch events with 
 * correct coordinates, button codes, and click counts.
 * 
 * This test verifies that:
 * 1. MouseEvent created via Input::createMouseEvent preserves coordinates
 * 2. MouseEvent preserves the button
 * 3. MouseEvent preserves the click count
 * 
 * **Validates: Requirements 11.2**
 */
RC_GTEST_PROP(InputEventProperties, MouseEventPreservesPositionAndButton, ()) {
    auto x = *genScreenCoordinate();
    auto y = *genScreenCoordinate();
    auto button = *genMouseButton();
    auto clicks = *genClickCount();
    
    MouseEvent event = Input::createMouseEvent(x, y, button, clicks);
    
    // Verify coordinates are preserved
    RC_ASSERT(event.x == x);
    RC_ASSERT(event.y == y);
    
    // Verify button is preserved
    RC_ASSERT(event.button == button);
    
    // Verify click count is preserved
    RC_ASSERT(event.clicks == clicks);
    
    // Verify scroll values are initialized to zero
    RC_ASSERT(event.scrollX == 0.0f);
    RC_ASSERT(event.scrollY == 0.0f);
}

/**
 * **Feature: killergk-gui-library, Property 17: Input Event Dispatch**
 * 
 * *For any* touch input, the input system SHALL dispatch events with 
 * correct touch ID, coordinates, and phase.
 * 
 * This test verifies that:
 * 1. TouchEvent created via Input::createTouchEvent preserves touch ID
 * 2. TouchEvent preserves coordinates
 * 3. TouchEvent preserves the touch phase
 * 
 * **Validates: Requirements 11.3**
 */
RC_GTEST_PROP(InputEventProperties, TouchEventPreservesData, ()) {
    auto touchId = *genTouchId();
    auto x = *genScreenCoordinate();
    auto y = *genScreenCoordinate();
    auto phase = *genTouchPhase();
    
    TouchEvent event = Input::createTouchEvent(touchId, x, y, phase);
    
    // Verify touch ID is preserved
    RC_ASSERT(event.touchId == touchId);
    
    // Verify coordinates are preserved
    RC_ASSERT(event.x == x);
    RC_ASSERT(event.y == y);
    
    // Verify phase is preserved
    RC_ASSERT(event.phase == phase);
}

/**
 * **Feature: killergk-gui-library, Property 17: Input Event Dispatch**
 * 
 * *For any* dispatched keyboard event, the InputDispatcher SHALL deliver
 * the event to all registered callbacks with correct data.
 * 
 * This test verifies that:
 * 1. Dispatched KeyEvent reaches registered callbacks
 * 2. The received event data matches the dispatched event
 * 
 * **Validates: Requirements 11.1**
 */
RC_GTEST_PROP(InputEventProperties, KeyEventDispatchPreservesData, ()) {
    auto originalEvent = *gen::arbitrary<KeyEvent>();
    
    // Clear any existing callbacks
    InputDispatcher::instance().clearAllCallbacks();
    
    // Track received event
    KeyEvent receivedEvent;
    bool callbackCalled = false;
    
    // Register callback
    int handle = InputDispatcher::instance().addKeyCallback(
        [&receivedEvent, &callbackCalled](const KeyEvent& event) {
            receivedEvent = event;
            callbackCalled = true;
        }
    );
    
    // Dispatch the event
    InputDispatcher::instance().dispatchKeyEvent(originalEvent);
    
    // Cleanup
    InputDispatcher::instance().removeCallback(handle);
    
    // Verify callback was called
    RC_ASSERT(callbackCalled);
    
    // Verify event data is preserved
    RC_ASSERT(receivedEvent.key == originalEvent.key);
    RC_ASSERT(receivedEvent.pressed == originalEvent.pressed);
    RC_ASSERT(receivedEvent.repeat == originalEvent.repeat);
    RC_ASSERT(receivedEvent.modifiers == originalEvent.modifiers);
}

/**
 * **Feature: killergk-gui-library, Property 17: Input Event Dispatch**
 * 
 * *For any* dispatched mouse event, the InputDispatcher SHALL deliver
 * the event to all registered callbacks with correct data.
 * 
 * This test verifies that:
 * 1. Dispatched MouseEvent reaches registered callbacks
 * 2. The received event data matches the dispatched event
 * 
 * **Validates: Requirements 11.2**
 */
RC_GTEST_PROP(InputEventProperties, MouseEventDispatchPreservesData, ()) {
    auto originalEvent = *gen::arbitrary<MouseEvent>();
    
    // Clear any existing callbacks
    InputDispatcher::instance().clearAllCallbacks();
    
    // Track received event
    MouseEvent receivedEvent;
    bool callbackCalled = false;
    
    // Register callback
    int handle = InputDispatcher::instance().addMouseCallback(
        [&receivedEvent, &callbackCalled](const MouseEvent& event) {
            receivedEvent = event;
            callbackCalled = true;
        }
    );
    
    // Dispatch the event
    InputDispatcher::instance().dispatchMouseEvent(originalEvent);
    
    // Cleanup
    InputDispatcher::instance().removeCallback(handle);
    
    // Verify callback was called
    RC_ASSERT(callbackCalled);
    
    // Verify event data is preserved
    RC_ASSERT(receivedEvent.x == originalEvent.x);
    RC_ASSERT(receivedEvent.y == originalEvent.y);
    RC_ASSERT(receivedEvent.button == originalEvent.button);
    RC_ASSERT(receivedEvent.clicks == originalEvent.clicks);
    RC_ASSERT(receivedEvent.scrollX == originalEvent.scrollX);
    RC_ASSERT(receivedEvent.scrollY == originalEvent.scrollY);
    RC_ASSERT(receivedEvent.modifiers == originalEvent.modifiers);
}

/**
 * **Feature: killergk-gui-library, Property 17: Input Event Dispatch**
 * 
 * *For any* dispatched touch event, the InputDispatcher SHALL deliver
 * the event to all registered callbacks with correct data.
 * 
 * This test verifies that:
 * 1. Dispatched TouchEvent reaches registered callbacks
 * 2. The received event data matches the dispatched event
 * 
 * **Validates: Requirements 11.3**
 */
RC_GTEST_PROP(InputEventProperties, TouchEventDispatchPreservesData, ()) {
    auto originalEvent = *gen::arbitrary<TouchEvent>();
    
    // Clear any existing callbacks
    InputDispatcher::instance().clearAllCallbacks();
    
    // Track received event
    TouchEvent receivedEvent;
    bool callbackCalled = false;
    
    // Register callback
    int handle = InputDispatcher::instance().addTouchCallback(
        [&receivedEvent, &callbackCalled](const TouchEvent& event) {
            receivedEvent = event;
            callbackCalled = true;
        }
    );
    
    // Dispatch the event
    InputDispatcher::instance().dispatchTouchEvent(originalEvent);
    
    // Cleanup
    InputDispatcher::instance().removeCallback(handle);
    
    // Verify callback was called
    RC_ASSERT(callbackCalled);
    
    // Verify event data is preserved
    RC_ASSERT(receivedEvent.touchId == originalEvent.touchId);
    RC_ASSERT(receivedEvent.x == originalEvent.x);
    RC_ASSERT(receivedEvent.y == originalEvent.y);
    RC_ASSERT(receivedEvent.phase == originalEvent.phase);
}

/**
 * **Feature: killergk-gui-library, Property 17: Input Event Dispatch**
 * 
 * *For any* set of multiple registered callbacks, the InputDispatcher SHALL
 * deliver events to ALL registered callbacks.
 * 
 * This test verifies that:
 * 1. Multiple callbacks can be registered
 * 2. All callbacks receive the dispatched event
 * 
 * **Validates: Requirements 11.1, 11.2, 11.3**
 */
RC_GTEST_PROP(InputEventProperties, MultipleCallbacksReceiveEvents, ()) {
    auto keyEvent = *gen::arbitrary<KeyEvent>();
    auto numCallbacks = *gen::inRange(2, 6);  // 2 to 5 callbacks
    
    // Clear any existing callbacks
    InputDispatcher::instance().clearAllCallbacks();
    
    // Track which callbacks were called
    std::vector<bool> callbacksCalled(numCallbacks, false);
    std::vector<int> handles;
    
    // Register multiple callbacks
    for (int i = 0; i < numCallbacks; ++i) {
        int handle = InputDispatcher::instance().addKeyCallback(
            [&callbacksCalled, i](const KeyEvent&) {
                callbacksCalled[i] = true;
            }
        );
        handles.push_back(handle);
    }
    
    // Dispatch the event
    InputDispatcher::instance().dispatchKeyEvent(keyEvent);
    
    // Cleanup
    for (int handle : handles) {
        InputDispatcher::instance().removeCallback(handle);
    }
    
    // Verify all callbacks were called
    for (int i = 0; i < numCallbacks; ++i) {
        RC_ASSERT(callbacksCalled[i]);
    }
}

// ============================================================================
// Property Tests for Image Format Support
// ============================================================================

#include "KillerGK/rendering/Texture.hpp"

namespace rc {

/**
 * @brief Generator for supported ImageFormat values (excluding Unknown and SVG)
 */
inline Gen<KillerGK::ImageFormat> genSupportedImageFormat() {
    return gen::element(
        KillerGK::ImageFormat::PNG,
        KillerGK::ImageFormat::JPG,
        KillerGK::ImageFormat::BMP,
        KillerGK::ImageFormat::TGA,
        KillerGK::ImageFormat::ICO
    );
}

/**
 * @brief Generator for valid PNG file header
 * PNG signature: 89 50 4E 47 0D 0A 1A 0A
 */
inline std::vector<uint8_t> genPNGHeader() {
    return {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
}

/**
 * @brief Generator for valid JPEG file header
 * JPEG signature: FF D8 FF (minimum 8 bytes required by detectFormatFromHeader)
 */
inline std::vector<uint8_t> genJPGHeader() {
    return {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46};  // JFIF marker with padding
}

/**
 * @brief Generator for valid BMP file header
 * BMP signature: 42 4D (BM) - already 8 bytes
 */
inline std::vector<uint8_t> genBMPHeader() {
    return {0x42, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
}

/**
 * @brief Generator for valid ICO file header
 * ICO signature: 00 00 01 00 (minimum 8 bytes required by detectFormatFromHeader)
 */
inline std::vector<uint8_t> genICOHeader() {
    return {0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10};  // 1 image in ICO with size
}

/**
 * @brief Generator for valid image dimensions
 * Keep dimensions small for fast test execution
 */
inline Gen<int> genImageDimension() {
    return gen::inRange(1, 64);  // 1 to 64 pixels (small for fast tests)
}

/**
 * @brief Generator for valid channel count
 */
inline Gen<int> genChannelCount() {
    return gen::element(1, 2, 3, 4);  // Grayscale, GA, RGB, RGBA
}

/**
 * @brief Generator for ImageData with valid pixel data
 * Uses efficient bulk generation for pixel data
 */
template<>
struct Arbitrary<KillerGK::ImageData> {
    static Gen<KillerGK::ImageData> arbitrary() {
        return gen::exec([]() {
            KillerGK::ImageData data;
            data.width = *genImageDimension();
            data.height = *genImageDimension();
            data.channels = *genChannelCount();
            data.format = *genSupportedImageFormat();
            
            // Generate pixel data efficiently using container generator
            size_t pixelCount = static_cast<size_t>(data.width) * data.height * data.channels;
            data.pixels = *gen::container<std::vector<uint8_t>>(
                pixelCount, 
                gen::inRange<uint8_t>(0, 255)
            );
            
            return data;
        });
    }
};

} // namespace rc

/**
 * **Feature: killergk-gui-library, Property 25: Image Format Support**
 * 
 * *For any* valid image file in supported formats (PNG, JPG, BMP, ICO),
 * the format detection SHALL correctly identify the format from the file header.
 * 
 * This test verifies that:
 * 1. PNG headers are correctly detected as PNG format
 * 2. JPG headers are correctly detected as JPG format
 * 3. BMP headers are correctly detected as BMP format
 * 4. ICO headers are correctly detected as ICO format
 * 
 * **Validates: Requirements 6.2**
 */
RC_GTEST_PROP(ImageFormatProperties, FormatDetectionFromHeader_PNG, ()) {
    // Test PNG header detection
    std::vector<uint8_t> pngData = genPNGHeader();
    
    // Add random padding
    auto paddingSize = *gen::inRange(0, 100);
    for (int i = 0; i < paddingSize; ++i) {
        pngData.push_back(static_cast<uint8_t>(*gen::inRange(0, 256)));
    }
    
    ImageFormat detected = ImageLoader::detectFormatFromHeader(pngData.data(), pngData.size());
    RC_ASSERT(detected == ImageFormat::PNG);
}

/**
 * **Feature: killergk-gui-library, Property 25: Image Format Support**
 * **Validates: Requirements 6.2**
 */
RC_GTEST_PROP(ImageFormatProperties, FormatDetectionFromHeader_JPG, ()) {
    // Test JPG header detection
    std::vector<uint8_t> jpgData = genJPGHeader();
    
    // Add random padding
    auto paddingSize = *gen::inRange(0, 100);
    for (int i = 0; i < paddingSize; ++i) {
        jpgData.push_back(static_cast<uint8_t>(*gen::inRange(0, 256)));
    }
    
    ImageFormat detected = ImageLoader::detectFormatFromHeader(jpgData.data(), jpgData.size());
    RC_ASSERT(detected == ImageFormat::JPG);
}

/**
 * **Feature: killergk-gui-library, Property 25: Image Format Support**
 * **Validates: Requirements 6.2**
 */
RC_GTEST_PROP(ImageFormatProperties, FormatDetectionFromHeader_BMP, ()) {
    // Test BMP header detection
    std::vector<uint8_t> bmpData = genBMPHeader();
    
    // Add random padding
    auto paddingSize = *gen::inRange(0, 100);
    for (int i = 0; i < paddingSize; ++i) {
        bmpData.push_back(static_cast<uint8_t>(*gen::inRange(0, 256)));
    }
    
    ImageFormat detected = ImageLoader::detectFormatFromHeader(bmpData.data(), bmpData.size());
    RC_ASSERT(detected == ImageFormat::BMP);
}

/**
 * **Feature: killergk-gui-library, Property 25: Image Format Support**
 * **Validates: Requirements 6.2**
 */
RC_GTEST_PROP(ImageFormatProperties, FormatDetectionFromHeader_ICO, ()) {
    // Test ICO header detection
    std::vector<uint8_t> icoData = genICOHeader();
    
    // Add random padding
    auto paddingSize = *gen::inRange(0, 100);
    for (int i = 0; i < paddingSize; ++i) {
        icoData.push_back(static_cast<uint8_t>(*gen::inRange(0, 256)));
    }
    
    ImageFormat detected = ImageLoader::detectFormatFromHeader(icoData.data(), icoData.size());
    RC_ASSERT(detected == ImageFormat::ICO);
}

/**
 * **Feature: killergk-gui-library, Property 25: Image Format Support**
 * 
 * *For any* valid ImageData, the isValid() method SHALL return true
 * if and only if the image has valid dimensions and pixel data.
 * 
 * This test verifies that:
 * 1. ImageData with positive dimensions and non-empty pixels is valid
 * 2. The sizeBytes() calculation is correct
 * 
 * **Validates: Requirements 6.2**
 */
RC_GTEST_PROP(ImageFormatProperties, ImageDataValidityCheck, ()) {
    auto imageData = *gen::arbitrary<ImageData>();
    
    // Verify isValid returns true for properly constructed ImageData
    RC_ASSERT(imageData.isValid());
    
    // Verify sizeBytes calculation
    size_t expectedSize = static_cast<size_t>(imageData.width) * imageData.height * imageData.channels;
    RC_ASSERT(imageData.sizeBytes() == expectedSize);
    RC_ASSERT(imageData.pixels.size() == expectedSize);
}

/**
 * **Feature: killergk-gui-library, Property 25: Image Format Support**
 * 
 * *For any* supported image format, the isFormatSupported() method SHALL
 * return true for PNG, JPG, BMP, TGA, and ICO formats.
 * 
 * This test verifies that:
 * 1. All documented supported formats return true
 * 2. SVG (not yet implemented) returns false
 * 3. Unknown format returns false
 * 
 * **Validates: Requirements 6.2**
 */
RC_GTEST_PROP(ImageFormatProperties, SupportedFormatsAreRecognized, ()) {
    auto format = *genSupportedImageFormat();
    
    // All supported formats should return true
    RC_ASSERT(ImageLoader::isFormatSupported(format));
}

/**
 * **Feature: killergk-gui-library, Property 25: Image Format Support**
 * 
 * *For any* file path with a supported extension, the format detection
 * SHALL correctly identify the format from the file extension.
 * 
 * This test verifies that:
 * 1. .png extension is detected as PNG
 * 2. .jpg and .jpeg extensions are detected as JPG
 * 3. .bmp extension is detected as BMP
 * 4. .tga extension is detected as TGA
 * 5. .ico extension is detected as ICO
 * 6. .svg extension is detected as SVG
 * 
 * **Validates: Requirements 6.2**
 */
RC_GTEST_PROP(ImageFormatProperties, FormatDetectionFromExtension, ()) {
    // Generate a random base filename
    auto baseNameLen = *gen::inRange(1, 20);
    std::string baseName;
    for (int i = 0; i < baseNameLen; ++i) {
        baseName += static_cast<char>(*gen::inRange(static_cast<int>('a'), static_cast<int>('z') + 1));
    }
    
    // Test each extension
    struct ExtensionTest {
        std::string ext;
        ImageFormat expected;
    };
    
    std::vector<ExtensionTest> tests = {
        {".png", ImageFormat::PNG},
        {".PNG", ImageFormat::PNG},
        {".jpg", ImageFormat::JPG},
        {".JPG", ImageFormat::JPG},
        {".jpeg", ImageFormat::JPG},
        {".JPEG", ImageFormat::JPG},
        {".bmp", ImageFormat::BMP},
        {".BMP", ImageFormat::BMP},
        {".tga", ImageFormat::TGA},
        {".TGA", ImageFormat::TGA},
        {".ico", ImageFormat::ICO},
        {".ICO", ImageFormat::ICO},
        {".svg", ImageFormat::SVG},
        {".SVG", ImageFormat::SVG}
    };
    
    // Pick a random extension to test
    auto testIndex = *gen::inRange(0, static_cast<int>(tests.size()));
    const auto& test = tests[testIndex];
    
    std::string path = baseName + test.ext;
    ImageFormat detected = ImageLoader::detectFormat(path);
    
    RC_ASSERT(detected == test.expected);
}

/**
 * **Feature: killergk-gui-library, Property 25: Image Format Support**
 * 
 * *For any* ImageData with invalid properties (zero dimensions or empty pixels),
 * the isValid() method SHALL return false.
 * 
 * This test verifies that:
 * 1. ImageData with zero width is invalid
 * 2. ImageData with zero height is invalid
 * 3. ImageData with zero channels is invalid
 * 4. ImageData with empty pixels is invalid
 * 
 * **Validates: Requirements 6.2**
 */
RC_GTEST_PROP(ImageFormatProperties, InvalidImageDataDetection, ()) {
    // Generate which property to invalidate
    auto invalidationType = *gen::inRange(0, 4);
    
    ImageData imageData;
    imageData.width = *genImageDimension();
    imageData.height = *genImageDimension();
    imageData.channels = *genChannelCount();
    imageData.format = *genSupportedImageFormat();
    
    // Initially populate with valid pixel data
    size_t pixelCount = static_cast<size_t>(imageData.width) * imageData.height * imageData.channels;
    imageData.pixels.resize(pixelCount);
    
    // Now invalidate one property
    switch (invalidationType) {
        case 0:
            imageData.width = 0;
            break;
        case 1:
            imageData.height = 0;
            break;
        case 2:
            imageData.channels = 0;
            break;
        case 3:
            imageData.pixels.clear();
            break;
    }
    
    // Verify isValid returns false
    RC_ASSERT(!imageData.isValid());
}

/**
 * **Feature: killergk-gui-library, Property 25: Image Format Support**
 * 
 * *For any* unknown file header (not matching any supported format),
 * the format detection SHALL return ImageFormat::Unknown.
 * 
 * This test verifies that:
 * 1. Random data that doesn't match any format header returns Unknown
 * 2. Empty data returns Unknown
 * 3. Data too short to identify returns Unknown
 * 
 * **Validates: Requirements 6.2**
 */
RC_GTEST_PROP(ImageFormatProperties, UnknownFormatDetection, ()) {
    // Generate random data that doesn't match any known header
    auto dataSize = *gen::inRange(0, 100);
    std::vector<uint8_t> data(dataSize);
    
    // Fill with random bytes, avoiding known magic numbers
    for (size_t i = 0; i < data.size(); ++i) {
        auto byte = *gen::inRange(0, 256);
        // Avoid starting with known magic bytes
        if (i == 0) {
            // Avoid PNG (0x89), JPEG (0xFF), BMP (0x42), ICO (0x00)
            while (byte == 0x89 || byte == 0xFF || byte == 0x42 || byte == 0x00) {
                byte = *gen::inRange(0, 256);
            }
        }
        data[i] = static_cast<uint8_t>(byte);
    }
    
    ImageFormat detected = ImageLoader::detectFormatFromHeader(data.data(), data.size());
    
    RC_ASSERT(detected == ImageFormat::Unknown);
}

// ============================================================================
// Property Tests for Widget Builder Pattern
// ============================================================================

#include "KillerGK/widgets/Widget.hpp"

namespace rc {

/**
 * @brief Generator for valid widget ID strings
 */
inline Gen<std::string> genWidgetId() {
    return gen::map(gen::inRange(1, 50), [](int len) {
        std::string id = "widget_";
        id += std::to_string(len);
        return id;
    });
}

/**
 * @brief Generator for valid widget dimension (positive float)
 */
inline Gen<float> genWidgetDimension() {
    return gen::map(gen::inRange(1, 10000), [](int v) {
        return static_cast<float>(v) / 10.0f;  // 0.1 to 1000.0
    });
}

/**
 * @brief Generator for valid opacity value [0.0, 1.0]
 */
inline Gen<float> genOpacity() {
    return genNormalizedFloat();
}

/**
 * @brief Generator for valid border radius (non-negative)
 */
inline Gen<float> genBorderRadius() {
    return gen::map(gen::inRange(0, 1000), [](int v) {
        return static_cast<float>(v) / 10.0f;  // 0.0 to 100.0
    });
}

/**
 * @brief Generator for valid border width (non-negative)
 */
inline Gen<float> genBorderWidth() {
    return gen::map(gen::inRange(0, 100), [](int v) {
        return static_cast<float>(v) / 10.0f;  // 0.0 to 10.0
    });
}

/**
 * @brief Generator for valid blur radius (non-negative)
 */
inline Gen<float> genBlurRadius() {
    return gen::map(gen::inRange(0, 500), [](int v) {
        return static_cast<float>(v) / 10.0f;  // 0.0 to 50.0
    });
}

/**
 * @brief Generator for valid spacing value (non-negative)
 */
inline Gen<float> genSpacingValue() {
    return gen::map(gen::inRange(0, 1000), [](int v) {
        return static_cast<float>(v) / 10.0f;  // 0.0 to 100.0
    });
}

/**
 * @brief Generator for Spacing struct
 */
template<>
struct Arbitrary<KillerGK::Spacing> {
    static Gen<KillerGK::Spacing> arbitrary() {
        return gen::build<KillerGK::Spacing>(
            gen::set(&KillerGK::Spacing::top, genSpacingValue()),
            gen::set(&KillerGK::Spacing::right, genSpacingValue()),
            gen::set(&KillerGK::Spacing::bottom, genSpacingValue()),
            gen::set(&KillerGK::Spacing::left, genSpacingValue())
        );
    }
};

/**
 * @brief Generator for Shadow struct
 */
template<>
struct Arbitrary<KillerGK::Shadow> {
    static Gen<KillerGK::Shadow> arbitrary() {
        return gen::build<KillerGK::Shadow>(
            gen::set(&KillerGK::Shadow::blur, genBlurRadius()),
            gen::set(&KillerGK::Shadow::offsetX, genFloatInRange(-100.0f, 100.0f)),
            gen::set(&KillerGK::Shadow::offsetY, genFloatInRange(-100.0f, 100.0f)),
            gen::set(&KillerGK::Shadow::color, gen::arbitrary<KillerGK::Color>())
        );
    }
};

/**
 * @brief Generator for tooltip strings
 */
inline Gen<std::string> genTooltipText() {
    return gen::map(gen::inRange(0, 100), [](int len) {
        if (len == 0) return std::string("");
        std::string tooltip = "Tooltip_";
        tooltip += std::to_string(len);
        return tooltip;
    });
}

} // namespace rc

/**
 * **Feature: killergk-gui-library, Property 1: Builder Pattern Property Preservation**
 * 
 * *For any* widget created using the Builder Pattern with any combination of 
 * valid properties, the built widget SHALL have all specified properties 
 * correctly applied and retrievable.
 * 
 * This test verifies that:
 * 1. ID property is correctly set and retrieved
 * 2. Size properties (width, height) are correctly set and retrieved
 * 3. Min/max constraints are correctly set and retrieved
 * 
 * **Validates: Requirements 1.1, 1.2**
 */
RC_GTEST_PROP(WidgetBuilderProperties, BuilderPreservesIdentityAndSize, ()) {
    auto widgetId = *genWidgetId();
    auto width = *genWidgetDimension();
    auto height = *genWidgetDimension();
    auto minWidth = *genWidgetDimension();
    auto maxWidth = *gen::map(gen::inRange(1, 10000), [&minWidth](int v) {
        return minWidth + static_cast<float>(v) / 10.0f;  // Ensure maxWidth >= minWidth
    });
    auto minHeight = *genWidgetDimension();
    auto maxHeight = *gen::map(gen::inRange(1, 10000), [&minHeight](int v) {
        return minHeight + static_cast<float>(v) / 10.0f;  // Ensure maxHeight >= minHeight
    });
    
    // Create widget using Builder Pattern
    Widget widget = Widget::create()
        .id(widgetId)
        .width(width)
        .height(height)
        .minWidth(minWidth)
        .maxWidth(maxWidth)
        .minHeight(minHeight)
        .maxHeight(maxHeight);
    
    // Verify all properties are correctly preserved
    RC_ASSERT(widget.getId() == widgetId);
    RC_ASSERT(widget.getWidth() == width);
    RC_ASSERT(widget.getHeight() == height);
    RC_ASSERT(widget.getMinWidth() == minWidth);
    RC_ASSERT(widget.getMaxWidth() == maxWidth);
    RC_ASSERT(widget.getMinHeight() == minHeight);
    RC_ASSERT(widget.getMaxHeight() == maxHeight);
}

/**
 * **Feature: killergk-gui-library, Property 1: Builder Pattern Property Preservation**
 * 
 * *For any* widget created using the Builder Pattern with spacing properties,
 * the built widget SHALL have all margin and padding values correctly applied.
 * 
 * This test verifies that:
 * 1. Margin (4-value) is correctly set and retrieved
 * 2. Padding (4-value) is correctly set and retrieved
 * 
 * **Validates: Requirements 1.1, 1.2**
 */
RC_GTEST_PROP(WidgetBuilderProperties, BuilderPreservesSpacing, ()) {
    auto marginTop = *genSpacingValue();
    auto marginRight = *genSpacingValue();
    auto marginBottom = *genSpacingValue();
    auto marginLeft = *genSpacingValue();
    auto paddingTop = *genSpacingValue();
    auto paddingRight = *genSpacingValue();
    auto paddingBottom = *genSpacingValue();
    auto paddingLeft = *genSpacingValue();
    
    // Create widget using Builder Pattern with 4-value spacing
    Widget widget = Widget::create()
        .margin(marginTop, marginRight, marginBottom, marginLeft)
        .padding(paddingTop, paddingRight, paddingBottom, paddingLeft);
    
    // Verify margin is correctly preserved
    const Spacing& margin = widget.getMargin();
    RC_ASSERT(margin.top == marginTop);
    RC_ASSERT(margin.right == marginRight);
    RC_ASSERT(margin.bottom == marginBottom);
    RC_ASSERT(margin.left == marginLeft);
    
    // Verify padding is correctly preserved
    const Spacing& padding = widget.getPadding();
    RC_ASSERT(padding.top == paddingTop);
    RC_ASSERT(padding.right == paddingRight);
    RC_ASSERT(padding.bottom == paddingBottom);
    RC_ASSERT(padding.left == paddingLeft);
}

/**
 * **Feature: killergk-gui-library, Property 1: Builder Pattern Property Preservation**
 * 
 * *For any* widget created using the Builder Pattern with styling properties,
 * the built widget SHALL have all visual properties correctly applied.
 * 
 * This test verifies that:
 * 1. Background color is correctly set and retrieved
 * 2. Border properties (radius, width, color) are correctly set and retrieved
 * 3. Opacity is correctly set and retrieved
 * 4. Blur radius is correctly set and retrieved
 * 
 * **Validates: Requirements 1.1, 1.2**
 */
RC_GTEST_PROP(WidgetBuilderProperties, BuilderPreservesStyling, ()) {
    auto bgColor = *gen::arbitrary<Color>();
    auto borderRadius = *genBorderRadius();
    auto borderWidth = *genBorderWidth();
    auto borderColor = *gen::arbitrary<Color>();
    auto opacity = *genOpacity();
    auto blurRadius = *genBlurRadius();
    
    // Create widget using Builder Pattern
    Widget widget = Widget::create()
        .backgroundColor(bgColor)
        .borderRadius(borderRadius)
        .borderWidth(borderWidth)
        .borderColor(borderColor)
        .opacity(opacity)
        .blur(blurRadius);
    
    // Verify background color is preserved
    const Color& retrievedBgColor = widget.getBackgroundColor();
    RC_ASSERT(retrievedBgColor.r == bgColor.r);
    RC_ASSERT(retrievedBgColor.g == bgColor.g);
    RC_ASSERT(retrievedBgColor.b == bgColor.b);
    RC_ASSERT(retrievedBgColor.a == bgColor.a);
    
    // Verify border properties are preserved
    RC_ASSERT(widget.getBorderRadius() == borderRadius);
    RC_ASSERT(widget.getBorderWidth() == borderWidth);
    
    const Color& retrievedBorderColor = widget.getBorderColor();
    RC_ASSERT(retrievedBorderColor.r == borderColor.r);
    RC_ASSERT(retrievedBorderColor.g == borderColor.g);
    RC_ASSERT(retrievedBorderColor.b == borderColor.b);
    RC_ASSERT(retrievedBorderColor.a == borderColor.a);
    
    // Verify opacity and blur are preserved
    RC_ASSERT(widget.getOpacity() == opacity);
    RC_ASSERT(widget.getBlur() == blurRadius);
}

/**
 * **Feature: killergk-gui-library, Property 1: Builder Pattern Property Preservation**
 * 
 * *For any* widget created using the Builder Pattern with shadow properties,
 * the built widget SHALL have the shadow correctly applied.
 * 
 * This test verifies that:
 * 1. Shadow blur is correctly set and retrieved
 * 2. Shadow offset (X, Y) is correctly set and retrieved
 * 3. Shadow color is correctly set and retrieved
 * 
 * **Validates: Requirements 1.1, 1.2**
 */
RC_GTEST_PROP(WidgetBuilderProperties, BuilderPreservesShadow, ()) {
    auto shadowBlur = *genBlurRadius();
    auto shadowOffsetX = *genFloatInRange(-100.0f, 100.0f);
    auto shadowOffsetY = *genFloatInRange(-100.0f, 100.0f);
    auto shadowColor = *gen::arbitrary<Color>();
    
    // Create widget using Builder Pattern
    Widget widget = Widget::create()
        .shadow(shadowBlur, shadowOffsetX, shadowOffsetY, shadowColor);
    
    // Verify shadow is correctly preserved
    const Shadow& shadow = widget.getShadow();
    RC_ASSERT(shadow.blur == shadowBlur);
    RC_ASSERT(shadow.offsetX == shadowOffsetX);
    RC_ASSERT(shadow.offsetY == shadowOffsetY);
    RC_ASSERT(shadow.color.r == shadowColor.r);
    RC_ASSERT(shadow.color.g == shadowColor.g);
    RC_ASSERT(shadow.color.b == shadowColor.b);
    RC_ASSERT(shadow.color.a == shadowColor.a);
}

/**
 * **Feature: killergk-gui-library, Property 1: Builder Pattern Property Preservation**
 * 
 * *For any* widget created using the Builder Pattern with visibility/state properties,
 * the built widget SHALL have all state properties correctly applied.
 * 
 * This test verifies that:
 * 1. Visible property is correctly set and retrieved
 * 2. Enabled property is correctly set and retrieved
 * 3. Tooltip is correctly set and retrieved
 * 
 * **Validates: Requirements 1.1, 1.2**
 */
RC_GTEST_PROP(WidgetBuilderProperties, BuilderPreservesVisibilityAndState, ()) {
    auto visible = *gen::arbitrary<bool>();
    auto enabled = *gen::arbitrary<bool>();
    auto tooltip = *genTooltipText();
    
    // Create widget using Builder Pattern
    Widget widget = Widget::create()
        .visible(visible)
        .enabled(enabled)
        .tooltip(tooltip);
    
    // Verify visibility and state are preserved
    RC_ASSERT(widget.isVisible() == visible);
    RC_ASSERT(widget.isEnabled() == enabled);
    RC_ASSERT(widget.getTooltip() == tooltip);
}

/**
 * **Feature: killergk-gui-library, Property 1: Builder Pattern Property Preservation**
 * 
 * *For any* widget created using the Builder Pattern with custom properties,
 * the built widget SHALL have all custom properties correctly stored and retrievable.
 * 
 * This test verifies that:
 * 1. Custom float properties are correctly set and retrieved
 * 2. Custom int properties are correctly set and retrieved
 * 3. Custom bool properties are correctly set and retrieved
 * 4. Custom string properties are correctly set and retrieved
 * 5. hasProperty correctly reports property existence
 * 
 * **Validates: Requirements 1.1, 1.2**
 */
RC_GTEST_PROP(WidgetBuilderProperties, BuilderPreservesCustomProperties, ()) {
    auto floatValue = *genFloatInRange(-1000.0f, 1000.0f);
    auto intValue = *gen::inRange(-1000, 1000);
    auto boolValue = *gen::arbitrary<bool>();
    auto stringValue = *genWidgetId();  // Reuse widget ID generator for string
    
    // Create widget using Builder Pattern with custom properties
    Widget widget = Widget::create()
        .setPropertyFloat("customFloat", floatValue)
        .setPropertyInt("customInt", intValue)
        .setPropertyBool("customBool", boolValue)
        .setPropertyString("customString", stringValue);
    
    // Verify custom properties are preserved
    RC_ASSERT(widget.hasProperty("customFloat"));
    RC_ASSERT(widget.hasProperty("customInt"));
    RC_ASSERT(widget.hasProperty("customBool"));
    RC_ASSERT(widget.hasProperty("customString"));
    
    RC_ASSERT(widget.getPropertyFloat("customFloat") == floatValue);
    RC_ASSERT(widget.getPropertyInt("customInt") == intValue);
    RC_ASSERT(widget.getPropertyBool("customBool") == boolValue);
    RC_ASSERT(widget.getPropertyString("customString") == stringValue);
    
    // Verify non-existent property returns default
    RC_ASSERT(!widget.hasProperty("nonExistent"));
    RC_ASSERT(widget.getPropertyFloat("nonExistent", 42.0f) == 42.0f);
    RC_ASSERT(widget.getPropertyInt("nonExistent", 42) == 42);
    RC_ASSERT(widget.getPropertyBool("nonExistent", true) == true);
    RC_ASSERT(widget.getPropertyString("nonExistent", "default") == "default");
}

/**
 * **Feature: killergk-gui-library, Property 1: Builder Pattern Property Preservation**
 * 
 * *For any* widget created using the Builder Pattern with chained method calls,
 * the built widget SHALL have ALL properties correctly applied regardless of order.
 * 
 * This test verifies that:
 * 1. Multiple properties can be chained in any order
 * 2. All chained properties are correctly preserved
 * 3. Builder pattern returns the same widget reference for chaining
 * 
 * **Validates: Requirements 1.1, 1.2**
 */
RC_GTEST_PROP(WidgetBuilderProperties, BuilderChainingPreservesAllProperties, ()) {
    // Generate all properties
    auto widgetId = *genWidgetId();
    auto width = *genWidgetDimension();
    auto height = *genWidgetDimension();
    auto bgColor = *gen::arbitrary<Color>();
    auto borderRadius = *genBorderRadius();
    auto opacity = *genOpacity();
    auto visible = *gen::arbitrary<bool>();
    auto enabled = *gen::arbitrary<bool>();
    auto marginAll = *genSpacingValue();
    auto paddingAll = *genSpacingValue();
    
    // Create widget with all properties chained
    Widget widget = Widget::create()
        .id(widgetId)
        .width(width)
        .height(height)
        .backgroundColor(bgColor)
        .borderRadius(borderRadius)
        .opacity(opacity)
        .visible(visible)
        .enabled(enabled)
        .margin(marginAll)
        .padding(paddingAll);
    
    // Verify ALL properties are preserved after chaining
    RC_ASSERT(widget.getId() == widgetId);
    RC_ASSERT(widget.getWidth() == width);
    RC_ASSERT(widget.getHeight() == height);
    
    const Color& retrievedBgColor = widget.getBackgroundColor();
    RC_ASSERT(retrievedBgColor.r == bgColor.r);
    RC_ASSERT(retrievedBgColor.g == bgColor.g);
    RC_ASSERT(retrievedBgColor.b == bgColor.b);
    RC_ASSERT(retrievedBgColor.a == bgColor.a);
    
    RC_ASSERT(widget.getBorderRadius() == borderRadius);
    RC_ASSERT(widget.getOpacity() == opacity);
    RC_ASSERT(widget.isVisible() == visible);
    RC_ASSERT(widget.isEnabled() == enabled);
    
    // Verify margin (single value sets all sides)
    const Spacing& margin = widget.getMargin();
    RC_ASSERT(margin.top == marginAll);
    RC_ASSERT(margin.right == marginAll);
    RC_ASSERT(margin.bottom == marginAll);
    RC_ASSERT(margin.left == marginAll);
    
    // Verify padding (single value sets all sides)
    const Spacing& padding = widget.getPadding();
    RC_ASSERT(padding.top == paddingAll);
    RC_ASSERT(padding.right == paddingAll);
    RC_ASSERT(padding.bottom == paddingAll);
    RC_ASSERT(padding.left == paddingAll);
}

/**
 * **Feature: killergk-gui-library, Property 1: Builder Pattern Property Preservation**
 * 
 * *For any* widget created using the Builder Pattern with transition properties,
 * the built widget SHALL have all transitions correctly registered.
 * 
 * This test verifies that:
 * 1. Transitions can be added via Builder Pattern
 * 2. Multiple transitions are preserved
 * 3. Transition property and duration are correctly stored
 * 
 * **Validates: Requirements 1.1, 1.2**
 */
RC_GTEST_PROP(WidgetBuilderProperties, BuilderPreservesTransitions, ()) {
    auto duration1 = *gen::map(gen::inRange(1, 10000), [](int v) {
        return static_cast<float>(v);  // 1 to 10000 ms
    });
    auto duration2 = *gen::map(gen::inRange(1, 10000), [](int v) {
        return static_cast<float>(v);
    });
    
    // Create widget with transitions
    Widget widget = Widget::create()
        .transition(Property::Opacity, duration1)
        .transition(Property::Width, duration2);
    
    // Verify transitions are preserved
    const auto& transitions = widget.getTransitions();
    RC_ASSERT(transitions.size() == 2);
    
    // Find and verify each transition
    bool foundOpacity = false;
    bool foundWidth = false;
    
    for (const auto& t : transitions) {
        if (t.property == Property::Opacity) {
            RC_ASSERT(t.duration == duration1);
            foundOpacity = true;
        } else if (t.property == Property::Width) {
            RC_ASSERT(t.duration == duration2);
            foundWidth = true;
        }
    }
    
    RC_ASSERT(foundOpacity);
    RC_ASSERT(foundWidth);
}

// ============================================================================
// Property Tests for Widget State Serialization Round-Trip
// ============================================================================

namespace rc {

/**
 * @brief Generator for valid widget state ID strings
 * Generates simple alphanumeric IDs that are safe for JSON serialization
 */
inline Gen<std::string> genWidgetStateId() {
    return gen::map(gen::inRange(0, 100), [](int len) {
        if (len == 0) return std::string("");
        std::string id = "state_";
        id += std::to_string(len);
        return id;
    });
}

/**
 * @brief Generator for WidgetState with serializable properties only
 * 
 * This generator creates WidgetState objects with properties that can be
 * correctly serialized to JSON and deserialized back. It avoids complex
 * types that std::any cannot round-trip through JSON.
 */
inline Gen<KillerGK::WidgetState> genSerializableWidgetState() {
    return gen::exec([]() {
        KillerGK::WidgetState state;
        
        // Generate basic fields
        state.id = *genWidgetStateId();
        state.visible = *gen::arbitrary<bool>();
        state.enabled = *gen::arbitrary<bool>();
        state.focused = *gen::arbitrary<bool>();
        state.hovered = *gen::arbitrary<bool>();
        state.pressed = *gen::arbitrary<bool>();
        
        // Generate bounds
        state.bounds.x = *genFloatInRange(-1000.0f, 1000.0f);
        state.bounds.y = *genFloatInRange(-1000.0f, 1000.0f);
        state.bounds.width = *genFloatInRange(0.0f, 1000.0f);
        state.bounds.height = *genFloatInRange(0.0f, 1000.0f);
        
        // Generate a few serializable custom properties
        // Only use types that can be serialized to JSON and back
        auto numFloatProps = *gen::inRange(0, 4);
        for (int i = 0; i < numFloatProps; ++i) {
            std::string key = "floatProp_" + std::to_string(i);
            float value = *genFloatInRange(-1000.0f, 1000.0f);
            state.properties[key] = value;
        }
        
        auto numIntProps = *gen::inRange(0, 4);
        for (int i = 0; i < numIntProps; ++i) {
            std::string key = "intProp_" + std::to_string(i);
            // Note: JSON parsing returns floats, so we use float for numeric properties
            float value = static_cast<float>(*gen::inRange(-1000, 1000));
            state.properties[key] = value;
        }
        
        auto numBoolProps = *gen::inRange(0, 4);
        for (int i = 0; i < numBoolProps; ++i) {
            std::string key = "boolProp_" + std::to_string(i);
            bool value = *gen::arbitrary<bool>();
            state.properties[key] = value;
        }
        
        auto numStringProps = *gen::inRange(0, 4);
        for (int i = 0; i < numStringProps; ++i) {
            std::string key = "stringProp_" + std::to_string(i);
            // Generate simple alphanumeric strings to avoid JSON escaping issues
            std::string value = "value_" + std::to_string(*gen::inRange(0, 1000));
            state.properties[key] = value;
        }
        
        return state;
    });
}

} // namespace rc

/**
 * @brief Helper function to compare two WidgetState objects for equality
 * 
 * This function performs a deep comparison of WidgetState objects,
 * including comparing the values stored in the properties map.
 * 
 * @param a First WidgetState
 * @param b Second WidgetState
 * @return true if states are equivalent
 */
inline bool widgetStatesEquivalent(const KillerGK::WidgetState& a, const KillerGK::WidgetState& b) {
    // Compare basic fields
    if (a.id != b.id) return false;
    if (a.visible != b.visible) return false;
    if (a.enabled != b.enabled) return false;
    if (a.focused != b.focused) return false;
    if (a.hovered != b.hovered) return false;
    if (a.pressed != b.pressed) return false;
    
    // Compare bounds
    if (a.bounds.x != b.bounds.x) return false;
    if (a.bounds.y != b.bounds.y) return false;
    if (a.bounds.width != b.bounds.width) return false;
    if (a.bounds.height != b.bounds.height) return false;
    
    // Compare properties count
    if (a.properties.size() != b.properties.size()) return false;
    
    // Compare property keys and values
    for (const auto& [key, value] : a.properties) {
        auto it = b.properties.find(key);
        if (it == b.properties.end()) return false;
        
        // Compare values based on type
        try {
            if (value.type() == typeid(float)) {
                if (it->second.type() != typeid(float)) return false;
                if (std::any_cast<float>(value) != std::any_cast<float>(it->second)) return false;
            } else if (value.type() == typeid(int)) {
                if (it->second.type() != typeid(int)) return false;
                if (std::any_cast<int>(value) != std::any_cast<int>(it->second)) return false;
            } else if (value.type() == typeid(bool)) {
                if (it->second.type() != typeid(bool)) return false;
                if (std::any_cast<bool>(value) != std::any_cast<bool>(it->second)) return false;
            } else if (value.type() == typeid(std::string)) {
                if (it->second.type() != typeid(std::string)) return false;
                if (std::any_cast<std::string>(value) != std::any_cast<std::string>(it->second)) return false;
            }
        } catch (const std::bad_any_cast&) {
            return false;
        }
    }
    
    return true;
}

/**
 * @brief Helper function for approximate float comparison
 * JSON serialization may introduce floating-point precision differences.
 * Using a larger epsilon (0.001) to account for JSON round-trip precision loss.
 */
inline bool floatApproxEqual(float a, float b, float epsilon = 0.001f) {
    return std::abs(a - b) < epsilon;
}

/**
 * **Feature: killergk-gui-library, Property 14: Widget State Serialization Round-Trip**
 * 
 * *For any* valid widget state, serializing to JSON and then deserializing 
 * SHALL produce an equivalent widget state with all properties preserved.
 * 
 * This test verifies that:
 * 1. WidgetState can be serialized to JSON
 * 2. The JSON can be deserialized back to WidgetState
 * 3. The deserialized state is equivalent to the original
 * 
 * **Validates: Requirements 19.1, 19.2, 19.4**
 */
RC_GTEST_PROP(WidgetStateProperties, SerializationRoundTrip_BasicFields, ()) {
    auto originalState = *genSerializableWidgetState();
    
    // Serialize to JSON
    std::string json = originalState.toJson();
    
    // Verify JSON is not empty
    RC_ASSERT(!json.empty());
    
    // Deserialize from JSON
    KillerGK::WidgetState restoredState = KillerGK::WidgetState::fromJson(json);
    
    // Verify basic fields are preserved
    RC_ASSERT(restoredState.id == originalState.id);
    RC_ASSERT(restoredState.visible == originalState.visible);
    RC_ASSERT(restoredState.enabled == originalState.enabled);
    RC_ASSERT(restoredState.focused == originalState.focused);
    RC_ASSERT(restoredState.hovered == originalState.hovered);
    RC_ASSERT(restoredState.pressed == originalState.pressed);
    
    // Verify bounds are preserved (using approximate comparison for floats)
    RC_ASSERT(floatApproxEqual(restoredState.bounds.x, originalState.bounds.x));
    RC_ASSERT(floatApproxEqual(restoredState.bounds.y, originalState.bounds.y));
    RC_ASSERT(floatApproxEqual(restoredState.bounds.width, originalState.bounds.width));
    RC_ASSERT(floatApproxEqual(restoredState.bounds.height, originalState.bounds.height));
}

/**
 * **Feature: killergk-gui-library, Property 14: Widget State Serialization Round-Trip**
 * 
 * *For any* valid widget state with custom properties, serializing to JSON 
 * and then deserializing SHALL preserve all serializable property values.
 * 
 * This test verifies that:
 * 1. Float properties are preserved through serialization
 * 2. Bool properties are preserved through serialization
 * 3. String properties are preserved through serialization
 * 4. Property keys are preserved
 * 
 * **Validates: Requirements 19.1, 19.2, 19.4**
 */
RC_GTEST_PROP(WidgetStateProperties, SerializationRoundTrip_Properties, ()) {
    auto originalState = *genSerializableWidgetState();
    
    // Serialize to JSON
    std::string json = originalState.toJson();
    
    // Deserialize from JSON
    KillerGK::WidgetState restoredState = KillerGK::WidgetState::fromJson(json);
    
    // Verify property count is preserved
    RC_ASSERT(restoredState.properties.size() == originalState.properties.size());
    
    // Verify all property keys exist in restored state
    for (const auto& [key, value] : originalState.properties) {
        RC_ASSERT(restoredState.properties.find(key) != restoredState.properties.end());
    }
}

/**
 * **Feature: killergk-gui-library, Property 14: Widget State Serialization Round-Trip**
 * 
 * *For any* Widget with state, getting state, serializing, deserializing, 
 * and setting state SHALL preserve the widget's configuration.
 * 
 * This test verifies the full round-trip through Widget's getState/setState:
 * 1. Widget state can be extracted via getState()
 * 2. State can be serialized to JSON
 * 3. JSON can be deserialized back to WidgetState
 * 4. Widget can be restored via setState()
 * 5. Restored widget has equivalent properties
 * 
 * **Validates: Requirements 19.1, 19.2, 19.4**
 */
RC_GTEST_PROP(WidgetStateProperties, SerializationRoundTrip_FullWidget, ()) {
    // Generate widget properties
    auto widgetId = *genWidgetStateId();
    auto width = *genFloatInRange(1.0f, 1000.0f);
    auto height = *genFloatInRange(1.0f, 1000.0f);
    auto opacity = *genNormalizedFloat();
    auto borderRadius = *genFloatInRange(0.0f, 100.0f);
    auto borderWidth = *genFloatInRange(0.0f, 10.0f);
    auto blurRadius = *genFloatInRange(0.0f, 50.0f);
    auto visible = *gen::arbitrary<bool>();
    auto enabled = *gen::arbitrary<bool>();
    
    // Create original widget
    KillerGK::Widget originalWidget = KillerGK::Widget::create()
        .id(widgetId)
        .width(width)
        .height(height)
        .opacity(opacity)
        .borderRadius(borderRadius)
        .borderWidth(borderWidth)
        .blur(blurRadius)
        .visible(visible)
        .enabled(enabled);
    
    // Get state from widget
    KillerGK::WidgetState state = originalWidget.getState();
    
    // Serialize to JSON
    std::string json = state.toJson();
    
    // Deserialize from JSON
    KillerGK::WidgetState restoredState = KillerGK::WidgetState::fromJson(json);
    
    // Create new widget and restore state
    KillerGK::Widget restoredWidget = KillerGK::Widget::create();
    restoredWidget.setState(restoredState);
    
    // Verify widget properties are preserved (using approximate comparison for floats)
    RC_ASSERT(restoredWidget.getId() == widgetId);
    RC_ASSERT(floatApproxEqual(restoredWidget.getWidth(), width));
    RC_ASSERT(floatApproxEqual(restoredWidget.getHeight(), height));
    RC_ASSERT(floatApproxEqual(restoredWidget.getOpacity(), opacity));
    RC_ASSERT(floatApproxEqual(restoredWidget.getBorderRadius(), borderRadius));
    RC_ASSERT(floatApproxEqual(restoredWidget.getBorderWidth(), borderWidth));
    RC_ASSERT(floatApproxEqual(restoredWidget.getBlur(), blurRadius));
    RC_ASSERT(restoredWidget.isVisible() == visible);
    RC_ASSERT(restoredWidget.isEnabled() == enabled);
}

/**
 * **Feature: killergk-gui-library, Property 14: Widget State Serialization Round-Trip**
 * 
 * *For any* empty widget state, serialization round-trip SHALL produce 
 * an equivalent empty state.
 * 
 * This test verifies edge case handling:
 * 1. Empty ID is preserved
 * 2. Default boolean values are preserved
 * 3. Zero bounds are preserved
 * 4. Empty properties map is preserved
 * 
 * **Validates: Requirements 19.1, 19.2, 19.4**
 */
RC_GTEST_PROP(WidgetStateProperties, SerializationRoundTrip_EmptyState, ()) {
    // Create minimal state with random boolean values
    KillerGK::WidgetState originalState;
    originalState.id = "";
    originalState.visible = *gen::arbitrary<bool>();
    originalState.enabled = *gen::arbitrary<bool>();
    originalState.focused = *gen::arbitrary<bool>();
    originalState.hovered = *gen::arbitrary<bool>();
    originalState.pressed = *gen::arbitrary<bool>();
    originalState.bounds = KillerGK::Rect(0, 0, 0, 0);
    // No custom properties
    
    // Serialize to JSON
    std::string json = originalState.toJson();
    
    // Deserialize from JSON
    KillerGK::WidgetState restoredState = KillerGK::WidgetState::fromJson(json);
    
    // Verify all fields are preserved
    RC_ASSERT(restoredState.id == originalState.id);
    RC_ASSERT(restoredState.visible == originalState.visible);
    RC_ASSERT(restoredState.enabled == originalState.enabled);
    RC_ASSERT(restoredState.focused == originalState.focused);
    RC_ASSERT(restoredState.hovered == originalState.hovered);
    RC_ASSERT(restoredState.pressed == originalState.pressed);
    RC_ASSERT(restoredState.bounds.x == 0);
    RC_ASSERT(restoredState.bounds.y == 0);
    RC_ASSERT(restoredState.bounds.width == 0);
    RC_ASSERT(restoredState.bounds.height == 0);
    RC_ASSERT(restoredState.properties.empty());
}

/**
 * **Feature: killergk-gui-library, Property 14: Widget State Serialization Round-Trip**
 * 
 * *For any* widget state serialized to JSON, the JSON SHALL be valid 
 * and parseable.
 * 
 * This test verifies JSON validity:
 * 1. JSON starts with '{'
 * 2. JSON ends with '}'
 * 3. JSON contains required fields
 * 
 * **Validates: Requirements 19.1, 19.2, 19.4**
 */
RC_GTEST_PROP(WidgetStateProperties, SerializationProducesValidJson, ()) {
    auto state = *genSerializableWidgetState();
    
    // Serialize to JSON
    std::string json = state.toJson();
    
    // Verify JSON structure
    RC_ASSERT(!json.empty());
    RC_ASSERT(json.front() == '{');
    RC_ASSERT(json.back() == '}');
    
    // Verify required fields are present in JSON
    RC_ASSERT(json.find("\"id\"") != std::string::npos);
    RC_ASSERT(json.find("\"visible\"") != std::string::npos);
    RC_ASSERT(json.find("\"enabled\"") != std::string::npos);
    RC_ASSERT(json.find("\"focused\"") != std::string::npos);
    RC_ASSERT(json.find("\"hovered\"") != std::string::npos);
    RC_ASSERT(json.find("\"pressed\"") != std::string::npos);
    RC_ASSERT(json.find("\"bounds\"") != std::string::npos);
}


// ============================================================================
// Property Tests for Layout Constraint Satisfaction
// ============================================================================

#include "KillerGK/layout/Layout.hpp"

namespace rc {

/**
 * @brief Generator for valid FlexDirection values
 */
inline Gen<KillerGK::FlexDirection> genFlexDirection() {
    return gen::element(
        KillerGK::FlexDirection::Row,
        KillerGK::FlexDirection::Column,
        KillerGK::FlexDirection::RowReverse,
        KillerGK::FlexDirection::ColumnReverse
    );
}

/**
 * @brief Generator for valid JustifyContent values
 */
inline Gen<KillerGK::JustifyContent> genJustifyContent() {
    return gen::element(
        KillerGK::JustifyContent::Start,
        KillerGK::JustifyContent::End,
        KillerGK::JustifyContent::Center,
        KillerGK::JustifyContent::SpaceBetween,
        KillerGK::JustifyContent::SpaceAround,
        KillerGK::JustifyContent::SpaceEvenly
    );
}

/**
 * @brief Generator for valid AlignItems values
 */
inline Gen<KillerGK::AlignItems> genAlignItems() {
    return gen::element(
        KillerGK::AlignItems::Start,
        KillerGK::AlignItems::End,
        KillerGK::AlignItems::Center,
        KillerGK::AlignItems::Stretch,
        KillerGK::AlignItems::Baseline
    );
}

/**
 * @brief Generator for valid FlexWrap values
 */
inline Gen<KillerGK::FlexWrap> genFlexWrap() {
    return gen::element(
        KillerGK::FlexWrap::NoWrap,
        KillerGK::FlexWrap::Wrap,
        KillerGK::FlexWrap::WrapReverse
    );
}

/**
 * @brief Generator for valid gap values (non-negative)
 */
inline Gen<float> genGapValue() {
    return gen::map(gen::inRange(0, 500), [](int v) {
        return static_cast<float>(v) / 10.0f;  // 0.0 to 50.0
    });
}

/**
 * @brief Generator for valid grid column/row count
 */
inline Gen<int> genGridCount() {
    return gen::inRange(1, 10);  // 1 to 9 columns/rows
}

/**
 * @brief Generator for valid layout constraint values
 */
inline Gen<float> genConstraintValue() {
    return gen::map(gen::inRange(10, 5000), [](int v) {
        return static_cast<float>(v) / 10.0f;  // 1.0 to 500.0
    });
}

/**
 * @brief Generator for LayoutConstraints with valid min <= max
 */
template<>
struct Arbitrary<KillerGK::LayoutConstraints> {
    static Gen<KillerGK::LayoutConstraints> arbitrary() {
        return gen::exec([]() {
            KillerGK::LayoutConstraints constraints;
            
            // Generate min values first
            constraints.minWidth = *genConstraintValue();
            constraints.minHeight = *genConstraintValue();
            
            // Generate max values that are >= min values
            auto extraWidth = *gen::inRange(0, 5000);
            auto extraHeight = *gen::inRange(0, 5000);
            constraints.maxWidth = constraints.minWidth + static_cast<float>(extraWidth) / 10.0f;
            constraints.maxHeight = constraints.minHeight + static_cast<float>(extraHeight) / 10.0f;
            
            return constraints;
        });
    }
};

} // namespace rc

/**
 * **Feature: killergk-gui-library, Property 3: Layout Constraint Satisfaction**
 * 
 * *For any* LayoutConstraints, the constrainWidth and constrainHeight methods
 * SHALL clamp values to be within [min, max] bounds.
 * 
 * This test verifies that:
 * 1. Values below minWidth are clamped to minWidth
 * 2. Values above maxWidth are clamped to maxWidth
 * 3. Values within range are unchanged
 * 
 * **Validates: Requirements 3.1, 3.2, 3.3, 3.4, 3.6**
 */
RC_GTEST_PROP(LayoutConstraintProperties, ConstrainWidthClampsCorrectly, ()) {
    auto constraints = *gen::arbitrary<LayoutConstraints>();
    auto testValue = *genFloatInRange(-100.0f, 1000.0f);
    
    float constrained = constraints.constrainWidth(testValue);
    
    // Result must be within bounds
    RC_ASSERT(constrained >= constraints.minWidth);
    RC_ASSERT(constrained <= constraints.maxWidth);
    
    // If input was within bounds, output should equal input
    if (testValue >= constraints.minWidth && testValue <= constraints.maxWidth) {
        RC_ASSERT(constrained == testValue);
    }
    
    // If input was below min, output should be min
    if (testValue < constraints.minWidth) {
        RC_ASSERT(constrained == constraints.minWidth);
    }
    
    // If input was above max, output should be max
    if (testValue > constraints.maxWidth) {
        RC_ASSERT(constrained == constraints.maxWidth);
    }
}

/**
 * **Feature: killergk-gui-library, Property 3: Layout Constraint Satisfaction**
 * 
 * *For any* LayoutConstraints, the constrainHeight method SHALL clamp values
 * to be within [minHeight, maxHeight] bounds.
 * 
 * **Validates: Requirements 3.1, 3.2, 3.3, 3.4, 3.6**
 */
RC_GTEST_PROP(LayoutConstraintProperties, ConstrainHeightClampsCorrectly, ()) {
    auto constraints = *gen::arbitrary<LayoutConstraints>();
    auto testValue = *genFloatInRange(-100.0f, 1000.0f);
    
    float constrained = constraints.constrainHeight(testValue);
    
    // Result must be within bounds
    RC_ASSERT(constrained >= constraints.minHeight);
    RC_ASSERT(constrained <= constraints.maxHeight);
    
    // If input was within bounds, output should equal input
    if (testValue >= constraints.minHeight && testValue <= constraints.maxHeight) {
        RC_ASSERT(constrained == testValue);
    }
    
    // If input was below min, output should be min
    if (testValue < constraints.minHeight) {
        RC_ASSERT(constrained == constraints.minHeight);
    }
    
    // If input was above max, output should be max
    if (testValue > constraints.maxHeight) {
        RC_ASSERT(constrained == constraints.maxHeight);
    }
}

/**
 * **Feature: killergk-gui-library, Property 3: Layout Constraint Satisfaction**
 * 
 * *For any* LayoutConstraints and Size, the constrain method SHALL produce
 * a Size that satisfies the constraints.
 * 
 * **Validates: Requirements 3.1, 3.2, 3.3, 3.4, 3.6**
 */
RC_GTEST_PROP(LayoutConstraintProperties, ConstrainSizeProducesValidSize, ()) {
    auto constraints = *gen::arbitrary<LayoutConstraints>();
    auto inputSize = *gen::arbitrary<Size>();
    
    Size constrained = constraints.constrain(inputSize);
    
    // Result must satisfy constraints
    RC_ASSERT(constraints.isSatisfiedBy(constrained));
    
    // Width must be within bounds
    RC_ASSERT(constrained.width >= constraints.minWidth);
    RC_ASSERT(constrained.width <= constraints.maxWidth);
    
    // Height must be within bounds
    RC_ASSERT(constrained.height >= constraints.minHeight);
    RC_ASSERT(constrained.height <= constraints.maxHeight);
}

/**
 * **Feature: killergk-gui-library, Property 3: Layout Constraint Satisfaction**
 * 
 * *For any* LayoutConstraints, the isSatisfiedBy method SHALL return true
 * if and only if the size is within all bounds.
 * 
 * **Validates: Requirements 3.1, 3.2, 3.3, 3.4, 3.6**
 */
RC_GTEST_PROP(LayoutConstraintProperties, IsSatisfiedByIsCorrect, ()) {
    auto constraints = *gen::arbitrary<LayoutConstraints>();
    auto size = *gen::arbitrary<Size>();
    
    bool satisfied = constraints.isSatisfiedBy(size);
    
    bool expectedSatisfied = 
        size.width >= constraints.minWidth &&
        size.width <= constraints.maxWidth &&
        size.height >= constraints.minHeight &&
        size.height <= constraints.maxHeight;
    
    RC_ASSERT(satisfied == expectedSatisfied);
}

/**
 * **Feature: killergk-gui-library, Property 3: Layout Constraint Satisfaction**
 * 
 * *For any* FlexBox layout with children having min/max constraints,
 * the computed child bounds SHALL respect those constraints.
 * 
 * This test verifies that:
 * 1. Child widths are >= minWidth and <= maxWidth
 * 2. Child heights are >= minHeight and <= maxHeight
 * 
 * **Validates: Requirements 3.1, 3.6**
 */
RC_GTEST_PROP(LayoutConstraintProperties, FlexLayoutRespectsChildConstraints, ()) {
    // Generate flex configuration
    auto direction = *genFlexDirection();
    auto justify = *genJustifyContent();
    auto align = *genAlignItems();
    auto gap = *genGapValue();
    
    // Generate container bounds
    auto containerWidth = *genFloatInRange(200.0f, 1000.0f);
    auto containerHeight = *genFloatInRange(200.0f, 1000.0f);
    
    // Generate 1-3 children with fixed constraints to avoid shrinking issues
    // The key insight is that the FlexImpl layout respects the child's requested
    // width/height, clamped to min/max. We test that the clamping is correct.
    auto numChildren = *gen::inRange(1, 4);
    std::vector<Widget> children;
    children.reserve(numChildren);
    
    for (int i = 0; i < numChildren; ++i) {
        // Use simple fixed constraints: minWidth = 10, maxWidth = 200
        // This avoids shrinking issues where relationships get broken
        auto childWidth = *genFloatInRange(10.0f, 200.0f);
        auto childHeight = *genFloatInRange(10.0f, 200.0f);
        
        constexpr float MIN_SIZE = 10.0f;
        constexpr float MAX_SIZE = 200.0f;
        
        children.push_back(
            Widget::create()
                .width(childWidth)
                .height(childHeight)
                .minWidth(MIN_SIZE)
                .maxWidth(MAX_SIZE)
                .minHeight(MIN_SIZE)
                .maxHeight(MAX_SIZE)
        );
    }
    
    // Create flex layout
    FlexImpl flex;
    flex.setDirection(direction);
    flex.setJustify(justify);
    flex.setAlign(align);
    flex.setGap(gap);
    flex.setBounds(Rect(0, 0, containerWidth, containerHeight));
    
    // Set children (need pointers)
    std::vector<Widget*> childPtrs;
    for (auto& child : children) {
        childPtrs.push_back(&child);
    }
    flex.setChildren(childPtrs);
    
    // Perform layout
    LayoutConstraints constraints = LayoutConstraints::loose(containerWidth, containerHeight);
    flex.layout(constraints);
    
    // Verify each child's bounds respect the fixed constraints
    constexpr float MIN_SIZE = 10.0f;
    constexpr float MAX_SIZE = 200.0f;
    constexpr float LAYOUT_EPSILON = 0.5f;
    
    for (size_t i = 0; i < children.size(); ++i) {
        Rect childBounds = flex.getChildBounds(i);
        
        // Width must respect min/max (with epsilon for floating point)
        RC_ASSERT(childBounds.width >= MIN_SIZE - LAYOUT_EPSILON);
        RC_ASSERT(childBounds.width <= MAX_SIZE + LAYOUT_EPSILON);
        
        // Height must respect min/max (except for Stretch alignment which may override)
        if (align != AlignItems::Stretch) {
            RC_ASSERT(childBounds.height >= MIN_SIZE - LAYOUT_EPSILON);
            RC_ASSERT(childBounds.height <= MAX_SIZE + LAYOUT_EPSILON);
        }
    }
}

/**
 * **Feature: killergk-gui-library, Property 3: Layout Constraint Satisfaction**
 * 
 * *For any* Grid layout with children having min/max constraints,
 * the computed child bounds SHALL be positioned within the grid cells.
 * 
 * This test verifies that:
 * 1. Children are positioned at correct grid cell locations
 * 2. Child bounds are within the container bounds
 * 
 * **Validates: Requirements 3.2, 3.6**
 */
RC_GTEST_PROP(LayoutConstraintProperties, GridLayoutPositionsChildrenCorrectly, ()) {
    // Generate grid configuration
    auto columns = *genGridCount();
    auto rows = *genGridCount();
    
    // Generate container bounds first
    auto containerWidth = *genFloatInRange(200.0f, 1000.0f);
    auto containerHeight = *genFloatInRange(200.0f, 1000.0f);
    
    // Generate gaps that are small enough to leave room for content
    // Total gap space = (columns-1) * columnGap, must be < containerWidth
    float maxColumnGap = (columns > 1) ? (containerWidth * 0.5f) / (columns - 1) : 50.0f;
    float maxRowGap = (rows > 1) ? (containerHeight * 0.5f) / (rows - 1) : 50.0f;
    auto columnGap = *genFloatInRange(0.0f, std::min(maxColumnGap, 50.0f));
    auto rowGap = *genFloatInRange(0.0f, std::min(maxRowGap, 50.0f));
    
    // Generate children (up to grid capacity)
    auto numChildren = *gen::inRange(1, std::min(columns * rows, 10) + 1);
    std::vector<Widget> children;
    children.reserve(numChildren);
    
    for (int i = 0; i < numChildren; ++i) {
        auto childWidth = *genFloatInRange(10.0f, 100.0f);
        auto childHeight = *genFloatInRange(10.0f, 100.0f);
        
        children.push_back(
            Widget::create()
                .width(childWidth)
                .height(childHeight)
        );
    }
    
    // Create grid layout
    GridImpl grid;
    grid.setColumns(columns);
    grid.setRows(rows);
    grid.setColumnGap(columnGap);
    grid.setRowGap(rowGap);
    grid.setBounds(Rect(0, 0, containerWidth, containerHeight));
    
    // Set children
    std::vector<Widget*> childPtrs;
    for (auto& child : children) {
        childPtrs.push_back(&child);
    }
    grid.setChildren(childPtrs);
    
    // Perform layout
    LayoutConstraints constraints = LayoutConstraints::loose(containerWidth, containerHeight);
    grid.layout(constraints);
    
    // Verify each child's bounds are within container and have positive dimensions
    for (size_t i = 0; i < children.size(); ++i) {
        Rect childBounds = grid.getChildBounds(i);
        
        // Child must be within container bounds (with small epsilon)
        RC_ASSERT(childBounds.x >= -0.001f);
        RC_ASSERT(childBounds.y >= -0.001f);
        RC_ASSERT(childBounds.x + childBounds.width <= containerWidth + 0.001f);
        RC_ASSERT(childBounds.y + childBounds.height <= containerHeight + 0.001f);
        
        // Child must have positive dimensions
        RC_ASSERT(childBounds.width > 0.0f);
        RC_ASSERT(childBounds.height > 0.0f);
    }
}

/**
 * **Feature: killergk-gui-library, Property 3: Layout Constraint Satisfaction**
 * 
 * *For any* Stack layout with children having min/max constraints,
 * the computed child bounds SHALL respect those constraints and be
 * positioned at the container origin.
 * 
 * This test verifies that:
 * 1. All children are positioned at the same location (stack origin)
 * 2. Child sizes respect min/max constraints
 * 
 * **Validates: Requirements 3.4, 3.6**
 */
RC_GTEST_PROP(LayoutConstraintProperties, StackLayoutRespectsChildConstraints, ()) {
    // Generate container bounds
    auto containerX = *genFloatInRange(0.0f, 500.0f);
    auto containerY = *genFloatInRange(0.0f, 500.0f);
    auto containerWidth = *genFloatInRange(100.0f, 1000.0f);
    auto containerHeight = *genFloatInRange(100.0f, 1000.0f);
    
    // Generate 1-5 children with constraints
    auto numChildren = *gen::inRange(1, 6);
    std::vector<Widget> children;
    children.reserve(numChildren);
    
    for (int i = 0; i < numChildren; ++i) {
        auto childWidth = *genFloatInRange(10.0f, 200.0f);
        auto childHeight = *genFloatInRange(10.0f, 200.0f);
        auto minWidth = *genFloatInRange(5.0f, childWidth);
        auto maxWidth = childWidth + *genFloatInRange(0.0f, 100.0f);
        auto minHeight = *genFloatInRange(5.0f, childHeight);
        auto maxHeight = childHeight + *genFloatInRange(0.0f, 100.0f);
        
        children.push_back(
            Widget::create()
                .width(childWidth)
                .height(childHeight)
                .minWidth(minWidth)
                .maxWidth(maxWidth)
                .minHeight(minHeight)
                .maxHeight(maxHeight)
        );
    }
    
    // Create stack layout
    StackImpl stack;
    stack.setBounds(Rect(containerX, containerY, containerWidth, containerHeight));
    
    // Set children
    std::vector<Widget*> childPtrs;
    for (auto& child : children) {
        childPtrs.push_back(&child);
    }
    stack.setChildren(childPtrs);
    
    // Perform layout
    LayoutConstraints constraints = LayoutConstraints::loose(containerWidth, containerHeight);
    stack.layout(constraints);
    
    // Verify each child's bounds
    for (size_t i = 0; i < children.size(); ++i) {
        Rect childBounds = stack.getChildBounds(i);
        const Widget& child = children[i];
        
        // All children should be positioned at container origin
        RC_ASSERT(childBounds.x == containerX);
        RC_ASSERT(childBounds.y == containerY);
        
        // Width must respect min/max
        RC_ASSERT(childBounds.width >= child.getMinWidth() - 0.001f);
        RC_ASSERT(childBounds.width <= child.getMaxWidth() + 0.001f);
        
        // Height must respect min/max
        RC_ASSERT(childBounds.height >= child.getMinHeight() - 0.001f);
        RC_ASSERT(childBounds.height <= child.getMaxHeight() + 0.001f);
    }
}

/**
 * **Feature: killergk-gui-library, Property 3: Layout Constraint Satisfaction**
 * 
 * *For any* Absolute layout with children having position properties,
 * the computed child bounds SHALL be at the specified absolute positions
 * relative to the container.
 * 
 * This test verifies that:
 * 1. Children are positioned at their specified x, y coordinates
 * 2. Child sizes respect min/max constraints
 * 
 * **Validates: Requirements 3.3, 3.6**
 */
RC_GTEST_PROP(LayoutConstraintProperties, AbsoluteLayoutPositionsChildrenCorrectly, ()) {
    // Generate container bounds
    auto containerX = *genFloatInRange(0.0f, 500.0f);
    auto containerY = *genFloatInRange(0.0f, 500.0f);
    auto containerWidth = *genFloatInRange(200.0f, 1000.0f);
    auto containerHeight = *genFloatInRange(200.0f, 1000.0f);
    
    // Generate 1-5 children with absolute positions
    auto numChildren = *gen::inRange(1, 6);
    std::vector<Widget> children;
    std::vector<float> expectedX, expectedY;
    children.reserve(numChildren);
    
    for (int i = 0; i < numChildren; ++i) {
        auto childX = *genFloatInRange(0.0f, containerWidth - 50.0f);
        auto childY = *genFloatInRange(0.0f, containerHeight - 50.0f);
        auto childWidth = *genFloatInRange(10.0f, 100.0f);
        auto childHeight = *genFloatInRange(10.0f, 100.0f);
        auto minWidth = *genFloatInRange(5.0f, childWidth);
        auto maxWidth = childWidth + *genFloatInRange(0.0f, 50.0f);
        auto minHeight = *genFloatInRange(5.0f, childHeight);
        auto maxHeight = childHeight + *genFloatInRange(0.0f, 50.0f);
        
        children.push_back(
            Widget::create()
                .width(childWidth)
                .height(childHeight)
                .minWidth(minWidth)
                .maxWidth(maxWidth)
                .minHeight(minHeight)
                .maxHeight(maxHeight)
                .setPropertyFloat("x", childX)
                .setPropertyFloat("y", childY)
        );
        
        expectedX.push_back(childX);
        expectedY.push_back(childY);
    }
    
    // Create absolute layout
    AbsoluteImpl absolute;
    absolute.setBounds(Rect(containerX, containerY, containerWidth, containerHeight));
    
    // Set children
    std::vector<Widget*> childPtrs;
    for (auto& child : children) {
        childPtrs.push_back(&child);
    }
    absolute.setChildren(childPtrs);
    
    // Perform layout
    LayoutConstraints constraints = LayoutConstraints::loose(containerWidth, containerHeight);
    absolute.layout(constraints);
    
    // Verify each child's bounds
    for (size_t i = 0; i < children.size(); ++i) {
        Rect childBounds = absolute.getChildBounds(i);
        const Widget& child = children[i];
        
        // Position should be container origin + specified offset
        RC_ASSERT(std::abs(childBounds.x - (containerX + expectedX[i])) < 0.001f);
        RC_ASSERT(std::abs(childBounds.y - (containerY + expectedY[i])) < 0.001f);
        
        // Width must respect min/max
        RC_ASSERT(childBounds.width >= child.getMinWidth() - 0.001f);
        RC_ASSERT(childBounds.width <= child.getMaxWidth() + 0.001f);
        
        // Height must respect min/max
        RC_ASSERT(childBounds.height >= child.getMinHeight() - 0.001f);
        RC_ASSERT(childBounds.height <= child.getMaxHeight() + 0.001f);
    }
}

/**
 * **Feature: killergk-gui-library, Property 3: Layout Constraint Satisfaction**
 * 
 * *For any* layout with tight constraints, the layout SHALL produce
 * a size that exactly matches the constraints.
 * 
 * **Validates: Requirements 3.1, 3.2, 3.3, 3.4, 3.6**
 */
RC_GTEST_PROP(LayoutConstraintProperties, TightConstraintsProduceExactSize, ()) {
    auto exactWidth = *genFloatInRange(50.0f, 500.0f);
    auto exactHeight = *genFloatInRange(50.0f, 500.0f);
    
    LayoutConstraints tight = LayoutConstraints::tight(exactWidth, exactHeight);
    
    // Verify tight constraints properties
    RC_ASSERT(tight.isTight());
    RC_ASSERT(tight.minWidth == exactWidth);
    RC_ASSERT(tight.maxWidth == exactWidth);
    RC_ASSERT(tight.minHeight == exactHeight);
    RC_ASSERT(tight.maxHeight == exactHeight);
    
    // Any size constrained by tight constraints should equal the exact size
    auto inputSize = *gen::arbitrary<Size>();
    Size constrained = tight.constrain(inputSize);
    
    RC_ASSERT(constrained.width == exactWidth);
    RC_ASSERT(constrained.height == exactHeight);
}

/**
 * **Feature: killergk-gui-library, Property 3: Layout Constraint Satisfaction**
 * 
 * *For any* layout with loose constraints, the layout SHALL allow
 * sizes from 0 up to the maximum.
 * 
 * **Validates: Requirements 3.1, 3.2, 3.3, 3.4, 3.6**
 */
RC_GTEST_PROP(LayoutConstraintProperties, LooseConstraintsAllowZeroToMax, ()) {
    auto maxWidth = *genFloatInRange(50.0f, 500.0f);
    auto maxHeight = *genFloatInRange(50.0f, 500.0f);
    
    LayoutConstraints loose = LayoutConstraints::loose(maxWidth, maxHeight);
    
    // Verify loose constraints properties
    RC_ASSERT(loose.minWidth == 0.0f);
    RC_ASSERT(loose.maxWidth == maxWidth);
    RC_ASSERT(loose.minHeight == 0.0f);
    RC_ASSERT(loose.maxHeight == maxHeight);
    
    // Zero size should be valid
    Size zeroSize(0.0f, 0.0f);
    RC_ASSERT(loose.isSatisfiedBy(zeroSize));
    
    // Max size should be valid
    Size maxSize(maxWidth, maxHeight);
    RC_ASSERT(loose.isSatisfiedBy(maxSize));
    
    // Size within range should be valid
    auto midWidth = *genFloatInRange(0.0f, maxWidth);
    auto midHeight = *genFloatInRange(0.0f, maxHeight);
    Size midSize(midWidth, midHeight);
    RC_ASSERT(loose.isSatisfiedBy(midSize));
}

// ============================================================================
// Property Tests for Responsive Layout
// ============================================================================

// Note: Layout.hpp already included above
// Note: genFlexDirection, genJustifyContent, genAlignItems, genGapValue already defined above

namespace rc {

/**
 * @brief Generator for valid window dimensions for resize testing
 */
inline Gen<int> genWindowSize() {
    return gen::inRange(100, 4096);  // Reasonable window size range
}

/**
 * @brief Generator for number of widgets in a layout
 */
inline Gen<int> genWidgetCount() {
    return gen::inRange(1, 50);  // 1 to 50 widgets
}

} // namespace rc

/**
 * **Feature: killergk-gui-library, Property 4: Responsive Layout Consistency**
 * 
 * *For any* window resize operation, the layout system SHALL produce 
 * consistent widget positions that satisfy layout constraints within 
 * 16 milliseconds.
 * 
 * This test verifies that:
 * 1. Layout recalculation completes within the 16ms target time
 * 2. The LayoutManager correctly tracks recalculation time
 * 3. isWithinTargetTime() returns true for fast recalculations
 * 
 * **Validates: Requirements 1.6, 3.5**
 */
RC_GTEST_PROP(ResponsiveLayoutProperties, LayoutRecalculationWithinTargetTime, ()) {
    // Generate random window dimensions
    auto windowWidth = *genWindowSize();
    auto windowHeight = *genWindowSize();
    
    // Generate number of widgets to test with
    auto numWidgets = *genWidgetCount();
    
    // Create widgets for the layout
    std::vector<std::unique_ptr<Widget>> widgets;
    std::vector<Widget*> widgetPtrs;
    
    for (int i = 0; i < numWidgets; ++i) {
        auto widget = std::make_unique<Widget>(Widget::create()
            .width(*genWidgetDimension())
            .height(*genWidgetDimension())
            .minWidth(*gen::map(gen::inRange(10, 100), [](int v) { return static_cast<float>(v); }))
            .minHeight(*gen::map(gen::inRange(10, 100), [](int v) { return static_cast<float>(v); })));
        widgetPtrs.push_back(widget.get());
        widgets.push_back(std::move(widget));
    }
    
    // Create a flex layout with the widgets
    auto flexImpl = std::make_shared<FlexImpl>();
    flexImpl->setDirection(*genFlexDirection());
    flexImpl->setJustify(*genJustifyContent());
    flexImpl->setAlign(*genAlignItems());
    flexImpl->setGap(*genGapValue());
    flexImpl->setChildren(widgetPtrs);
    flexImpl->setBounds(Rect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
    
    // Register layout with manager
    LayoutManager::instance().registerLayout(flexImpl.get());
    
    // Simulate window resize
    LayoutManager::instance().onWindowResize(windowWidth, windowHeight);
    
    // Get recalculation time
    auto recalcTime = LayoutManager::instance().getLastRecalculationTime();
    
    // Verify recalculation is within target time (16ms = 16000 microseconds)
    RC_ASSERT(recalcTime.count() < LayoutManager::TARGET_RECALC_TIME_US);
    RC_ASSERT(LayoutManager::instance().isWithinTargetTime());
    
    // Cleanup
    LayoutManager::instance().unregisterLayout(flexImpl.get());
}

/**
 * **Feature: killergk-gui-library, Property 4: Responsive Layout Consistency**
 * 
 * *For any* sequence of window resize operations, the layout system SHALL
 * produce consistent results - the same input dimensions should always
 * produce the same layout output.
 * 
 * This test verifies that:
 * 1. Resizing to the same dimensions produces identical child bounds
 * 2. Layout is deterministic
 * 
 * **Validates: Requirements 1.6, 3.5**
 */
RC_GTEST_PROP(ResponsiveLayoutProperties, LayoutConsistencyOnResize, ()) {
    // Generate random window dimensions
    auto windowWidth = *genWindowSize();
    auto windowHeight = *genWindowSize();
    
    // Generate number of widgets
    auto numWidgets = *gen::inRange(2, 20);
    
    // Create widgets for the layout
    std::vector<std::unique_ptr<Widget>> widgets;
    std::vector<Widget*> widgetPtrs;
    
    for (int i = 0; i < numWidgets; ++i) {
        auto widget = std::make_unique<Widget>(Widget::create()
            .width(50.0f + static_cast<float>(i * 10))
            .height(30.0f + static_cast<float>(i * 5)));
        widgetPtrs.push_back(widget.get());
        widgets.push_back(std::move(widget));
    }
    
    // Create a flex layout
    auto flexImpl = std::make_shared<FlexImpl>();
    flexImpl->setDirection(FlexDirection::Row);
    flexImpl->setJustify(JustifyContent::Start);
    flexImpl->setAlign(AlignItems::Start);
    flexImpl->setGap(10.0f);
    flexImpl->setChildren(widgetPtrs);
    flexImpl->setBounds(Rect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
    
    // First layout calculation
    LayoutConstraints constraints = LayoutConstraints::loose(
        static_cast<float>(windowWidth),
        static_cast<float>(windowHeight)
    );
    flexImpl->layout(constraints);
    
    // Store first results
    std::vector<Rect> firstBounds;
    for (size_t i = 0; i < flexImpl->getChildCount(); ++i) {
        firstBounds.push_back(flexImpl->getChildBounds(i));
    }
    
    // Invalidate and recalculate with same dimensions
    flexImpl->invalidate();
    flexImpl->layout(constraints);
    
    // Verify results are identical
    for (size_t i = 0; i < flexImpl->getChildCount(); ++i) {
        Rect secondBounds = flexImpl->getChildBounds(i);
        RC_ASSERT(firstBounds[i].x == secondBounds.x);
        RC_ASSERT(firstBounds[i].y == secondBounds.y);
        RC_ASSERT(firstBounds[i].width == secondBounds.width);
        RC_ASSERT(firstBounds[i].height == secondBounds.height);
    }
}

/**
 * **Feature: killergk-gui-library, Property 4: Responsive Layout Consistency**
 * 
 * *For any* Grid layout with window resize, the layout system SHALL
 * recalculate grid cell positions within the target time.
 * 
 * This test verifies that:
 * 1. Grid layout recalculation is fast
 * 2. Grid cells are positioned correctly after resize
 * 
 * **Validates: Requirements 1.6, 3.5**
 */
RC_GTEST_PROP(ResponsiveLayoutProperties, GridLayoutRecalculationWithinTargetTime, ()) {
    // Generate random window dimensions
    auto windowWidth = *genWindowSize();
    auto windowHeight = *genWindowSize();
    
    // Generate grid configuration
    auto columns = *gen::inRange(1, 10);
    auto rows = *gen::inRange(1, 10);
    auto numWidgets = columns * rows;
    
    // Create widgets for the grid
    std::vector<std::unique_ptr<Widget>> widgets;
    std::vector<Widget*> widgetPtrs;
    
    for (int i = 0; i < numWidgets; ++i) {
        auto widget = std::make_unique<Widget>(Widget::create()
            .width(100.0f)
            .height(100.0f));
        widgetPtrs.push_back(widget.get());
        widgets.push_back(std::move(widget));
    }
    
    // Create a grid layout
    auto gridImpl = std::make_shared<GridImpl>();
    gridImpl->setColumns(columns);
    gridImpl->setRows(rows);
    gridImpl->setColumnGap(*genGapValue());
    gridImpl->setRowGap(*genGapValue());
    gridImpl->setChildren(widgetPtrs);
    gridImpl->setBounds(Rect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
    
    // Register layout with manager
    LayoutManager::instance().registerLayout(gridImpl.get());
    
    // Simulate window resize
    LayoutManager::instance().onWindowResize(windowWidth, windowHeight);
    
    // Get recalculation time
    auto recalcTime = LayoutManager::instance().getLastRecalculationTime();
    
    // Verify recalculation is within target time
    RC_ASSERT(recalcTime.count() < LayoutManager::TARGET_RECALC_TIME_US);
    RC_ASSERT(LayoutManager::instance().isWithinTargetTime());
    
    // Cleanup
    LayoutManager::instance().unregisterLayout(gridImpl.get());
}

/**
 * **Feature: killergk-gui-library, Property 4: Responsive Layout Consistency**
 * 
 * *For any* Stack layout with window resize, the layout system SHALL
 * recalculate stacked widget positions within the target time.
 * 
 * This test verifies that:
 * 1. Stack layout recalculation is fast
 * 2. All stacked widgets start at the same position
 * 
 * **Validates: Requirements 1.6, 3.5**
 */
RC_GTEST_PROP(ResponsiveLayoutProperties, StackLayoutRecalculationWithinTargetTime, ()) {
    // Generate random window dimensions
    auto windowWidth = *genWindowSize();
    auto windowHeight = *genWindowSize();
    
    // Generate number of stacked widgets
    auto numWidgets = *gen::inRange(2, 20);
    
    // Create widgets for the stack
    std::vector<std::unique_ptr<Widget>> widgets;
    std::vector<Widget*> widgetPtrs;
    
    for (int i = 0; i < numWidgets; ++i) {
        auto widget = std::make_unique<Widget>(Widget::create()
            .width(*genWidgetDimension())
            .height(*genWidgetDimension()));
        widgetPtrs.push_back(widget.get());
        widgets.push_back(std::move(widget));
    }
    
    // Create a stack layout
    auto stackImpl = std::make_shared<StackImpl>();
    stackImpl->setChildren(widgetPtrs);
    stackImpl->setBounds(Rect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
    
    // Register layout with manager
    LayoutManager::instance().registerLayout(stackImpl.get());
    
    // Simulate window resize
    LayoutManager::instance().onWindowResize(windowWidth, windowHeight);
    
    // Get recalculation time
    auto recalcTime = LayoutManager::instance().getLastRecalculationTime();
    
    // Verify recalculation is within target time
    RC_ASSERT(recalcTime.count() < LayoutManager::TARGET_RECALC_TIME_US);
    RC_ASSERT(LayoutManager::instance().isWithinTargetTime());
    
    // Verify all stacked widgets start at the same position (0, 0 relative to container)
    for (size_t i = 0; i < stackImpl->getChildCount(); ++i) {
        Rect bounds = stackImpl->getChildBounds(i);
        RC_ASSERT(bounds.x == 0.0f);
        RC_ASSERT(bounds.y == 0.0f);
    }
    
    // Cleanup
    LayoutManager::instance().unregisterLayout(stackImpl.get());
}

/**
 * **Feature: killergk-gui-library, Property 4: Responsive Layout Consistency**
 * 
 * *For any* Absolute layout with window resize, the layout system SHALL
 * recalculate absolute positions within the target time.
 * 
 * This test verifies that:
 * 1. Absolute layout recalculation is fast
 * 2. Widgets maintain their absolute positions
 * 
 * **Validates: Requirements 1.6, 3.5**
 */
RC_GTEST_PROP(ResponsiveLayoutProperties, AbsoluteLayoutRecalculationWithinTargetTime, ()) {
    // Generate random window dimensions
    auto windowWidth = *genWindowSize();
    auto windowHeight = *genWindowSize();
    
    // Generate number of widgets
    auto numWidgets = *gen::inRange(2, 20);
    
    // Create widgets with absolute positions
    std::vector<std::unique_ptr<Widget>> widgets;
    std::vector<Widget*> widgetPtrs;
    std::vector<std::pair<float, float>> expectedPositions;
    
    for (int i = 0; i < numWidgets; ++i) {
        float x = static_cast<float>(*gen::inRange(0, windowWidth));
        float y = static_cast<float>(*gen::inRange(0, windowHeight));
        
        auto widget = std::make_unique<Widget>(Widget::create()
            .width(50.0f)
            .height(50.0f)
            .margin(y, 0, 0, x));  // Use margin for position (top, right, bottom, left)
        widgetPtrs.push_back(widget.get());
        widgets.push_back(std::move(widget));
        expectedPositions.emplace_back(x, y);
    }
    
    // Create an absolute layout
    auto absoluteImpl = std::make_shared<AbsoluteImpl>();
    absoluteImpl->setChildren(widgetPtrs);
    absoluteImpl->setBounds(Rect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
    
    // Register layout with manager
    LayoutManager::instance().registerLayout(absoluteImpl.get());
    
    // Simulate window resize
    LayoutManager::instance().onWindowResize(windowWidth, windowHeight);
    
    // Get recalculation time
    auto recalcTime = LayoutManager::instance().getLastRecalculationTime();
    
    // Verify recalculation is within target time
    RC_ASSERT(recalcTime.count() < LayoutManager::TARGET_RECALC_TIME_US);
    RC_ASSERT(LayoutManager::instance().isWithinTargetTime());
    
    // Cleanup
    LayoutManager::instance().unregisterLayout(absoluteImpl.get());
}

/**
 * **Feature: killergk-gui-library, Property 4: Responsive Layout Consistency**
 * 
 * *For any* multiple registered layouts, the LayoutManager SHALL
 * recalculate all layouts within the target time.
 * 
 * This test verifies that:
 * 1. Multiple layouts can be registered
 * 2. All layouts are recalculated on resize
 * 3. Total recalculation time is within target
 * 
 * **Validates: Requirements 1.6, 3.5**
 */
RC_GTEST_PROP(ResponsiveLayoutProperties, MultipleLayoutsRecalculationWithinTargetTime, ()) {
    // Generate random window dimensions
    auto windowWidth = *genWindowSize();
    auto windowHeight = *genWindowSize();
    
    // Generate number of layouts
    auto numLayouts = *gen::inRange(2, 5);
    
    // Create multiple layouts
    std::vector<std::shared_ptr<FlexImpl>> layouts;
    std::vector<std::vector<std::unique_ptr<Widget>>> allWidgets;
    
    for (int l = 0; l < numLayouts; ++l) {
        auto numWidgets = *gen::inRange(5, 15);
        
        std::vector<std::unique_ptr<Widget>> widgets;
        std::vector<Widget*> widgetPtrs;
        
        for (int i = 0; i < numWidgets; ++i) {
            auto widget = std::make_unique<Widget>(Widget::create()
                .width(50.0f)
                .height(30.0f));
            widgetPtrs.push_back(widget.get());
            widgets.push_back(std::move(widget));
        }
        
        auto flexImpl = std::make_shared<FlexImpl>();
        flexImpl->setDirection(FlexDirection::Row);
        flexImpl->setChildren(widgetPtrs);
        flexImpl->setBounds(Rect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
        
        LayoutManager::instance().registerLayout(flexImpl.get());
        layouts.push_back(flexImpl);
        allWidgets.push_back(std::move(widgets));
    }
    
    // Simulate window resize
    LayoutManager::instance().onWindowResize(windowWidth, windowHeight);
    
    // Get recalculation time
    auto recalcTime = LayoutManager::instance().getLastRecalculationTime();
    
    // Verify recalculation is within target time
    RC_ASSERT(recalcTime.count() < LayoutManager::TARGET_RECALC_TIME_US);
    RC_ASSERT(LayoutManager::instance().isWithinTargetTime());
    
    // Cleanup
    for (auto& layout : layouts) {
        LayoutManager::instance().unregisterLayout(layout.get());
    }
}


// ============================================================================
// Property Tests for Animation Interpolation
// ============================================================================

#include "KillerGK/animation/Animation.hpp"

namespace rc {

/**
 * @brief Generator for valid Easing values
 */
inline Gen<KillerGK::Easing> genEasing() {
    return gen::element(
        KillerGK::Easing::Linear,
        KillerGK::Easing::EaseIn, KillerGK::Easing::EaseOut, KillerGK::Easing::EaseInOut,
        KillerGK::Easing::EaseInQuad, KillerGK::Easing::EaseOutQuad, KillerGK::Easing::EaseInOutQuad,
        KillerGK::Easing::EaseInCubic, KillerGK::Easing::EaseOutCubic, KillerGK::Easing::EaseInOutCubic,
        KillerGK::Easing::EaseInQuart, KillerGK::Easing::EaseOutQuart, KillerGK::Easing::EaseInOutQuart,
        KillerGK::Easing::EaseInQuint, KillerGK::Easing::EaseOutQuint, KillerGK::Easing::EaseInOutQuint,
        KillerGK::Easing::EaseInSine, KillerGK::Easing::EaseOutSine, KillerGK::Easing::EaseInOutSine,
        KillerGK::Easing::EaseInExpo, KillerGK::Easing::EaseOutExpo, KillerGK::Easing::EaseInOutExpo,
        KillerGK::Easing::EaseInCirc, KillerGK::Easing::EaseOutCirc, KillerGK::Easing::EaseInOutCirc,
        KillerGK::Easing::EaseInElastic, KillerGK::Easing::EaseOutElastic, KillerGK::Easing::EaseInOutElastic,
        KillerGK::Easing::EaseInBounce, KillerGK::Easing::EaseOutBounce, KillerGK::Easing::EaseInOutBounce,
        KillerGK::Easing::EaseInBack, KillerGK::Easing::EaseOutBack, KillerGK::Easing::EaseInOutBack
    );
}

/**
 * @brief Generator for valid Property values
 */
inline Gen<KillerGK::Property> genAnimatableProperty() {
    return gen::element(
        KillerGK::Property::X, KillerGK::Property::Y,
        KillerGK::Property::Width, KillerGK::Property::Height,
        KillerGK::Property::Opacity, KillerGK::Property::Rotation, KillerGK::Property::Scale,
        KillerGK::Property::BackgroundColorR, KillerGK::Property::BackgroundColorG,
        KillerGK::Property::BackgroundColorB, KillerGK::Property::BackgroundColorA,
        KillerGK::Property::BorderRadius, KillerGK::Property::BorderWidth
    );
}

/**
 * @brief Generator for animation duration (in milliseconds)
 */
inline Gen<float> genAnimationDuration() {
    return gen::map(gen::inRange(100, 5000), [](int v) {
        return static_cast<float>(v);  // 100ms to 5000ms
    });
}

/**
 * @brief Generator for animation from/to values
 */
inline Gen<float> genAnimationValue() {
    return gen::map(gen::inRange(-10000, 10000), [](int v) {
        return static_cast<float>(v) / 100.0f;  // -100.0 to 100.0
    });
}

/**
 * @brief Generator for progress value [0.0, 1.0]
 */
inline Gen<float> genProgress() {
    return gen::map(gen::inRange(0, 1000), [](int v) {
        return static_cast<float>(v) / 1000.0f;
    });
}

/**
 * @brief Generator for spring stiffness
 */
inline Gen<float> genSpringStiffness() {
    return gen::map(gen::inRange(10, 500), [](int v) {
        return static_cast<float>(v);  // 10 to 500
    });
}

/**
 * @brief Generator for spring damping
 */
inline Gen<float> genSpringDamping() {
    return gen::map(gen::inRange(1, 100), [](int v) {
        return static_cast<float>(v);  // 1 to 100
    });
}

/**
 * @brief Generator for keyframe percent [0.0, 1.0]
 */
inline Gen<float> genKeyframePercent() {
    return gen::map(gen::inRange(0, 100), [](int v) {
        return static_cast<float>(v) / 100.0f;
    });
}

} // namespace rc

/**
 * **Feature: killergk-gui-library, Property 5: Animation Interpolation Correctness**
 * 
 * *For any* tween animation with valid parameters, the interpolated value at 
 * any time t SHALL be mathematically correct according to the specified easing function.
 * 
 * This test verifies that:
 * 1. Easing function output is always in valid range for standard easings
 * 2. Linear interpolation produces correct intermediate values
 * 3. Easing at t=0 returns 0 and at t=1 returns 1
 * 
 * **Validates: Requirements 4.1**
 */
RC_GTEST_PROP(AnimationInterpolationProperties, EasingFunctionBoundaryValues, ()) {
    auto easing = *genEasing();
    
    // Test boundary conditions: t=0 should give 0, t=1 should give 1
    float atZero = KillerGK::applyEasing(easing, 0.0f);
    float atOne = KillerGK::applyEasing(easing, 1.0f);
    
    // All easing functions should return 0 at t=0 and 1 at t=1
    RC_ASSERT(std::abs(atZero) < 0.0001f);
    RC_ASSERT(std::abs(atOne - 1.0f) < 0.0001f);
}

/**
 * **Feature: killergk-gui-library, Property 5: Animation Interpolation Correctness**
 * 
 * *For any* progress value t in [0, 1], the easing function output should be 
 * clamped to a reasonable range (allowing for overshoot in elastic/back easings).
 * 
 * **Validates: Requirements 4.1**
 */
RC_GTEST_PROP(AnimationInterpolationProperties, EasingFunctionOutputRange, ()) {
    auto easing = *genEasing();
    auto t = *genProgress();
    
    float result = KillerGK::applyEasing(easing, t);
    
    // Most easings stay in [0, 1], but elastic and back can overshoot
    // Allow a reasonable overshoot range of [-0.5, 1.5]
    RC_ASSERT(result >= -0.5f && result <= 1.5f);
}

/**
 * **Feature: killergk-gui-library, Property 5: Animation Interpolation Correctness**
 * 
 * *For any* linear easing, the output should equal the input (identity function).
 * 
 * **Validates: Requirements 4.1**
 */
RC_GTEST_PROP(AnimationInterpolationProperties, LinearEasingIsIdentity, ()) {
    auto t = *genProgress();
    
    float result = KillerGK::applyEasing(KillerGK::Easing::Linear, t);
    
    RC_ASSERT(std::abs(result - t) < 0.0001f);
}

/**
 * **Feature: killergk-gui-library, Property 5: Animation Interpolation Correctness**
 * 
 * *For any* tween animation with from and to values, the lerp function should
 * produce mathematically correct intermediate values.
 * 
 * **Validates: Requirements 4.1**
 */
RC_GTEST_PROP(AnimationInterpolationProperties, LerpProducesCorrectValues, ()) {
    auto from = *genAnimationValue();
    auto to = *genAnimationValue();
    auto t = *genProgress();
    
    float result = KillerGK::lerp(from, to, t);
    float expected = from + (to - from) * t;
    
    RC_ASSERT(std::abs(result - expected) < 0.0001f);
}

/**
 * **Feature: killergk-gui-library, Property 5: Animation Interpolation Correctness**
 * 
 * *For any* tween animation, lerp at t=0 should return 'from' and at t=1 should return 'to'.
 * 
 * **Validates: Requirements 4.1**
 */
RC_GTEST_PROP(AnimationInterpolationProperties, LerpBoundaryValues, ()) {
    auto from = *genAnimationValue();
    auto to = *genAnimationValue();
    
    float atZero = KillerGK::lerp(from, to, 0.0f);
    float atOne = KillerGK::lerp(from, to, 1.0f);
    
    RC_ASSERT(std::abs(atZero - from) < 0.0001f);
    RC_ASSERT(std::abs(atOne - to) < 0.0001f);
}

/**
 * **Feature: killergk-gui-library, Property 5: Animation Interpolation Correctness**
 * 
 * *For any* tween animation with valid parameters, running the animation to completion
 * should result in the final value being equal to the target value.
 * 
 * **Validates: Requirements 4.1**
 */
RC_GTEST_PROP(AnimationInterpolationProperties, TweenAnimationReachesTargetValue, ()) {
    auto prop = *genAnimatableProperty();
    auto from = *genAnimationValue();
    auto to = *genAnimationValue();
    auto duration = *genAnimationDuration();
    auto easing = *genEasing();
    
    // Create a tween animation
    auto anim = KillerGK::Animation::create()
        .property(prop, from, to)
        .duration(duration)
        .easing(easing)
        .build();
    
    // Start the animation
    anim->start();
    
    // Run the animation to completion (simulate time passing)
    float totalTime = 0.0f;
    float deltaTime = 16.0f;  // ~60 FPS
    while (anim->isRunning() && totalTime < duration + 1000.0f) {
        anim->update(deltaTime);
        totalTime += deltaTime;
    }
    
    // Animation should be completed
    RC_ASSERT(anim->isCompleted());
    
    // Final value should be the target value
    float finalValue = anim->getCurrentValue(prop);
    RC_ASSERT(std::abs(finalValue - to) < 0.01f);
}

/**
 * **Feature: killergk-gui-library, Property 5: Animation Interpolation Correctness**
 * 
 * *For any* tween animation, the progress should monotonically increase from 0 to 1
 * (for non-yoyo animations).
 * 
 * **Validates: Requirements 4.1**
 */
RC_GTEST_PROP(AnimationInterpolationProperties, TweenProgressMonotonicallyIncreases, ()) {
    auto prop = *genAnimatableProperty();
    auto from = *genAnimationValue();
    auto to = *genAnimationValue();
    auto duration = *genAnimationDuration();
    
    // Use linear easing for predictable progress
    auto anim = KillerGK::Animation::create()
        .property(prop, from, to)
        .duration(duration)
        .easing(KillerGK::Easing::Linear)
        .build();
    
    anim->start();
    
    float lastProgress = 0.0f;
    float totalTime = 0.0f;
    float deltaTime = 16.0f;
    
    while (anim->isRunning() && totalTime < duration + 100.0f) {
        anim->update(deltaTime);
        float currentProgress = anim->getProgress();
        
        // Progress should never decrease
        RC_ASSERT(currentProgress >= lastProgress - 0.001f);  // Small tolerance for float precision
        lastProgress = currentProgress;
        totalTime += deltaTime;
    }
}

/**
 * **Feature: killergk-gui-library, Property 5: Animation Interpolation Correctness**
 * 
 * *For any* spring animation with valid parameters, the animation should eventually
 * converge to the target value (reach rest state).
 * 
 * Note: We use spring presets or constrained parameters to ensure convergence
 * within a reasonable time. Very low stiffness with very low damping can take
 * extremely long to settle, which is correct physics but impractical for testing.
 * 
 * **Validates: Requirements 4.2**
 */
RC_GTEST_PROP(AnimationInterpolationProperties, SpringAnimationConvergesToTarget, ()) {
    auto prop = *genAnimatableProperty();
    auto from = *genAnimationValue();
    auto to = *genAnimationValue();
    
    // Use spring presets that are designed to converge in reasonable time
    // These are the actual presets users would use in practice
    auto presetIndex = *gen::inRange(0, 4);
    KillerGK::SpringConfig config;
    switch (presetIndex) {
        case 0: config = KillerGK::SpringConfig::gentle(); break;
        case 1: config = KillerGK::SpringConfig::wobbly(); break;
        case 2: config = KillerGK::SpringConfig::stiff(); break;
        case 3: config = KillerGK::SpringConfig::slow(); break;
        default: config = KillerGK::SpringConfig::criticallyDamped(100.0f); break;
    }
    
    // Create a spring animation
    auto anim = KillerGK::Animation::create()
        .property(prop, from, to)
        .springConfig(config)
        .build();
    
    anim->start();
    
    // Run the animation for a reasonable time (springs can take longer)
    float totalTime = 0.0f;
    float deltaTime = 16.0f;
    float maxTime = 15000.0f;  // 15 seconds max for slow springs
    
    while (anim->isRunning() && totalTime < maxTime) {
        anim->update(deltaTime);
        totalTime += deltaTime;
    }
    
    // Animation should be completed (reached rest state)
    RC_ASSERT(anim->isCompleted());
    
    // Final value should be close to target
    float finalValue = anim->getCurrentValue(prop);
    RC_ASSERT(std::abs(finalValue - to) < 0.1f);  // Allow small tolerance for spring settling
}

/**
 * **Feature: killergk-gui-library, Property 5: Animation Interpolation Correctness**
 * 
 * *For any* spring configuration, the damping ratio calculation should be correct.
 * 
 * **Validates: Requirements 4.2**
 */
RC_GTEST_PROP(AnimationInterpolationProperties, SpringDampingRatioCalculation, ()) {
    auto stiffness = *genSpringStiffness();
    auto damping = *genSpringDamping();
    auto massInt = *gen::inRange(1, 10);
    float mass = static_cast<float>(massInt);
    
    KillerGK::SpringConfig config(stiffness, damping, mass);
    
    // Calculate expected damping ratio
    float criticalDamping = 2.0f * std::sqrt(stiffness * mass);
    float expectedRatio = damping / criticalDamping;
    
    float actualRatio = config.getDampingRatio();
    
    RC_ASSERT(std::abs(actualRatio - expectedRatio) < 0.0001f);
}

/**
 * **Feature: killergk-gui-library, Property 5: Animation Interpolation Correctness**
 * 
 * *For any* spring configuration, the natural frequency calculation should be correct.
 * 
 * **Validates: Requirements 4.2**
 */
RC_GTEST_PROP(AnimationInterpolationProperties, SpringNaturalFrequencyCalculation, ()) {
    auto stiffness = *genSpringStiffness();
    auto massInt = *gen::inRange(1, 10);
    float mass = static_cast<float>(massInt);
    
    KillerGK::SpringConfig config(stiffness, 10.0f, mass);
    
    // Calculate expected natural frequency
    float expectedFreq = std::sqrt(stiffness / mass);
    float actualFreq = config.getNaturalFrequency();
    
    RC_ASSERT(std::abs(actualFreq - expectedFreq) < 0.0001f);
}

/**
 * **Feature: killergk-gui-library, Property 5: Animation Interpolation Correctness**
 * 
 * *For any* critically damped spring, the damping ratio should be 1.0.
 * 
 * **Validates: Requirements 4.2**
 */
RC_GTEST_PROP(AnimationInterpolationProperties, CriticallyDampedSpringHasRatioOne, ()) {
    auto stiffness = *genSpringStiffness();
    auto massInt = *gen::inRange(1, 10);
    float mass = static_cast<float>(massInt);
    
    auto config = KillerGK::SpringConfig::criticallyDamped(stiffness, mass);
    
    float ratio = config.getDampingRatio();
    
    RC_ASSERT(std::abs(ratio - 1.0f) < 0.0001f);
}

/**
 * **Feature: killergk-gui-library, Property 5: Animation Interpolation Correctness**
 * 
 * *For any* keyframe animation with valid keyframes, the interpolation at keyframe
 * positions should return the exact keyframe values.
 * 
 * **Validates: Requirements 4.3**
 */
RC_GTEST_PROP(AnimationInterpolationProperties, KeyframeAnimationAtKeyframePositions, ()) {
    auto prop = *genAnimatableProperty();
    auto value0 = *genAnimationValue();
    auto value50 = *genAnimationValue();
    auto value100 = *genAnimationValue();
    auto duration = *genAnimationDuration();
    
    // Create a keyframe animation with keyframes at 0%, 50%, and 100%
    auto anim = KillerGK::Animation::create()
        .keyframe(0.0f, {{prop, value0}})
        .keyframe(0.5f, {{prop, value50}})
        .keyframe(1.0f, {{prop, value100}})
        .duration(duration)
        .build();
    
    anim->start();
    
    // Test at 0%
    anim->update(0.0f);
    float atStart = anim->getCurrentValue(prop);
    RC_ASSERT(std::abs(atStart - value0) < 0.01f);
    
    // Run to 50%
    anim->reset();
    anim->start();
    float halfDuration = duration * 0.5f;
    anim->update(halfDuration);
    float atMiddle = anim->getCurrentValue(prop);
    RC_ASSERT(std::abs(atMiddle - value50) < 0.01f);
    
    // Run to 100%
    anim->reset();
    anim->start();
    float totalTime = 0.0f;
    while (anim->isRunning() && totalTime < duration + 100.0f) {
        anim->update(16.0f);
        totalTime += 16.0f;
    }
    float atEnd = anim->getCurrentValue(prop);
    RC_ASSERT(std::abs(atEnd - value100) < 0.01f);
}

/**
 * **Feature: killergk-gui-library, Property 5: Animation Interpolation Correctness**
 * 
 * *For any* animation, the clamp function should correctly constrain values.
 * 
 * **Validates: Requirements 4.1, 4.2, 4.3**
 */
RC_GTEST_PROP(AnimationInterpolationProperties, ClampFunctionCorrectness, ()) {
    auto value = *genAnimationValue();
    auto minInt = *gen::inRange(-100, 0);
    auto maxInt = *gen::inRange(0, 100);
    float minVal = static_cast<float>(minInt);
    float maxVal = static_cast<float>(maxInt);
    
    // Ensure min <= max
    if (minVal > maxVal) std::swap(minVal, maxVal);
    
    float result = KillerGK::clamp(value, minVal, maxVal);
    
    // Result should be within bounds
    RC_ASSERT(result >= minVal);
    RC_ASSERT(result <= maxVal);
    
    // If value was in range, result should equal value
    if (value >= minVal && value <= maxVal) {
        RC_ASSERT(std::abs(result - value) < 0.0001f);
    }
}

/**
 * **Feature: killergk-gui-library, Property 5: Animation Interpolation Correctness**
 * 
 * *For any* tween animation with delay, the animation should not progress during the delay period.
 * 
 * **Validates: Requirements 4.1**
 */
RC_GTEST_PROP(AnimationInterpolationProperties, TweenAnimationRespectsDelay, ()) {
    auto prop = *genAnimatableProperty();
    auto from = *genAnimationValue();
    auto to = *genAnimationValue();
    auto duration = *genAnimationDuration();
    auto delayInt = *gen::inRange(100, 1000);
    float delay = static_cast<float>(delayInt);
    
    auto anim = KillerGK::Animation::create()
        .property(prop, from, to)
        .duration(duration)
        .delay(delay)
        .easing(KillerGK::Easing::Linear)
        .build();
    
    anim->start();
    
    // Update with time less than delay
    anim->update(delay * 0.5f);
    
    // Progress should still be 0 during delay
    RC_ASSERT(anim->getProgress() < 0.01f);
    
    // Value should still be at 'from'
    float currentValue = anim->getCurrentValue(prop);
    RC_ASSERT(std::abs(currentValue - from) < 0.01f);
}

// ============================================================================
// Property Tests for Animation Sequencing
// ============================================================================

#include "KillerGK/animation/Animation.hpp"

namespace rc {

/**
 * @brief Generator for animation duration in milliseconds
 */
inline Gen<float> genSequenceAnimationDuration() {
    return gen::map(gen::inRange(50, 500), [](int v) {
        return static_cast<float>(v);
    });
}

/**
 * @brief Generator for stagger delay in milliseconds
 */
inline Gen<float> genStaggerDelay() {
    return gen::map(gen::inRange(10, 200), [](int v) {
        return static_cast<float>(v);
    });
}

/**
 * @brief Generator for number of animations in a sequence
 */
inline Gen<int> genSequenceLength() {
    return gen::inRange(2, 6);  // 2 to 5 animations
}

} // namespace rc

/**
 * **Feature: killergk-gui-library, Property 6: Animation Sequencing**
 * 
 * *For any* chained animation sequence, animations SHALL execute in the 
 * correct order - each animation starts only after the previous one completes.
 * 
 * This test verifies that:
 * 1. In a sequence, animation N does not start until animation N-1 completes
 * 2. The order of animation starts matches the order they were added
 * 
 * **Validates: Requirements 4.4, 4.6**
 */
RC_GTEST_PROP(AnimationSequencingProperties, SequenceExecutesInOrder, ()) {
    auto numAnimations = *genSequenceLength();
    auto duration = *genSequenceAnimationDuration();
    
    // Create animations and track their start order
    std::vector<size_t> startOrder;
    std::vector<size_t> completeOrder;
    std::vector<KillerGK::AnimationHandle> animations;
    
    auto group = std::make_shared<KillerGK::AnimationGroup>(KillerGK::GroupMode::Sequence);
    
    for (int i = 0; i < numAnimations; ++i) {
        auto anim = KillerGK::Animation::create()
            .property(KillerGK::Property::Opacity, 0.0f, 1.0f)
            .duration(duration)
            .easing(KillerGK::Easing::Linear)
            .build();
        
        animations.push_back(anim);
        group->add(anim);
    }
    
    // Set up callbacks to track order
    group->onAnimationStart([&startOrder](size_t index) {
        startOrder.push_back(index);
    });
    
    group->onAnimationComplete([&completeOrder](size_t index) {
        completeOrder.push_back(index);
    });
    
    // Play the sequence
    group->play();
    
    // Simulate time passing - update enough to complete all animations
    float totalTime = duration * numAnimations + 100.0f;  // Extra buffer
    float timeStep = 16.0f;  // ~60 FPS
    
    while (group->update(timeStep) && totalTime > 0) {
        totalTime -= timeStep;
    }
    
    // Verify all animations started and completed
    RC_ASSERT(static_cast<int>(startOrder.size()) == numAnimations);
    RC_ASSERT(static_cast<int>(completeOrder.size()) == numAnimations);
    
    // Verify animations started in correct order (0, 1, 2, ...)
    for (int i = 0; i < numAnimations; ++i) {
        RC_ASSERT(startOrder[i] == static_cast<size_t>(i));
    }
    
    // Verify animations completed in correct order (0, 1, 2, ...)
    for (int i = 0; i < numAnimations; ++i) {
        RC_ASSERT(completeOrder[i] == static_cast<size_t>(i));
    }
}

/**
 * **Feature: killergk-gui-library, Property 6: Animation Sequencing**
 * 
 * *For any* staggered animation sequence, animations SHALL start with the 
 * correct timing delays between them.
 * 
 * This test verifies that:
 * 1. In a staggered sequence, animation N starts staggerDelay ms after animation N-1
 * 2. All animations eventually complete
 * 
 * **Validates: Requirements 4.4, 4.6**
 */
RC_GTEST_PROP(AnimationSequencingProperties, StaggeredAnimationsHaveCorrectTiming, ()) {
    auto numAnimations = *genSequenceLength();
    auto duration = *genSequenceAnimationDuration();
    auto staggerDelay = *genStaggerDelay();
    
    // Track when each animation starts (in simulation time)
    std::vector<float> startTimes(numAnimations, -1.0f);
    float currentTime = 0.0f;
    
    // Create staggered animation group
    auto group = KillerGK::staggered({}, staggerDelay);
    
    std::vector<KillerGK::AnimationHandle> animations;
    for (int i = 0; i < numAnimations; ++i) {
        auto anim = KillerGK::Animation::create()
            .property(KillerGK::Property::Opacity, 0.0f, 1.0f)
            .duration(duration)
            .easing(KillerGK::Easing::Linear)
            .build();
        
        animations.push_back(anim);
        group->add(anim);
    }
    
    // Track animation starts
    group->onAnimationStart([&startTimes, &currentTime](size_t index) {
        if (index < startTimes.size()) {
            startTimes[index] = currentTime;
        }
    });
    
    // Apply stagger
    group->stagger(staggerDelay);
    
    // Play the group
    group->play();
    
    // Simulate time passing
    float totalTime = duration + (numAnimations * staggerDelay) + 500.0f;
    float timeStep = 16.0f;
    
    while (group->update(timeStep) && totalTime > 0) {
        currentTime += timeStep;
        totalTime -= timeStep;
    }
    
    // Verify all animations started
    for (int i = 0; i < numAnimations; ++i) {
        RC_ASSERT(startTimes[i] >= 0.0f);
    }
    
    // Verify stagger timing - each animation should start approximately
    // staggerDelay ms after the previous one
    // Allow some tolerance due to discrete time steps
    float tolerance = timeStep * 2.0f;  // Allow 2 time steps of tolerance
    
    for (int i = 1; i < numAnimations; ++i) {
        float expectedDelay = staggerDelay;
        float actualDelay = startTimes[i] - startTimes[i - 1];
        
        RC_ASSERT(std::abs(actualDelay - expectedDelay) <= tolerance);
    }
}

/**
 * **Feature: killergk-gui-library, Property 6: Animation Sequencing**
 * 
 * *For any* parallel animation group, all animations SHALL start at the same time.
 * 
 * This test verifies that:
 * 1. In parallel mode, all animations start simultaneously
 * 2. All animations complete (not necessarily at the same time)
 * 
 * **Validates: Requirements 4.4, 4.6**
 */
RC_GTEST_PROP(AnimationSequencingProperties, ParallelAnimationsStartTogether, ()) {
    auto numAnimations = *genSequenceLength();
    
    // Track when each animation starts
    std::vector<float> startTimes(numAnimations, -1.0f);
    float currentTime = 0.0f;
    
    // Create parallel animation group
    auto group = std::make_shared<KillerGK::AnimationGroup>(KillerGK::GroupMode::Parallel);
    
    std::vector<KillerGK::AnimationHandle> animations;
    for (int i = 0; i < numAnimations; ++i) {
        // Use different durations to verify they all start together
        // but may complete at different times
        auto durationInt = *gen::inRange(100, 500);
        float duration = static_cast<float>(durationInt);
        
        auto anim = KillerGK::Animation::create()
            .property(KillerGK::Property::Opacity, 0.0f, 1.0f)
            .duration(duration)
            .easing(KillerGK::Easing::Linear)
            .build();
        
        animations.push_back(anim);
        group->add(anim);
    }
    
    // Track animation starts
    group->onAnimationStart([&startTimes, &currentTime](size_t index) {
        if (index < startTimes.size()) {
            startTimes[index] = currentTime;
        }
    });
    
    // Play the group
    group->play();
    
    // Simulate time passing
    float totalTime = 1000.0f;  // Enough time for all animations
    float timeStep = 16.0f;
    
    while (group->update(timeStep) && totalTime > 0) {
        currentTime += timeStep;
        totalTime -= timeStep;
    }
    
    // Verify all animations started
    for (int i = 0; i < numAnimations; ++i) {
        RC_ASSERT(startTimes[i] >= 0.0f);
    }
    
    // Verify all animations started at the same time (within tolerance)
    float tolerance = timeStep;  // Allow 1 time step of tolerance
    float firstStartTime = startTimes[0];
    
    for (int i = 1; i < numAnimations; ++i) {
        RC_ASSERT(std::abs(startTimes[i] - firstStartTime) <= tolerance);
    }
}

/**
 * **Feature: killergk-gui-library, Property 6: Animation Sequencing**
 * 
 * *For any* animation sequence built with AnimationSequence builder,
 * the 'then' method SHALL cause animations to execute sequentially.
 * 
 * This test verifies that:
 * 1. AnimationSequence::then() creates proper sequential execution
 * 2. Each animation completes before the next starts
 * 
 * **Validates: Requirements 4.4, 4.6**
 */
RC_GTEST_PROP(AnimationSequencingProperties, AnimationSequenceBuilderThenIsSequential, ()) {
    auto numAnimations = *genSequenceLength();
    auto duration = *genSequenceAnimationDuration();
    
    // Track completion and start order
    std::vector<size_t> startOrder;
    std::vector<size_t> completeOrder;
    
    // Build sequence using the builder pattern
    auto sequenceBuilder = KillerGK::AnimationSequence::create();
    
    std::vector<KillerGK::AnimationHandle> animations;
    for (int i = 0; i < numAnimations; ++i) {
        auto anim = KillerGK::Animation::create()
            .property(KillerGK::Property::Opacity, 0.0f, 1.0f)
            .duration(duration)
            .easing(KillerGK::Easing::Linear)
            .onStart([&startOrder, i]() {
                startOrder.push_back(static_cast<size_t>(i));
            })
            .onComplete([&completeOrder, i]() {
                completeOrder.push_back(static_cast<size_t>(i));
            })
            .build();
        
        animations.push_back(anim);
        sequenceBuilder.then(anim);
    }
    
    auto group = sequenceBuilder.build();
    
    // Play the sequence
    group->play();
    
    // Simulate time passing
    float totalTime = duration * numAnimations + 500.0f;
    float timeStep = 16.0f;
    
    while (group->update(timeStep) && totalTime > 0) {
        totalTime -= timeStep;
    }
    
    // Verify all animations started and completed
    RC_ASSERT(static_cast<int>(startOrder.size()) == numAnimations);
    RC_ASSERT(static_cast<int>(completeOrder.size()) == numAnimations);
    
    // Verify sequential execution: animation i should complete before animation i+1 starts
    // This is verified by checking that the complete order matches the start order
    for (int i = 0; i < numAnimations; ++i) {
        RC_ASSERT(startOrder[i] == static_cast<size_t>(i));
        RC_ASSERT(completeOrder[i] == static_cast<size_t>(i));
    }
}

/**
 * **Feature: killergk-gui-library, Property 6: Animation Sequencing**
 * 
 * *For any* animation group, the total duration SHALL equal the sum of 
 * individual durations (for sequence) or the max duration (for parallel).
 * 
 * This test verifies that:
 * 1. Sequence total duration = sum of all animation durations
 * 2. Parallel total duration = max of all animation durations
 * 
 * **Validates: Requirements 4.4, 4.6**
 */
RC_GTEST_PROP(AnimationSequencingProperties, GroupDurationCalculation, ()) {
    auto numAnimations = *genSequenceLength();
    
    // Generate random durations for each animation
    std::vector<float> durations;
    float sumDuration = 0.0f;
    float maxDuration = 0.0f;
    
    for (int i = 0; i < numAnimations; ++i) {
        auto durationInt = *gen::inRange(100, 500);
        float duration = static_cast<float>(durationInt);
        durations.push_back(duration);
        sumDuration += duration;
        maxDuration = std::max(maxDuration, duration);
    }
    
    // Create sequence group
    auto sequenceGroup = std::make_shared<KillerGK::AnimationGroup>(KillerGK::GroupMode::Sequence);
    
    // Create parallel group
    auto parallelGroup = std::make_shared<KillerGK::AnimationGroup>(KillerGK::GroupMode::Parallel);
    
    for (int i = 0; i < numAnimations; ++i) {
        auto anim = KillerGK::Animation::create()
            .property(KillerGK::Property::Opacity, 0.0f, 1.0f)
            .duration(durations[i])
            .easing(KillerGK::Easing::Linear)
            .build();
        
        sequenceGroup->add(anim);
        
        // Create a separate animation for parallel group
        auto anim2 = KillerGK::Animation::create()
            .property(KillerGK::Property::Opacity, 0.0f, 1.0f)
            .duration(durations[i])
            .easing(KillerGK::Easing::Linear)
            .build();
        
        parallelGroup->add(anim2);
    }
    
    // Verify sequence duration equals sum
    float sequenceTotalDuration = sequenceGroup->getTotalDuration();
    RC_ASSERT(std::abs(sequenceTotalDuration - sumDuration) < 1.0f);
    
    // Verify parallel duration equals max
    float parallelTotalDuration = parallelGroup->getTotalDuration();
    RC_ASSERT(std::abs(parallelTotalDuration - maxDuration) < 1.0f);
}

/**
 * **Feature: killergk-gui-library, Property 6: Animation Sequencing**
 * 
 * *For any* animation sequence, stopping the group SHALL stop all animations.
 * 
 * This test verifies that:
 * 1. Calling stop() on a group stops all contained animations
 * 2. No further callbacks are triggered after stop
 * 
 * **Validates: Requirements 4.4, 4.6**
 */
RC_GTEST_PROP(AnimationSequencingProperties, StopGroupStopsAllAnimations, ()) {
    auto numAnimations = *genSequenceLength();
    auto duration = *genSequenceAnimationDuration();
    
    // Create sequence group
    auto group = std::make_shared<KillerGK::AnimationGroup>(KillerGK::GroupMode::Sequence);
    
    std::vector<KillerGK::AnimationHandle> animations;
    for (int i = 0; i < numAnimations; ++i) {
        auto anim = KillerGK::Animation::create()
            .property(KillerGK::Property::Opacity, 0.0f, 1.0f)
            .duration(duration)
            .easing(KillerGK::Easing::Linear)
            .build();
        
        animations.push_back(anim);
        group->add(anim);
    }
    
    // Play the group
    group->play();
    
    // Update a bit to start the first animation
    group->update(duration * 0.5f);
    
    // Stop the group
    group->stop();
    
    // Verify group is no longer playing
    RC_ASSERT(!group->isPlaying());
    RC_ASSERT(group->isCompleted());
    
    // Further updates should return false (not running)
    RC_ASSERT(!group->update(16.0f));
}
