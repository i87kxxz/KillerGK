/**
 * @file Animation.cpp
 * @brief Animation system implementation (stub)
 */

#include "KillerGK/animation/Animation.hpp"
#include <cmath>

namespace KillerGK {

struct Animation::Impl {
    std::map<Property, std::pair<float, float>> properties;
    float duration = 300.0f;
    float delay = 0.0f;
    Easing easing = Easing::Linear;
    int loopCount = 1;
    bool yoyo = false;
    float springStiffness = 0.0f;
    float springDamping = 0.0f;
    std::vector<std::pair<float, std::map<Property, float>>> keyframes;
    std::function<void()> onStartCallback;
    std::function<void()> onCompleteCallback;
    std::function<void(float)> onUpdateCallback;
};

Animation::Animation() : m_impl(std::make_shared<Impl>()) {}

Animation Animation::create() {
    return Animation();
}

Animation& Animation::property(Property prop, float from, float to) {
    m_impl->properties[prop] = {from, to};
    return *this;
}

Animation& Animation::duration(float milliseconds) {
    m_impl->duration = milliseconds;
    return *this;
}

Animation& Animation::delay(float milliseconds) {
    m_impl->delay = milliseconds;
    return *this;
}

Animation& Animation::easing(Easing easing) {
    m_impl->easing = easing;
    return *this;
}

Animation& Animation::loop(int count) {
    m_impl->loopCount = count;
    return *this;
}

Animation& Animation::yoyo(bool enabled) {
    m_impl->yoyo = enabled;
    return *this;
}

Animation& Animation::spring(float stiffness, float damping) {
    m_impl->springStiffness = stiffness;
    m_impl->springDamping = damping;
    return *this;
}

Animation& Animation::keyframe(float percent, std::map<Property, float> values) {
    m_impl->keyframes.emplace_back(percent, std::move(values));
    return *this;
}

Animation& Animation::onStart(std::function<void()> callback) {
    m_impl->onStartCallback = std::move(callback);
    return *this;
}

Animation& Animation::onComplete(std::function<void()> callback) {
    m_impl->onCompleteCallback = std::move(callback);
    return *this;
}

Animation& Animation::onUpdate(std::function<void(float)> callback) {
    m_impl->onUpdateCallback = std::move(callback);
    return *this;
}

AnimationHandle Animation::build() {
    // TODO: Implement
    return nullptr;
}

// Easing functions implementation
float applyEasing(Easing easing, float t) {
    constexpr float PI = 3.14159265358979323846f;
    
    switch (easing) {
        case Easing::Linear:
            return t;
            
        case Easing::EaseIn:
        case Easing::EaseInQuad:
            return t * t;
            
        case Easing::EaseOut:
        case Easing::EaseOutQuad:
            return t * (2 - t);
            
        case Easing::EaseInOut:
        case Easing::EaseInOutQuad:
            return t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
            
        case Easing::EaseInCubic:
            return t * t * t;
            
        case Easing::EaseOutCubic:
            return (--t) * t * t + 1;
            
        case Easing::EaseInOutCubic:
            return t < 0.5f ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
            
        case Easing::EaseInSine:
            return 1 - std::cos(t * PI / 2);
            
        case Easing::EaseOutSine:
            return std::sin(t * PI / 2);
            
        case Easing::EaseInOutSine:
            return -(std::cos(PI * t) - 1) / 2;
            
        case Easing::EaseInBounce:
            return 1 - applyEasing(Easing::EaseOutBounce, 1 - t);
            
        case Easing::EaseOutBounce: {
            if (t < 1 / 2.75f) {
                return 7.5625f * t * t;
            } else if (t < 2 / 2.75f) {
                t -= 1.5f / 2.75f;
                return 7.5625f * t * t + 0.75f;
            } else if (t < 2.5f / 2.75f) {
                t -= 2.25f / 2.75f;
                return 7.5625f * t * t + 0.9375f;
            } else {
                t -= 2.625f / 2.75f;
                return 7.5625f * t * t + 0.984375f;
            }
        }
            
        case Easing::EaseInOutBounce:
            return t < 0.5f
                ? (1 - applyEasing(Easing::EaseOutBounce, 1 - 2 * t)) / 2
                : (1 + applyEasing(Easing::EaseOutBounce, 2 * t - 1)) / 2;
            
        default:
            return t;
    }
}

} // namespace KillerGK
