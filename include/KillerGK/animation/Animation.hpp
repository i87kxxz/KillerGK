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
#include <cmath>

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
 * 
 * Spring physics simulation using the damped harmonic oscillator equation:
 * F = -kx - cv
 * Where:
 *   k = stiffness (spring constant)
 *   x = displacement from target
 *   c = damping coefficient
 *   v = velocity
 * 
 * The damping ratio (zeta) determines the behavior:
 *   zeta < 1: Underdamped (oscillates)
 *   zeta = 1: Critically damped (fastest without oscillation)
 *   zeta > 1: Overdamped (slow approach without oscillation)
 */
struct SpringConfig {
    float stiffness = 100.0f;      // Spring stiffness (k) - higher = faster oscillation
    float damping = 10.0f;         // Damping coefficient (c) - higher = less oscillation
    float mass = 1.0f;             // Mass (m) - higher = slower, more momentum
    float velocity = 0.0f;         // Initial velocity
    float restThreshold = 0.001f;  // Threshold to consider at rest
    float velocityThreshold = 0.001f;  // Velocity threshold to consider at rest

    SpringConfig() = default;
    SpringConfig(float s, float d, float m = 1.0f)
        : stiffness(s), damping(d), mass(m) {}

    /**
     * @brief Calculate the damping ratio (zeta)
     * @return Damping ratio: < 1 underdamped, = 1 critical, > 1 overdamped
     */
    [[nodiscard]] float getDampingRatio() const {
        float criticalDamping = 2.0f * std::sqrt(stiffness * mass);
        return damping / criticalDamping;
    }

    /**
     * @brief Calculate the natural frequency (omega_n)
     * @return Natural angular frequency in rad/s
     */
    [[nodiscard]] float getNaturalFrequency() const {
        return std::sqrt(stiffness / mass);
    }

    /**
     * @brief Create a critically damped spring (fastest without oscillation)
     * @param stiffness Spring stiffness
     * @param mass Spring mass (default 1.0)
     * @return SpringConfig with critical damping
     */
    static SpringConfig criticallyDamped(float stiffness, float mass = 1.0f) {
        float criticalDamping = 2.0f * std::sqrt(stiffness * mass);
        return SpringConfig(stiffness, criticalDamping, mass);
    }

    /**
     * @brief Create an underdamped spring (bouncy)
     * @param stiffness Spring stiffness
     * @param dampingRatio Ratio < 1.0 (e.g., 0.5 for bouncy)
     * @param mass Spring mass (default 1.0)
     * @return SpringConfig with underdamping
     */
    static SpringConfig underdamped(float stiffness, float dampingRatio, float mass = 1.0f) {
        float criticalDamping = 2.0f * std::sqrt(stiffness * mass);
        return SpringConfig(stiffness, criticalDamping * dampingRatio, mass);
    }

    /**
     * @brief Create an overdamped spring (slow, no oscillation)
     * @param stiffness Spring stiffness
     * @param dampingRatio Ratio > 1.0 (e.g., 1.5 for slow)
     * @param mass Spring mass (default 1.0)
     * @return SpringConfig with overdamping
     */
    static SpringConfig overdamped(float stiffness, float dampingRatio, float mass = 1.0f) {
        float criticalDamping = 2.0f * std::sqrt(stiffness * mass);
        return SpringConfig(stiffness, criticalDamping * dampingRatio, mass);
    }

    // Preset configurations
    
    /**
     * @brief Gentle spring - slow, smooth motion
     */
    static SpringConfig gentle() {
        return SpringConfig(50.0f, 14.0f, 1.0f);
    }

    /**
     * @brief Wobbly spring - bouncy, playful motion
     */
    static SpringConfig wobbly() {
        return SpringConfig(180.0f, 12.0f, 1.0f);
    }

    /**
     * @brief Stiff spring - quick, snappy motion
     */
    static SpringConfig stiff() {
        return SpringConfig(400.0f, 28.0f, 1.0f);
    }

    /**
     * @brief Slow spring - very slow, heavy motion
     */
    static SpringConfig slow() {
        return SpringConfig(50.0f, 20.0f, 1.0f);
    }

    /**
     * @brief Molasses spring - extremely slow, viscous motion
     */
    static SpringConfig molasses() {
        return SpringConfig(30.0f, 30.0f, 1.0f);
    }
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
    Animation& springConfig(const SpringConfig& config);
    
    // Spring presets
    Animation& springGentle();    // Slow, smooth motion
    Animation& springWobbly();    // Bouncy, playful motion
    Animation& springStiff();     // Quick, snappy motion
    Animation& springSlow();      // Very slow, heavy motion

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
 * 
 * AnimationGroup allows you to combine multiple animations and control
 * how they execute - either in sequence (one after another) or in parallel
 * (all at once). Stagger delays can be applied to create wave-like effects.
 * 
 * Example - Sequence:
 * @code
 * auto group = AnimationGroup(GroupMode::Sequence);
 * group.add(anim1).add(anim2).add(anim3);
 * group.play();  // anim1, then anim2, then anim3
 * @endcode
 * 
 * Example - Parallel with stagger:
 * @code
 * auto group = AnimationGroup(GroupMode::Parallel);
 * group.add(anim1).add(anim2).add(anim3).stagger(100.0f);
 * group.play();  // All start, but with 100ms delay between each
 * @endcode
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
    [[nodiscard]] bool isCompleted() const { return m_completed; }
    [[nodiscard]] GroupMode getMode() const { return m_mode; }
    [[nodiscard]] size_t getAnimationCount() const { return m_entries.size(); }
    [[nodiscard]] size_t getCurrentIndex() const { return m_currentIndex; }
    [[nodiscard]] float getTotalDuration() const;

    // Callbacks
    AnimationGroup& onComplete(std::function<void()> callback);
    AnimationGroup& onAnimationStart(std::function<void(size_t index)> callback);
    AnimationGroup& onAnimationComplete(std::function<void(size_t index)> callback);

private:
    struct GroupEntry {
        AnimationHandle animation;
        float delay = 0.0f;
        float elapsedDelay = 0.0f;
        bool started = false;
        bool completed = false;
    };

    GroupMode m_mode;
    std::vector<GroupEntry> m_entries;
    float m_staggerDelay = 0.0f;
    bool m_playing = false;
    bool m_completed = false;
    size_t m_currentIndex = 0;
    std::function<void()> m_onComplete;
    std::function<void(size_t)> m_onAnimationStart;
    std::function<void(size_t)> m_onAnimationComplete;
};

/**
 * @class AnimationSequence
 * @brief Builder for creating animation sequences with fluent API
 * 
 * AnimationSequence provides a convenient way to chain animations together
 * using a fluent builder pattern. It supports both sequential and parallel
 * execution modes.
 * 
 * Example:
 * @code
 * auto sequence = AnimationSequence::create()
 *     .then(fadeInAnim)
 *     .then(slideAnim)
 *     .with(scaleAnim)  // runs parallel with slideAnim
 *     .then(bounceAnim)
 *     .stagger(50.0f)
 *     .build();
 * sequence->play();
 * @endcode
 */
class AnimationSequence {
public:
    /**
     * @brief Create a new animation sequence builder
     * @return AnimationSequence builder
     */
    static AnimationSequence create();

    /**
     * @brief Add an animation to run after the previous one completes
     * @param animation The animation to add
     * @return Reference to this for chaining
     */
    AnimationSequence& then(AnimationHandle animation);

    /**
     * @brief Add an animation to run after a delay from the previous one
     * @param animation The animation to add
     * @param delayMs Delay in milliseconds after previous animation starts
     * @return Reference to this for chaining
     */
    AnimationSequence& thenAfter(AnimationHandle animation, float delayMs);

    /**
     * @brief Add an animation to run in parallel with the previous one
     * @param animation The animation to add
     * @return Reference to this for chaining
     */
    AnimationSequence& with(AnimationHandle animation);

    /**
     * @brief Add an animation to run in parallel with a delay
     * @param animation The animation to add
     * @param delayMs Delay in milliseconds
     * @return Reference to this for chaining
     */
    AnimationSequence& withDelay(AnimationHandle animation, float delayMs);

    /**
     * @brief Set stagger delay between animations
     * @param delayMs Delay in milliseconds between each animation start
     * @return Reference to this for chaining
     */
    AnimationSequence& stagger(float delayMs);

    /**
     * @brief Set callback for when sequence completes
     * @param callback Function to call on completion
     * @return Reference to this for chaining
     */
    AnimationSequence& onComplete(std::function<void()> callback);

    /**
     * @brief Build the animation sequence
     * @return Shared pointer to the built AnimationGroup
     */
    std::shared_ptr<AnimationGroup> build();

private:
    AnimationSequence();
    
    struct SequenceEntry {
        AnimationHandle animation;
        float delay = 0.0f;
        bool parallel = false;  // true if should run with previous
    };
    
    std::vector<SequenceEntry> m_entries;
    float m_staggerDelay = 0.0f;
    std::function<void()> m_onComplete;
};

/**
 * @brief Create a sequence of animations that run one after another
 * @param animations List of animations to sequence
 * @return AnimationGroup configured for sequential playback
 */
std::shared_ptr<AnimationGroup> sequence(std::initializer_list<AnimationHandle> animations);

/**
 * @brief Create a group of animations that run in parallel
 * @param animations List of animations to run together
 * @return AnimationGroup configured for parallel playback
 */
std::shared_ptr<AnimationGroup> parallel(std::initializer_list<AnimationHandle> animations);

/**
 * @brief Create a staggered animation sequence
 * @param animations List of animations to stagger
 * @param delayBetween Delay in milliseconds between each animation start
 * @return AnimationGroup configured with stagger delay
 */
std::shared_ptr<AnimationGroup> staggered(std::initializer_list<AnimationHandle> animations, float delayBetween);



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
class TweenAnimatorImpl;

/**
 * @class SpringAnimator
 * @brief Animates widget properties using spring physics
 * 
 * SpringAnimator provides physics-based animation with natural-feeling
 * motion that responds to stiffness and damping parameters.
 * 
 * Example:
 * @code
 * auto animator = SpringAnimator::create(myWidget)
 *     .property(Property::X, 0.0f, 100.0f)
 *     .stiffness(200.0f)
 *     .damping(15.0f)
 *     .build();
 * animator->start();
 * @endcode
 */
class SpringAnimator {
public:
    /**
     * @brief Create a new SpringAnimator for a widget
     * @param widget The widget to animate
     * @return SpringAnimator builder
     */
    static SpringAnimator create(Widget* widget);

    /**
     * @brief Add a property to animate
     * @param prop The property to animate
     * @param from Starting value
     * @param to Ending value
     * @return Reference to this for chaining
     */
    SpringAnimator& property(Property prop, float from, float to);

    /**
     * @brief Add a property to animate from current value
     * @param prop The property to animate
     * @param to Ending value
     * @return Reference to this for chaining
     */
    SpringAnimator& propertyTo(Property prop, float to);

    /**
     * @brief Set spring stiffness
     * @param value Stiffness value (higher = faster oscillation)
     * @return Reference to this for chaining
     */
    SpringAnimator& stiffness(float value);

    /**
     * @brief Set spring damping
     * @param value Damping value (higher = less oscillation)
     * @return Reference to this for chaining
     */
    SpringAnimator& damping(float value);

    /**
     * @brief Set spring mass
     * @param value Mass value (higher = slower, more momentum)
     * @return Reference to this for chaining
     */
    SpringAnimator& mass(float value);

    /**
     * @brief Set initial velocity
     * @param value Initial velocity
     * @return Reference to this for chaining
     */
    SpringAnimator& velocity(float value);

    /**
     * @brief Use a preset spring configuration
     * @param config The spring configuration to use
     * @return Reference to this for chaining
     */
    SpringAnimator& config(const SpringConfig& config);

    /**
     * @brief Use gentle preset (slow, smooth)
     * @return Reference to this for chaining
     */
    SpringAnimator& gentle();

    /**
     * @brief Use wobbly preset (bouncy, playful)
     * @return Reference to this for chaining
     */
    SpringAnimator& wobbly();

    /**
     * @brief Use stiff preset (quick, snappy)
     * @return Reference to this for chaining
     */
    SpringAnimator& stiff();

    /**
     * @brief Use slow preset (very slow, heavy)
     * @return Reference to this for chaining
     */
    SpringAnimator& slow();

    /**
     * @brief Set callback for animation start
     * @param callback Function to call when animation starts
     * @return Reference to this for chaining
     */
    SpringAnimator& onStart(std::function<void()> callback);

    /**
     * @brief Set callback for animation completion
     * @param callback Function to call when animation completes
     * @return Reference to this for chaining
     */
    SpringAnimator& onComplete(std::function<void()> callback);

    /**
     * @brief Set callback for animation updates
     * @param callback Function to call on each update with progress (0-1)
     * @return Reference to this for chaining
     */
    SpringAnimator& onUpdate(std::function<void(float)> callback);

    /**
     * @brief Build the animator
     * @return Shared pointer to the built animator implementation
     */
    std::shared_ptr<class SpringAnimatorImpl> build();

private:
    SpringAnimator(Widget* widget);
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

/**
 * @class SpringAnimatorImpl
 * @brief Implementation of SpringAnimator that manages spring animation state
 */
class SpringAnimatorImpl : public std::enable_shared_from_this<SpringAnimatorImpl> {
public:
    SpringAnimatorImpl(Widget* widget);
    ~SpringAnimatorImpl() = default;

    void start();
    void pause();
    void resume();
    void stop();
    void reset();

    /**
     * @brief Update the animation
     * @param deltaTimeMs Time elapsed since last update in milliseconds
     * @return true if animation is still running
     */
    bool update(float deltaTimeMs);

    [[nodiscard]] bool isRunning() const;
    [[nodiscard]] bool isCompleted() const;
    [[nodiscard]] float getProgress() const;
    [[nodiscard]] Widget* getWidget() const { return m_widget; }

    // Configuration
    void addProperty(Property prop, float from, float to);
    void setSpringConfig(const SpringConfig& config) { m_springConfig = config; }
    void setOnStart(std::function<void()> cb) { m_onStart = std::move(cb); }
    void setOnComplete(std::function<void()> cb) { m_onComplete = std::move(cb); }
    void setOnUpdate(std::function<void(float)> cb) { m_onUpdate = std::move(cb); }

private:
    void applyCurrentValues();

    Widget* m_widget;
    AnimationState m_state = AnimationState::Idle;
    float m_progress = 0.0f;

    SpringConfig m_springConfig;
    std::vector<PropertyAnimation> m_properties;
    std::map<Property, float> m_velocities;

    std::function<void()> m_onStart;
    std::function<void()> m_onComplete;
    std::function<void(float)> m_onUpdate;
};

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
