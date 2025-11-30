/**
 * @file Animation.hpp
 * @brief Animation system for KillerGK
 */

#pragma once

#include "../widgets/Widget.hpp"
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <chrono>

namespace KillerGK {

/**
 * @enum Easing
 * @brief Easing functions for animations
 */
enum class Easing {
    Linear,
    EaseIn, EaseOut, EaseInOut,
    EaseInQuad, EaseOutQuad, EaseInOutQuad,
    EaseInCubic, EaseOutCubic, EaseInOutCubic,
    EaseInQuart, EaseOutQuart, EaseInOutQuart,
    EaseInQuint, EaseOutQuint, EaseInOutQuint,
    EaseInSine, EaseOutSine, EaseInOutSine,
    EaseInExpo, EaseOutExpo, EaseInOutExpo,
    EaseInCirc, EaseOutCirc, EaseInOutCirc,
    EaseInElastic, EaseOutElastic, EaseInOutElastic,
    EaseInBounce, EaseOutBounce, EaseInOutBounce,
    EaseInBack, EaseOutBack, EaseInOutBack
};

/**
 * @enum AnimationState
 * @brief Current state of an animation
 */
enum class AnimationState {
    Idle,
    Running,
    Paused,
    Completed
};

/**
 * @enum AnimationType
 * @brief Type of animation
 */
enum class AnimationType {
    Tween,
    Spring,
    Keyframe
};

// Forward declarations
class AnimationImpl;
class AnimationTimeline;
class AnimationGroup;

/**
 * @brief Handle to a built animation
 */
using AnimationHandle = std::shared_ptr<AnimationImpl>;

/**
 * @struct Keyframe
 * @brief A single keyframe in a keyframe animation
 */
struct Keyframe {
    float percent;  // 0.0 to 1.0
    std::map<Property, float> values;
    Easing easing = Easing::Linear;

    Keyframe() : percent(0.0f) {}
    Keyframe(float p, std::map<Property, float> v, Easing e = Easing::Linear)
        : percent(p), values(std::move(v)), easing(e) {}
};

/**
 * @struct SpringConfig
 * @brief Configuration for spring-based animations
 */
struct SpringConfig {
    float stiffness = 100.0f;   // Spring stiffness (k)
    float damping = 10.0f;      // Damping coefficient (c)
    float mass = 1.0f;          // Mass (m)
    float velocity = 0.0f;      // Initial velocity
    float restThreshold = 0.001f;  // Threshold to consider at rest

    SpringConfig() = default;
    SpringConfig(float s, float d, float m = 1.0f)
        : stiffness(s), damping(d), mass(m) {}
};

/**
 * @struct PropertyAnimation
 * @brief Animation data for a single property
 */
struct PropertyAnimation {
    Property property;
    float fromValue;
    float toValue;
    float currentValue;

    PropertyAnimation() : property(Property::Opacity), fromValue(0), toValue(0), currentValue(0) {}
    PropertyAnimation(Property p, float from, float to)
        : property(p), fromValue(from), toValue(to), currentValue(from) {}
};

/**
 * @class AnimationImpl
 * @brief Implementation of a single animation
 */
class AnimationImpl : public std::enable_shared_from_this<AnimationImpl> {
public:
    AnimationImpl();
    ~AnimationImpl() = default;

    // Animation control
    void start();
    void pause();
    void resume();
    void stop();
    void reset();

    // Update animation (called each frame)
    // Returns true if animation is still running
    bool update(float deltaTimeMs);

    // State queries
    [[nodiscard]] AnimationState getState() const { return m_state; }
    [[nodiscard]] bool isRunning() const { return m_state == AnimationState::Running; }
    [[nodiscard]] bool isCompleted() const { return m_state == AnimationState::Completed; }
    [[nodiscard]] float getProgress() const { return m_progress; }
    [[nodiscard]] AnimationType getType() const { return m_type; }

    // Get current interpolated value for a property
    [[nodiscard]] float getCurrentValue(Property prop) const;

    // Configuration
    void setType(AnimationType type) { m_type = type; }
    void setDuration(float ms) { m_duration = ms; }
    void setDelay(float ms) { m_delay = ms; }
    void setEasing(Easing e) { m_easing = e; }
    void setLoopCount(int count) { m_loopCount = count; }
    void setYoyo(bool enabled) { m_yoyo = enabled; }
    void setSpringConfig(const SpringConfig& config) { m_springConfig = config; }

    void addProperty(Property prop, float from, float to);
    void addKeyframe(const Keyframe& keyframe);

    void setOnStart(std::function<void()> cb) { m_onStart = std::move(cb); }
    void setOnComplete(std::function<void()> cb) { m_onComplete = std::move(cb); }
    void setOnUpdate(std::function<void(float)> cb) { m_onUpdate = std::move(cb); }

    // Getters
    [[nodiscard]] float getDuration() const { return m_duration; }
    [[nodiscard]] float getDelay() const { return m_delay; }
    [[nodiscard]] Easing getEasing() const { return m_easing; }
    [[nodiscard]] int getLoopCount() const { return m_loopCount; }
    [[nodiscard]] bool getYoyo() const { return m_yoyo; }
    [[nodiscard]] const SpringConfig& getSpringConfig() const { return m_springConfig; }
    [[nodiscard]] const std::vector<PropertyAnimation>& getProperties() const { return m_properties; }
    [[nodiscard]] const std::vector<Keyframe>& getKeyframes() const { return m_keyframes; }

private:
    void updateTween(float progress);
    void updateSpring(float deltaTimeMs);
    void updateKeyframe(float progress);
    float interpolateKeyframes(Property prop, float progress) const;

    AnimationType m_type = AnimationType::Tween;
    AnimationState m_state = AnimationState::Idle;

    float m_duration = 300.0f;
    float m_delay = 0.0f;
    float m_elapsedTime = 0.0f;
    float m_progress = 0.0f;
    Easing m_easing = Easing::Linear;

    int m_loopCount = 1;
    int m_currentLoop = 0;
    bool m_yoyo = false;
    bool m_reversed = false;

    SpringConfig m_springConfig;
    std::vector<PropertyAnimation> m_properties;
    std::vector<Keyframe> m_keyframes;

    // Spring simulation state
    std::map<Property, float> m_springVelocities;

    std::function<void()> m_onStart;
    std::function<void()> m_onComplete;
    std::function<void(float)> m_onUpdate;
};

/**
 * @class Animation
 * @brief Animation builder class
 */
class Animation {
public:
    static Animation create();

    // Property animation
    Animation& property(Property prop, float from, float to);
    Animation& duration(float milliseconds);
    Animation& delay(float milliseconds);
    Animation& easing(Easing easing);
    Animation& loop(int count);  // -1 for infinite
    Animation& yoyo(bool enabled);

    // Spring animation
    Animation& spring(float stiffness, float damping);
    Animation& springMass(float mass);
    Animation& springVelocity(float velocity);

    // Keyframes
    Animation& keyframe(float percent, std::map<Property, float> values);
    Animation& keyframe(float percent, std::map<Property, float> values, Easing easing);

    // Callbacks
    Animation& onStart(std::function<void()> callback);
    Animation& onComplete(std::function<void()> callback);
    Animation& onUpdate(std::function<void(float)> callback);

    AnimationHandle build();

private:
    Animation();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

/**
 * @class AnimationTimeline
 * @brief Manages multiple animations with timing control
 */
class AnimationTimeline {
public:
    AnimationTimeline();
    ~AnimationTimeline() = default;

    // Add animations
    void add(AnimationHandle animation, float startTime = 0.0f);
    void addAt(AnimationHandle animation, float startTime);
    void addAfter(AnimationHandle animation, AnimationHandle after, float delay = 0.0f);

    // Timeline control
    void play();
    void pause();
    void stop();
    void reset();
    void seek(float timeMs);

    // Update (call each frame)
    bool update(float deltaTimeMs);

    // State
    [[nodiscard]] bool isPlaying() const { return m_playing; }
    [[nodiscard]] float getCurrentTime() const { return m_currentTime; }
    [[nodiscard]] float getTotalDuration() const;

    // Callbacks
    void onComplete(std::function<void()> callback) { m_onComplete = std::move(callback); }

private:
    struct TimelineEntry {
        AnimationHandle animation;
        float startTime;
        bool started = false;
    };

    std::vector<TimelineEntry> m_entries;
    float m_currentTime = 0.0f;
    bool m_playing = false;
    std::function<void()> m_onComplete;
};

/**
 * @enum GroupMode
 * @brief How animations in a group are executed
 */
enum class GroupMode {
    Sequence,   // One after another
    Parallel    // All at once
};

/**
 * @class AnimationGroup
 * @brief Groups multiple animations for coordinated playback
 */
class AnimationGroup {
public:
    explicit AnimationGroup(GroupMode mode = GroupMode::Sequence);
    ~AnimationGroup() = default;

    // Add animations
    AnimationGroup& add(AnimationHandle animation);
    AnimationGroup& addWithDelay(AnimationHandle animation, float delay);

    // Stagger animations (for sequence mode)
    AnimationGroup& stagger(float delayBetween);

    // Group control
    void play();
    void pause();
    void stop();
    void reset();

    // Update (call each frame)
    bool update(float deltaTimeMs);

    // State
    [[nodiscard]] bool isPlaying() const { return m_playing; }
    [[nodiscard]] GroupMode getMode() const { return m_mode; }

    // Callbacks
    AnimationGroup& onComplete(std::function<void()> callback);

private:
    struct GroupEntry {
        AnimationHandle animation;
        float delay = 0.0f;
        float elapsedDelay = 0.0f;
        bool started = false;
    };

    GroupMode m_mode;
    std::vector<GroupEntry> m_entries;
    float m_staggerDelay = 0.0f;
    bool m_playing = false;
    size_t m_currentIndex = 0;
    std::function<void()> m_onComplete;
};



/**
 * @class AnimationManager
 * @brief Global animation manager for the application
 */
class AnimationManager {
public:
    static AnimationManager& instance();

    // Register animations for automatic updates
    void registerAnimation(AnimationHandle animation);
    void unregisterAnimation(AnimationHandle animation);

    // Update all registered animations
    void update(float deltaTimeMs);

    // Convenience methods
    AnimationHandle createTween(Property prop, float from, float to, float duration, Easing easing = Easing::Linear);
    AnimationHandle createSpring(Property prop, float from, float to, float stiffness, float damping);

    // Clear all animations
    void clear();

    // Statistics
    [[nodiscard]] size_t getActiveAnimationCount() const;

private:
    AnimationManager() = default;
    std::vector<std::weak_ptr<AnimationImpl>> m_animations;
};

/**
 * @brief Apply easing function to a value
 * @param easing The easing function to use
 * @param t Progress value between 0 and 1
 * @return Eased value
 */
float applyEasing(Easing easing, float t);

/**
 * @brief Linear interpolation between two values
 * @param from Start value
 * @param to End value
 * @param t Progress (0-1)
 * @return Interpolated value
 */
inline float lerp(float from, float to, float t) {
    return from + (to - from) * t;
}

/**
 * @brief Clamp a value between min and max
 * @param value Value to clamp
 * @param min Minimum value
 * @param max Maximum value
 * @return Clamped value
 */
inline float clamp(float value, float min, float max) {
    return value < min ? min : (value > max ? max : value);
}

// ============================================================================
// Property Value Helpers
// ============================================================================

/**
 * @brief Get a property value from a widget
 * @param widget The widget to read from
 * @param prop The property to get
 * @return The current property value
 */
float getWidgetPropertyValue(const Widget& widget, Property prop);

/**
 * @brief Set a property value on a widget
 * @param widget The widget to modify
 * @param prop The property to set
 * @param value The value to set
 */
void setWidgetPropertyValue(Widget& widget, Property prop, float value);

/**
 * @brief Check if a property is animatable
 * @param prop The property to check
 * @return true if the property can be animated
 */
bool isAnimatableProperty(Property prop);

/**
 * @brief Get the name of a property as a string
 * @param prop The property
 * @return String name of the property
 */
const char* getPropertyName(Property prop);

// ============================================================================
// TweenAnimator - Animates widgets directly
// ============================================================================

/**
 * @class TweenAnimator
 * @brief Animates widget properties using tween interpolation
 * 
 * TweenAnimator provides a convenient way to animate widget properties
 * directly, handling the interpolation and application of values.
 * 
 * Example:
 * @code
 * auto animator = TweenAnimator::create(myWidget)
 *     .property(Property::Opacity, 0.0f, 1.0f)
 *     .property(Property::Scale, 0.5f, 1.0f)
 *     .duration(500.0f)
 *     .easing(Easing::EaseOutCubic)
 *     .build();
 * animator->start();
 * @endcode
 */
class TweenAnimator {
public:
    /**
     * @brief Create a new TweenAnimator for a widget
     * @param widget The widget to animate
     * @return TweenAnimator builder
     */
    static TweenAnimator create(Widget* widget);

    /**
     * @brief Add a property to animate
     * @param prop The property to animate
     * @param from Starting value
     * @param to Ending value
     * @return Reference to this for chaining
     */
    TweenAnimator& property(Property prop, float from, float to);

    /**
     * @brief Add a property to animate from current value
     * @param prop The property to animate
     * @param to Ending value
     * @return Reference to this for chaining
     */
    TweenAnimator& propertyTo(Property prop, float to);

    /**
     * @brief Set animation duration
     * @param milliseconds Duration in milliseconds
     * @return Reference to this for chaining
     */
    TweenAnimator& duration(float milliseconds);

    /**
     * @brief Set animation delay
     * @param milliseconds Delay before animation starts
     * @return Reference to this for chaining
     */
    TweenAnimator& delay(float milliseconds);

    /**
     * @brief Set easing function
     * @param easing The easing function to use
     * @return Reference to this for chaining
     */
    TweenAnimator& easing(Easing easing);

    /**
     * @brief Set loop count
     * @param count Number of loops (-1 for infinite)
     * @return Reference to this for chaining
     */
    TweenAnimator& loop(int count);

    /**
     * @brief Enable yoyo mode (reverse on each loop)
     * @param enabled Whether to enable yoyo
     * @return Reference to this for chaining
     */
    TweenAnimator& yoyo(bool enabled);

    /**
     * @brief Set callback for animation start
     * @param callback Function to call when animation starts
     * @return Reference to this for chaining
     */
    TweenAnimator& onStart(std::function<void()> callback);

    /**
     * @brief Set callback for animation completion
     * @param callback Function to call when animation completes
     * @return Reference to this for chaining
     */
    TweenAnimator& onComplete(std::function<void()> callback);

    /**
     * @brief Set callback for animation updates
     * @param callback Function to call on each update with progress (0-1)
     * @return Reference to this for chaining
     */
    TweenAnimator& onUpdate(std::function<void(float)> callback);

    /**
     * @brief Build the animator
     * @return Shared pointer to the built animator implementation
     */
    std::shared_ptr<class TweenAnimatorImpl> build();

private:
    TweenAnimator(Widget* widget);
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

/**
 * @class TweenAnimatorImpl
 * @brief Implementation of TweenAnimator that manages animation state
 */
class TweenAnimatorImpl : public std::enable_shared_from_this<TweenAnimatorImpl> {
public:
    TweenAnimatorImpl(Widget* widget);
    ~TweenAnimatorImpl() = default;

    /**
     * @brief Start the animation
     */
    void start();

    /**
     * @brief Pause the animation
     */
    void pause();

    /**
     * @brief Resume a paused animation
     */
    void resume();

    /**
     * @brief Stop the animation
     */
    void stop();

    /**
     * @brief Reset the animation to initial state
     */
    void reset();

    /**
     * @brief Update the animation
     * @param deltaTimeMs Time elapsed since last update in milliseconds
     * @return true if animation is still running
     */
    bool update(float deltaTimeMs);

    /**
     * @brief Check if animation is running
     * @return true if running
     */
    [[nodiscard]] bool isRunning() const;

    /**
     * @brief Check if animation is completed
     * @return true if completed
     */
    [[nodiscard]] bool isCompleted() const;

    /**
     * @brief Get current progress (0-1)
     * @return Current progress
     */
    [[nodiscard]] float getProgress() const;

    /**
     * @brief Get the target widget
     * @return Pointer to the widget being animated
     */
    [[nodiscard]] Widget* getWidget() const { return m_widget; }

    // Configuration (called by TweenAnimator builder)
    void addProperty(Property prop, float from, float to);
    void setDuration(float ms) { m_duration = ms; }
    void setDelay(float ms) { m_delay = ms; }
    void setEasing(Easing e) { m_easing = e; }
    void setLoopCount(int count) { m_loopCount = count; }
    void setYoyo(bool enabled) { m_yoyo = enabled; }
    void setOnStart(std::function<void()> cb) { m_onStart = std::move(cb); }
    void setOnComplete(std::function<void()> cb) { m_onComplete = std::move(cb); }
    void setOnUpdate(std::function<void(float)> cb) { m_onUpdate = std::move(cb); }

private:
    void applyCurrentValues();

    Widget* m_widget;
    AnimationState m_state = AnimationState::Idle;

    float m_duration = 300.0f;
    float m_delay = 0.0f;
    float m_elapsedTime = 0.0f;
    float m_progress = 0.0f;
    Easing m_easing = Easing::Linear;

    int m_loopCount = 1;
    int m_currentLoop = 0;
    bool m_yoyo = false;
    bool m_reversed = false;

    std::vector<PropertyAnimation> m_properties;

    std::function<void()> m_onStart;
    std::function<void()> m_onComplete;
    std::function<void(float)> m_onUpdate;
};

// ============================================================================
// State Transition System
// ============================================================================

/**
 * @enum WidgetStateType
 * @brief Widget interaction states for automatic transitions
 */
enum class WidgetStateType {
    Normal,
    Hovered,
    Pressed,
    Focused,
    Disabled
};

/**
 * @struct StateTransitionConfig
 * @brief Configuration for automatic state transitions
 */
struct StateTransitionConfig {
    float duration = 150.0f;  // milliseconds
    Easing easing = Easing::EaseOut;
    std::map<Property, float> propertyDeltas;  // Changes from normal state

    StateTransitionConfig() = default;
    StateTransitionConfig(float d, Easing e) : duration(d), easing(e) {}
};

/**
 * @class StateTransitionManager
 * @brief Manages automatic state transitions for widgets
 */
class StateTransitionManager {
public:
    StateTransitionManager();
    ~StateTransitionManager() = default;

    // Configure transitions for a widget state
    void setTransition(WidgetStateType state, const StateTransitionConfig& config);
    [[nodiscard]] const StateTransitionConfig& getTransition(WidgetStateType state) const;

    // Trigger state change
    void transitionTo(WidgetStateType newState, Widget* widget);

    // Get current state
    [[nodiscard]] WidgetStateType getCurrentState() const { return m_currentState; }

    // Update active transitions
    bool update(float deltaTimeMs);

    // Apply theme defaults
    void applyThemeDefaults(const class Theme& theme);

private:
    WidgetStateType m_currentState = WidgetStateType::Normal;
    WidgetStateType m_targetState = WidgetStateType::Normal;
    std::map<WidgetStateType, StateTransitionConfig> m_transitions;
    AnimationHandle m_activeTransition;
    Widget* m_targetWidget = nullptr;
};

} // namespace KillerGK
