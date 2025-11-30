/**
 * @file Animation.hpp
 * @brief Animation system for KillerGK
 */

#pragma once

#include "../widgets/Widget.hpp"
#include <functional>
#include <map>
#include <memory>

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
 * @brief Handle to a built animation
 */
using AnimationHandle = std::shared_ptr<class AnimationImpl>;

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

    // Keyframes
    Animation& keyframe(float percent, std::map<Property, float> values);

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
 * @brief Apply easing function to a value
 * @param easing The easing function to use
 * @param t Progress value between 0 and 1
 * @return Eased value
 */
float applyEasing(Easing easing, float t);

} // namespace KillerGK
