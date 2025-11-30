/**
 * @file Animation.cpp
 * @brief Animation system implementation
 */

#include "KillerGK/animation/Animation.hpp"
#include "KillerGK/theme/Theme.hpp"
#include <cmath>
#include <algorithm>

namespace KillerGK {

// ============================================================================
// Easing Functions Implementation
// ============================================================================

float applyEasing(Easing easing, float t) {
    // Clamp t to [0, 1]
    t = clamp(t, 0.0f, 1.0f);

    constexpr float PI = 3.14159265358979323846f;
    constexpr float c1 = 1.70158f;
    constexpr float c2 = c1 * 1.525f;
    constexpr float c3 = c1 + 1.0f;
    constexpr float c4 = (2.0f * PI) / 3.0f;
    constexpr float c5 = (2.0f * PI) / 4.5f;

    switch (easing) {
        case Easing::Linear:
            return t;

        // Quadratic
        case Easing::EaseIn:
        case Easing::EaseInQuad:
            return t * t;

        case Easing::EaseOut:
        case Easing::EaseOutQuad:
            return 1.0f - (1.0f - t) * (1.0f - t);

        case Easing::EaseInOut:
        case Easing::EaseInOutQuad:
            return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;

        // Cubic
        case Easing::EaseInCubic:
            return t * t * t;

        case Easing::EaseOutCubic:
            return 1.0f - std::pow(1.0f - t, 3.0f);

        case Easing::EaseInOutCubic:
            return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;

        // Quartic
        case Easing::EaseInQuart:
            return t * t * t * t;

        case Easing::EaseOutQuart:
            return 1.0f - std::pow(1.0f - t, 4.0f);

        case Easing::EaseInOutQuart:
            return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 4.0f) / 2.0f;

        // Quintic
        case Easing::EaseInQuint:
            return t * t * t * t * t;

        case Easing::EaseOutQuint:
            return 1.0f - std::pow(1.0f - t, 5.0f);

        case Easing::EaseInOutQuint:
            return t < 0.5f ? 16.0f * t * t * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 5.0f) / 2.0f;

        // Sine
        case Easing::EaseInSine:
            return 1.0f - std::cos((t * PI) / 2.0f);

        case Easing::EaseOutSine:
            return std::sin((t * PI) / 2.0f);

        case Easing::EaseInOutSine:
            return -(std::cos(PI * t) - 1.0f) / 2.0f;

        // Exponential
        case Easing::EaseInExpo:
            return t == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * t - 10.0f);

        case Easing::EaseOutExpo:
            return t == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);

        case Easing::EaseInOutExpo:
            if (t == 0.0f) return 0.0f;
            if (t == 1.0f) return 1.0f;
            return t < 0.5f
                ? std::pow(2.0f, 20.0f * t - 10.0f) / 2.0f
                : (2.0f - std::pow(2.0f, -20.0f * t + 10.0f)) / 2.0f;

        // Circular
        case Easing::EaseInCirc:
            return 1.0f - std::sqrt(1.0f - std::pow(t, 2.0f));

        case Easing::EaseOutCirc:
            return std::sqrt(1.0f - std::pow(t - 1.0f, 2.0f));

        case Easing::EaseInOutCirc:
            return t < 0.5f
                ? (1.0f - std::sqrt(1.0f - std::pow(2.0f * t, 2.0f))) / 2.0f
                : (std::sqrt(1.0f - std::pow(-2.0f * t + 2.0f, 2.0f)) + 1.0f) / 2.0f;

        // Elastic
        case Easing::EaseInElastic:
            if (t == 0.0f) return 0.0f;
            if (t == 1.0f) return 1.0f;
            return -std::pow(2.0f, 10.0f * t - 10.0f) * std::sin((t * 10.0f - 10.75f) * c4);

        case Easing::EaseOutElastic:
            if (t == 0.0f) return 0.0f;
            if (t == 1.0f) return 1.0f;
            return std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * c4) + 1.0f;

        case Easing::EaseInOutElastic:
            if (t == 0.0f) return 0.0f;
            if (t == 1.0f) return 1.0f;
            return t < 0.5f
                ? -(std::pow(2.0f, 20.0f * t - 10.0f) * std::sin((20.0f * t - 11.125f) * c5)) / 2.0f
                : (std::pow(2.0f, -20.0f * t + 10.0f) * std::sin((20.0f * t - 11.125f) * c5)) / 2.0f + 1.0f;

        // Back
        case Easing::EaseInBack:
            return c3 * t * t * t - c1 * t * t;

        case Easing::EaseOutBack:
            return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);

        case Easing::EaseInOutBack:
            return t < 0.5f
                ? (std::pow(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f
                : (std::pow(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;

        // Bounce
        case Easing::EaseInBounce:
            return 1.0f - applyEasing(Easing::EaseOutBounce, 1.0f - t);

        case Easing::EaseOutBounce: {
            constexpr float n1 = 7.5625f;
            constexpr float d1 = 2.75f;

            if (t < 1.0f / d1) {
                return n1 * t * t;
            } else if (t < 2.0f / d1) {
                float t2 = t - 1.5f / d1;
                return n1 * t2 * t2 + 0.75f;
            } else if (t < 2.5f / d1) {
                float t2 = t - 2.25f / d1;
                return n1 * t2 * t2 + 0.9375f;
            } else {
                float t2 = t - 2.625f / d1;
                return n1 * t2 * t2 + 0.984375f;
            }
        }

        case Easing::EaseInOutBounce:
            return t < 0.5f
                ? (1.0f - applyEasing(Easing::EaseOutBounce, 1.0f - 2.0f * t)) / 2.0f
                : (1.0f + applyEasing(Easing::EaseOutBounce, 2.0f * t - 1.0f)) / 2.0f;

        default:
            return t;
    }
}

// ============================================================================
// AnimationImpl Implementation
// ============================================================================

AnimationImpl::AnimationImpl() = default;

void AnimationImpl::start() {
    if (m_state == AnimationState::Running) return;

    m_state = AnimationState::Running;
    m_elapsedTime = 0.0f;
    m_progress = 0.0f;
    m_currentLoop = 0;
    m_reversed = false;

    // Initialize spring velocities
    if (m_type == AnimationType::Spring) {
        m_springVelocities.clear();
        for (auto& prop : m_properties) {
            m_springVelocities[prop.property] = m_springConfig.velocity;
            prop.currentValue = prop.fromValue;
        }
    }

    if (m_onStart) {
        m_onStart();
    }
}

void AnimationImpl::pause() {
    if (m_state == AnimationState::Running) {
        m_state = AnimationState::Paused;
    }
}

void AnimationImpl::resume() {
    if (m_state == AnimationState::Paused) {
        m_state = AnimationState::Running;
    }
}

void AnimationImpl::stop() {
    m_state = AnimationState::Completed;
    if (m_onComplete) {
        m_onComplete();
    }
}

void AnimationImpl::reset() {
    m_state = AnimationState::Idle;
    m_elapsedTime = 0.0f;
    m_progress = 0.0f;
    m_currentLoop = 0;
    m_reversed = false;

    for (auto& prop : m_properties) {
        prop.currentValue = prop.fromValue;
    }
}

bool AnimationImpl::update(float deltaTimeMs) {
    if (m_state != AnimationState::Running) {
        return m_state != AnimationState::Completed;
    }

    m_elapsedTime += deltaTimeMs;

    // Handle delay
    if (m_elapsedTime < m_delay) {
        return true;
    }

    float activeTime = m_elapsedTime - m_delay;

    // Spring animations don't use duration-based progress
    if (m_type == AnimationType::Spring) {
        updateSpring(deltaTimeMs);

        // Check if spring is at rest
        bool atRest = true;
        for (const auto& prop : m_properties) {
            float distance = std::abs(prop.currentValue - prop.toValue);
            float velocity = std::abs(m_springVelocities[prop.property]);
            if (distance > m_springConfig.restThreshold || velocity > m_springConfig.restThreshold) {
                atRest = false;
                break;
            }
        }

        if (atRest) {
            // Snap to final values
            for (auto& prop : m_properties) {
                prop.currentValue = prop.toValue;
            }
            m_progress = 1.0f;
            stop();
            return false;
        }

        if (m_onUpdate) {
            m_onUpdate(m_progress);
        }
        return true;
    }

    // Calculate progress for tween/keyframe animations
    float rawProgress = m_duration > 0.0f ? activeTime / m_duration : 1.0f;

    // Handle looping
    if (rawProgress >= 1.0f) {
        m_currentLoop++;

        if (m_loopCount == -1 || m_currentLoop < m_loopCount) {
            // Continue looping
            if (m_yoyo) {
                m_reversed = !m_reversed;
            }
            m_elapsedTime = m_delay;
            rawProgress = 0.0f;
        } else {
            // Animation complete
            rawProgress = 1.0f;
            m_progress = m_reversed ? 0.0f : 1.0f;

            if (m_type == AnimationType::Keyframe) {
                updateKeyframe(m_progress);
            } else {
                updateTween(m_progress);
            }

            stop();
            return false;
        }
    }

    // Apply yoyo reversal
    m_progress = m_reversed ? (1.0f - rawProgress) : rawProgress;

    // Update based on animation type
    if (m_type == AnimationType::Keyframe) {
        updateKeyframe(m_progress);
    } else {
        updateTween(m_progress);
    }

    if (m_onUpdate) {
        m_onUpdate(m_progress);
    }

    return true;
}

void AnimationImpl::updateTween(float progress) {
    float easedProgress = applyEasing(m_easing, progress);

    for (auto& prop : m_properties) {
        prop.currentValue = lerp(prop.fromValue, prop.toValue, easedProgress);
    }
}

void AnimationImpl::updateSpring(float deltaTimeMs) {
    // Convert to seconds for physics calculations
    float dt = deltaTimeMs / 1000.0f;

    for (auto& prop : m_properties) {
        float& velocity = m_springVelocities[prop.property];
        float displacement = prop.currentValue - prop.toValue;

        // Spring force: F = -kx - cv
        // Where k = stiffness, x = displacement, c = damping, v = velocity
        float springForce = -m_springConfig.stiffness * displacement;
        float dampingForce = -m_springConfig.damping * velocity;
        float acceleration = (springForce + dampingForce) / m_springConfig.mass;

        // Update velocity and position using semi-implicit Euler
        velocity += acceleration * dt;
        prop.currentValue += velocity * dt;
    }

    // Calculate approximate progress based on distance to target
    if (!m_properties.empty()) {
        float totalDistance = 0.0f;
        float currentDistance = 0.0f;
        for (const auto& prop : m_properties) {
            totalDistance += std::abs(prop.toValue - prop.fromValue);
            currentDistance += std::abs(prop.currentValue - prop.fromValue);
        }
        m_progress = totalDistance > 0.0f ? clamp(currentDistance / totalDistance, 0.0f, 1.0f) : 1.0f;
    }
}

void AnimationImpl::updateKeyframe(float progress) {
    for (auto& prop : m_properties) {
        prop.currentValue = interpolateKeyframes(prop.property, progress);
    }
}

float AnimationImpl::interpolateKeyframes(Property prop, float progress) const {
    if (m_keyframes.empty()) {
        // Fall back to property from/to values
        for (const auto& p : m_properties) {
            if (p.property == prop) {
                return lerp(p.fromValue, p.toValue, applyEasing(m_easing, progress));
            }
        }
        return 0.0f;
    }

    // Find surrounding keyframes
    const Keyframe* prevKeyframe = nullptr;
    const Keyframe* nextKeyframe = nullptr;

    for (const auto& kf : m_keyframes) {
        if (kf.percent <= progress) {
            if (kf.values.count(prop)) {
                prevKeyframe = &kf;
            }
        }
        if (kf.percent >= progress && nextKeyframe == nullptr) {
            if (kf.values.count(prop)) {
                nextKeyframe = &kf;
            }
        }
    }

    // Handle edge cases
    if (!prevKeyframe && !nextKeyframe) {
        return 0.0f;
    }
    if (!prevKeyframe) {
        return nextKeyframe->values.at(prop);
    }
    if (!nextKeyframe) {
        return prevKeyframe->values.at(prop);
    }
    if (prevKeyframe == nextKeyframe) {
        return prevKeyframe->values.at(prop);
    }

    // Interpolate between keyframes
    float keyframeProgress = (progress - prevKeyframe->percent) /
                             (nextKeyframe->percent - prevKeyframe->percent);
    float easedProgress = applyEasing(nextKeyframe->easing, keyframeProgress);

    return lerp(prevKeyframe->values.at(prop), nextKeyframe->values.at(prop), easedProgress);
}

float AnimationImpl::getCurrentValue(Property prop) const {
    for (const auto& p : m_properties) {
        if (p.property == prop) {
            return p.currentValue;
        }
    }
    return 0.0f;
}

void AnimationImpl::addProperty(Property prop, float from, float to) {
    m_properties.emplace_back(prop, from, to);
}

void AnimationImpl::addKeyframe(const Keyframe& keyframe) {
    m_keyframes.push_back(keyframe);
    // Sort keyframes by percent
    std::sort(m_keyframes.begin(), m_keyframes.end(),
              [](const Keyframe& a, const Keyframe& b) { return a.percent < b.percent; });

    // Extract properties from keyframes
    for (const auto& [prop, value] : keyframe.values) {
        bool found = false;
        for (auto& p : m_properties) {
            if (p.property == prop) {
                found = true;
                break;
            }
        }
        if (!found) {
            m_properties.emplace_back(prop, value, value);
        }
    }
}

// ============================================================================
// Animation Builder Implementation
// ============================================================================

struct Animation::Impl {
    std::vector<PropertyAnimation> properties;
    float duration = 300.0f;
    float delay = 0.0f;
    Easing easing = Easing::Linear;
    int loopCount = 1;
    bool yoyo = false;
    SpringConfig springConfig;
    bool useSpring = false;
    std::vector<Keyframe> keyframes;
    std::function<void()> onStartCallback;
    std::function<void()> onCompleteCallback;
    std::function<void(float)> onUpdateCallback;
};

Animation::Animation() : m_impl(std::make_shared<Impl>()) {}

Animation Animation::create() {
    return Animation();
}

Animation& Animation::property(Property prop, float from, float to) {
    m_impl->properties.emplace_back(prop, from, to);
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
    m_impl->useSpring = true;
    m_impl->springConfig.stiffness = stiffness;
    m_impl->springConfig.damping = damping;
    return *this;
}

Animation& Animation::springMass(float mass) {
    m_impl->springConfig.mass = mass;
    return *this;
}

Animation& Animation::springVelocity(float velocity) {
    m_impl->springConfig.velocity = velocity;
    return *this;
}

Animation& Animation::keyframe(float percent, std::map<Property, float> values) {
    m_impl->keyframes.emplace_back(percent, std::move(values), Easing::Linear);
    return *this;
}

Animation& Animation::keyframe(float percent, std::map<Property, float> values, Easing easing) {
    m_impl->keyframes.emplace_back(percent, std::move(values), easing);
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
    auto anim = std::make_shared<AnimationImpl>();

    // Determine animation type
    if (m_impl->useSpring) {
        anim->setType(AnimationType::Spring);
        anim->setSpringConfig(m_impl->springConfig);
    } else if (!m_impl->keyframes.empty()) {
        anim->setType(AnimationType::Keyframe);
        for (const auto& kf : m_impl->keyframes) {
            anim->addKeyframe(kf);
        }
    } else {
        anim->setType(AnimationType::Tween);
    }

    // Set common properties
    anim->setDuration(m_impl->duration);
    anim->setDelay(m_impl->delay);
    anim->setEasing(m_impl->easing);
    anim->setLoopCount(m_impl->loopCount);
    anim->setYoyo(m_impl->yoyo);

    // Add property animations
    for (const auto& prop : m_impl->properties) {
        anim->addProperty(prop.property, prop.fromValue, prop.toValue);
    }

    // Set callbacks
    if (m_impl->onStartCallback) {
        anim->setOnStart(m_impl->onStartCallback);
    }
    if (m_impl->onCompleteCallback) {
        anim->setOnComplete(m_impl->onCompleteCallback);
    }
    if (m_impl->onUpdateCallback) {
        anim->setOnUpdate(m_impl->onUpdateCallback);
    }

    return anim;
}


// ============================================================================
// AnimationTimeline Implementation
// ============================================================================

AnimationTimeline::AnimationTimeline() = default;

void AnimationTimeline::add(AnimationHandle animation, float startTime) {
    addAt(animation, startTime);
}

void AnimationTimeline::addAt(AnimationHandle animation, float startTime) {
    m_entries.push_back({animation, startTime, false});
}

void AnimationTimeline::addAfter(AnimationHandle animation, AnimationHandle after, float delay) {
    float startTime = 0.0f;

    // Find the end time of the 'after' animation
    for (const auto& entry : m_entries) {
        if (entry.animation == after) {
            startTime = entry.startTime + entry.animation->getDuration() + entry.animation->getDelay();
            break;
        }
    }

    addAt(animation, startTime + delay);
}

void AnimationTimeline::play() {
    m_playing = true;
    m_currentTime = 0.0f;

    // Reset all entries
    for (auto& entry : m_entries) {
        entry.started = false;
        entry.animation->reset();
    }
}

void AnimationTimeline::pause() {
    m_playing = false;

    // Pause all running animations
    for (auto& entry : m_entries) {
        if (entry.started && entry.animation->isRunning()) {
            entry.animation->pause();
        }
    }
}

void AnimationTimeline::stop() {
    m_playing = false;

    for (auto& entry : m_entries) {
        entry.animation->stop();
    }

    if (m_onComplete) {
        m_onComplete();
    }
}

void AnimationTimeline::reset() {
    m_playing = false;
    m_currentTime = 0.0f;

    for (auto& entry : m_entries) {
        entry.started = false;
        entry.animation->reset();
    }
}

void AnimationTimeline::seek(float timeMs) {
    m_currentTime = timeMs;

    for (auto& entry : m_entries) {
        if (timeMs >= entry.startTime) {
            if (!entry.started) {
                entry.started = true;
                entry.animation->start();
            }

            // Calculate how much time has passed since this animation started
            float animTime = timeMs - entry.startTime;
            float duration = entry.animation->getDuration() + entry.animation->getDelay();

            if (animTime >= duration) {
                // Animation should be complete
                entry.animation->stop();
            } else {
                // Update animation to correct position
                entry.animation->reset();
                entry.animation->start();
                entry.animation->update(animTime);
            }
        } else {
            entry.started = false;
            entry.animation->reset();
        }
    }
}

bool AnimationTimeline::update(float deltaTimeMs) {
    if (!m_playing) {
        return false;
    }

    m_currentTime += deltaTimeMs;

    bool anyRunning = false;

    for (auto& entry : m_entries) {
        // Start animation if it's time
        if (!entry.started && m_currentTime >= entry.startTime) {
            entry.started = true;
            entry.animation->start();
        }

        // Update running animations
        if (entry.started) {
            if (entry.animation->update(deltaTimeMs)) {
                anyRunning = true;
            }
        } else {
            // Animation hasn't started yet
            anyRunning = true;
        }
    }

    if (!anyRunning) {
        m_playing = false;
        if (m_onComplete) {
            m_onComplete();
        }
    }

    return anyRunning;
}

float AnimationTimeline::getTotalDuration() const {
    float maxEnd = 0.0f;

    for (const auto& entry : m_entries) {
        float endTime = entry.startTime + entry.animation->getDuration() + entry.animation->getDelay();
        maxEnd = std::max(maxEnd, endTime);
    }

    return maxEnd;
}

// ============================================================================
// AnimationGroup Implementation
// ============================================================================

AnimationGroup::AnimationGroup(GroupMode mode) : m_mode(mode) {}

AnimationGroup& AnimationGroup::add(AnimationHandle animation) {
    m_entries.push_back({animation, 0.0f, 0.0f, false});
    return *this;
}

AnimationGroup& AnimationGroup::addWithDelay(AnimationHandle animation, float delay) {
    m_entries.push_back({animation, delay, 0.0f, false});
    return *this;
}

AnimationGroup& AnimationGroup::stagger(float delayBetween) {
    m_staggerDelay = delayBetween;
    return *this;
}

void AnimationGroup::play() {
    m_playing = true;
    m_currentIndex = 0;

    // Reset all entries
    for (auto& entry : m_entries) {
        entry.started = false;
        entry.elapsedDelay = 0.0f;
        entry.animation->reset();
    }

    // Apply stagger delays for sequence mode
    if (m_mode == GroupMode::Sequence && m_staggerDelay > 0.0f) {
        float cumulativeDelay = 0.0f;
        for (auto& entry : m_entries) {
            entry.delay += cumulativeDelay;
            cumulativeDelay += m_staggerDelay;
        }
    }

    // For parallel mode, start all animations immediately (respecting delays)
    if (m_mode == GroupMode::Parallel) {
        for (size_t i = 0; i < m_entries.size(); ++i) {
            if (m_staggerDelay > 0.0f) {
                m_entries[i].delay += i * m_staggerDelay;
            }
        }
    }
}

void AnimationGroup::pause() {
    m_playing = false;

    for (auto& entry : m_entries) {
        if (entry.started && entry.animation->isRunning()) {
            entry.animation->pause();
        }
    }
}

void AnimationGroup::stop() {
    m_playing = false;

    for (auto& entry : m_entries) {
        entry.animation->stop();
    }

    if (m_onComplete) {
        m_onComplete();
    }
}

void AnimationGroup::reset() {
    m_playing = false;
    m_currentIndex = 0;

    for (auto& entry : m_entries) {
        entry.started = false;
        entry.elapsedDelay = 0.0f;
        entry.animation->reset();
    }
}

bool AnimationGroup::update(float deltaTimeMs) {
    if (!m_playing || m_entries.empty()) {
        return false;
    }

    bool anyRunning = false;

    if (m_mode == GroupMode::Sequence) {
        // Process animations one at a time
        if (m_currentIndex < m_entries.size()) {
            auto& entry = m_entries[m_currentIndex];

            // Handle delay
            if (entry.elapsedDelay < entry.delay) {
                entry.elapsedDelay += deltaTimeMs;
                return true;
            }

            // Start animation if not started
            if (!entry.started) {
                entry.started = true;
                entry.animation->start();
            }

            // Update current animation
            if (entry.animation->update(deltaTimeMs)) {
                anyRunning = true;
            } else {
                // Move to next animation
                m_currentIndex++;
                if (m_currentIndex < m_entries.size()) {
                    anyRunning = true;
                }
            }
        }
    } else {
        // Parallel mode - update all animations
        for (auto& entry : m_entries) {
            // Handle delay
            if (entry.elapsedDelay < entry.delay) {
                entry.elapsedDelay += deltaTimeMs;
                anyRunning = true;
                continue;
            }

            // Start animation if not started
            if (!entry.started) {
                entry.started = true;
                entry.animation->start();
            }

            // Update animation
            if (entry.animation->update(deltaTimeMs)) {
                anyRunning = true;
            }
        }
    }

    if (!anyRunning) {
        m_playing = false;
        if (m_onComplete) {
            m_onComplete();
        }
    }

    return anyRunning;
}

AnimationGroup& AnimationGroup::onComplete(std::function<void()> callback) {
    m_onComplete = std::move(callback);
    return *this;
}

// ============================================================================
// AnimationManager Implementation
// ============================================================================

AnimationManager& AnimationManager::instance() {
    static AnimationManager instance;
    return instance;
}

void AnimationManager::registerAnimation(AnimationHandle animation) {
    m_animations.push_back(animation);
}

void AnimationManager::unregisterAnimation(AnimationHandle animation) {
    m_animations.erase(
        std::remove_if(m_animations.begin(), m_animations.end(),
                       [&animation](const std::weak_ptr<AnimationImpl>& weak) {
                           auto shared = weak.lock();
                           return !shared || shared == animation;
                       }),
        m_animations.end());
}

void AnimationManager::update(float deltaTimeMs) {
    // Remove expired animations and update active ones
    m_animations.erase(
        std::remove_if(m_animations.begin(), m_animations.end(),
                       [deltaTimeMs](std::weak_ptr<AnimationImpl>& weak) {
                           auto shared = weak.lock();
                           if (!shared) {
                               return true;  // Remove expired
                           }
                           if (shared->isRunning()) {
                               shared->update(deltaTimeMs);
                           }
                           return shared->isCompleted();
                       }),
        m_animations.end());
}

AnimationHandle AnimationManager::createTween(Property prop, float from, float to, float duration, Easing easing) {
    auto anim = Animation::create()
                    .property(prop, from, to)
                    .duration(duration)
                    .easing(easing)
                    .build();
    registerAnimation(anim);
    return anim;
}

AnimationHandle AnimationManager::createSpring(Property prop, float from, float to, float stiffness, float damping) {
    auto anim = Animation::create()
                    .property(prop, from, to)
                    .spring(stiffness, damping)
                    .build();
    registerAnimation(anim);
    return anim;
}

void AnimationManager::clear() {
    m_animations.clear();
}

size_t AnimationManager::getActiveAnimationCount() const {
    size_t count = 0;
    for (const auto& weak : m_animations) {
        auto shared = weak.lock();
        if (shared && shared->isRunning()) {
            count++;
        }
    }
    return count;
}

// ============================================================================
// StateTransitionManager Implementation
// ============================================================================

StateTransitionManager::StateTransitionManager() {
    // Set up default transitions
    m_transitions[WidgetStateType::Normal] = StateTransitionConfig(150.0f, Easing::EaseOut);
    m_transitions[WidgetStateType::Hovered] = StateTransitionConfig(100.0f, Easing::EaseOut);
    m_transitions[WidgetStateType::Pressed] = StateTransitionConfig(50.0f, Easing::EaseOut);
    m_transitions[WidgetStateType::Focused] = StateTransitionConfig(150.0f, Easing::EaseInOut);
    m_transitions[WidgetStateType::Disabled] = StateTransitionConfig(200.0f, Easing::EaseOut);
}

void StateTransitionManager::setTransition(WidgetStateType state, const StateTransitionConfig& config) {
    m_transitions[state] = config;
}

const StateTransitionConfig& StateTransitionManager::getTransition(WidgetStateType state) const {
    static StateTransitionConfig defaultConfig;
    auto it = m_transitions.find(state);
    return it != m_transitions.end() ? it->second : defaultConfig;
}

void StateTransitionManager::transitionTo(WidgetStateType newState, Widget* widget) {
    if (newState == m_currentState || !widget) {
        return;
    }

    m_targetState = newState;
    m_targetWidget = widget;

    const auto& config = getTransition(newState);

    // Create animation for the transition
    auto animBuilder = Animation::create()
                           .duration(config.duration)
                           .easing(config.easing);

    // Add property animations based on state deltas
    for (const auto& [prop, delta] : config.propertyDeltas) {
        float currentValue = 0.0f;

        // Get current value from widget
        switch (prop) {
            case Property::Opacity:
                currentValue = widget->getOpacity();
                break;
            case Property::Scale:
                currentValue = 1.0f;  // Default scale
                break;
            case Property::BackgroundColorA:
                currentValue = widget->getBackgroundColor().a;
                break;
            default:
                break;
        }

        animBuilder.property(prop, currentValue, currentValue + delta);
    }

    m_activeTransition = animBuilder.build();
    m_activeTransition->start();
    m_currentState = newState;
}

bool StateTransitionManager::update(float deltaTimeMs) {
    if (!m_activeTransition || !m_activeTransition->isRunning()) {
        return false;
    }

    bool running = m_activeTransition->update(deltaTimeMs);

    // Apply animated values to widget
    if (m_targetWidget) {
        for (const auto& prop : m_activeTransition->getProperties()) {
            switch (prop.property) {
                case Property::Opacity:
                    m_targetWidget->opacity(prop.currentValue);
                    break;
                case Property::BackgroundColorA: {
                    Color bg = m_targetWidget->getBackgroundColor();
                    bg.a = prop.currentValue;
                    m_targetWidget->backgroundColor(bg);
                    break;
                }
                default:
                    break;
            }
        }
    }

    return running;
}

void StateTransitionManager::applyThemeDefaults(const Theme& theme) {
    // Configure transitions based on theme settings
    // Theme can be used to customize transition durations/colors in the future
    (void)theme;  // Suppress unused parameter warning for now

    // Hover state - slight opacity change
    StateTransitionConfig hoverConfig(100.0f, Easing::EaseOut);
    hoverConfig.propertyDeltas[Property::Opacity] = 0.1f;
    setTransition(WidgetStateType::Hovered, hoverConfig);

    // Pressed state - quick response
    StateTransitionConfig pressedConfig(50.0f, Easing::EaseOut);
    pressedConfig.propertyDeltas[Property::Opacity] = -0.1f;
    pressedConfig.propertyDeltas[Property::Scale] = -0.02f;
    setTransition(WidgetStateType::Pressed, pressedConfig);

    // Focused state
    StateTransitionConfig focusedConfig(150.0f, Easing::EaseInOut);
    setTransition(WidgetStateType::Focused, focusedConfig);

    // Disabled state - fade out
    StateTransitionConfig disabledConfig(200.0f, Easing::EaseOut);
    disabledConfig.propertyDeltas[Property::Opacity] = -0.4f;
    setTransition(WidgetStateType::Disabled, disabledConfig);
}

// ============================================================================
// Property Value Helpers Implementation
// ============================================================================

float getWidgetPropertyValue(const Widget& widget, Property prop) {
    switch (prop) {
        case Property::X:
            return widget.getPropertyFloat("x", 0.0f);
        case Property::Y:
            return widget.getPropertyFloat("y", 0.0f);
        case Property::Width:
            return widget.getWidth();
        case Property::Height:
            return widget.getHeight();
        case Property::Opacity:
            return widget.getOpacity();
        case Property::Rotation:
            return widget.getPropertyFloat("rotation", 0.0f);
        case Property::Scale:
            return widget.getPropertyFloat("scale", 1.0f);
        case Property::BackgroundColorR:
            return widget.getBackgroundColor().r;
        case Property::BackgroundColorG:
            return widget.getBackgroundColor().g;
        case Property::BackgroundColorB:
            return widget.getBackgroundColor().b;
        case Property::BackgroundColorA:
            return widget.getBackgroundColor().a;
        case Property::BorderRadius:
            return widget.getBorderRadius();
        case Property::BorderWidth:
            return widget.getBorderWidth();
        case Property::MarginTop:
            return widget.getMargin().top;
        case Property::MarginRight:
            return widget.getMargin().right;
        case Property::MarginBottom:
            return widget.getMargin().bottom;
        case Property::MarginLeft:
            return widget.getMargin().left;
        case Property::PaddingTop:
            return widget.getPadding().top;
        case Property::PaddingRight:
            return widget.getPadding().right;
        case Property::PaddingBottom:
            return widget.getPadding().bottom;
        case Property::PaddingLeft:
            return widget.getPadding().left;
        default:
            return 0.0f;
    }
}

void setWidgetPropertyValue(Widget& widget, Property prop, float value) {
    switch (prop) {
        case Property::X:
            widget.setPropertyFloat("x", value);
            break;
        case Property::Y:
            widget.setPropertyFloat("y", value);
            break;
        case Property::Width:
            widget.width(value);
            break;
        case Property::Height:
            widget.height(value);
            break;
        case Property::Opacity:
            widget.opacity(value);
            break;
        case Property::Rotation:
            widget.setPropertyFloat("rotation", value);
            break;
        case Property::Scale:
            widget.setPropertyFloat("scale", value);
            break;
        case Property::BackgroundColorR: {
            Color bg = widget.getBackgroundColor();
            bg.r = value;
            widget.backgroundColor(bg);
            break;
        }
        case Property::BackgroundColorG: {
            Color bg = widget.getBackgroundColor();
            bg.g = value;
            widget.backgroundColor(bg);
            break;
        }
        case Property::BackgroundColorB: {
            Color bg = widget.getBackgroundColor();
            bg.b = value;
            widget.backgroundColor(bg);
            break;
        }
        case Property::BackgroundColorA: {
            Color bg = widget.getBackgroundColor();
            bg.a = value;
            widget.backgroundColor(bg);
            break;
        }
        case Property::BorderRadius:
            widget.borderRadius(value);
            break;
        case Property::BorderWidth:
            widget.borderWidth(value);
            break;
        case Property::MarginTop: {
            Spacing m = widget.getMargin();
            widget.margin(value, m.right, m.bottom, m.left);
            break;
        }
        case Property::MarginRight: {
            Spacing m = widget.getMargin();
            widget.margin(m.top, value, m.bottom, m.left);
            break;
        }
        case Property::MarginBottom: {
            Spacing m = widget.getMargin();
            widget.margin(m.top, m.right, value, m.left);
            break;
        }
        case Property::MarginLeft: {
            Spacing m = widget.getMargin();
            widget.margin(m.top, m.right, m.bottom, value);
            break;
        }
        case Property::PaddingTop: {
            Spacing p = widget.getPadding();
            widget.padding(value, p.right, p.bottom, p.left);
            break;
        }
        case Property::PaddingRight: {
            Spacing p = widget.getPadding();
            widget.padding(p.top, value, p.bottom, p.left);
            break;
        }
        case Property::PaddingBottom: {
            Spacing p = widget.getPadding();
            widget.padding(p.top, p.right, value, p.left);
            break;
        }
        case Property::PaddingLeft: {
            Spacing p = widget.getPadding();
            widget.padding(p.top, p.right, p.bottom, value);
            break;
        }
        default:
            break;
    }
}

bool isAnimatableProperty(Property prop) {
    // All properties in the Property enum are animatable
    switch (prop) {
        case Property::X:
        case Property::Y:
        case Property::Width:
        case Property::Height:
        case Property::Opacity:
        case Property::Rotation:
        case Property::Scale:
        case Property::BackgroundColorR:
        case Property::BackgroundColorG:
        case Property::BackgroundColorB:
        case Property::BackgroundColorA:
        case Property::BorderRadius:
        case Property::BorderWidth:
        case Property::MarginTop:
        case Property::MarginRight:
        case Property::MarginBottom:
        case Property::MarginLeft:
        case Property::PaddingTop:
        case Property::PaddingRight:
        case Property::PaddingBottom:
        case Property::PaddingLeft:
            return true;
        default:
            return false;
    }
}

const char* getPropertyName(Property prop) {
    switch (prop) {
        case Property::X: return "X";
        case Property::Y: return "Y";
        case Property::Width: return "Width";
        case Property::Height: return "Height";
        case Property::Opacity: return "Opacity";
        case Property::Rotation: return "Rotation";
        case Property::Scale: return "Scale";
        case Property::BackgroundColorR: return "BackgroundColorR";
        case Property::BackgroundColorG: return "BackgroundColorG";
        case Property::BackgroundColorB: return "BackgroundColorB";
        case Property::BackgroundColorA: return "BackgroundColorA";
        case Property::BorderRadius: return "BorderRadius";
        case Property::BorderWidth: return "BorderWidth";
        case Property::MarginTop: return "MarginTop";
        case Property::MarginRight: return "MarginRight";
        case Property::MarginBottom: return "MarginBottom";
        case Property::MarginLeft: return "MarginLeft";
        case Property::PaddingTop: return "PaddingTop";
        case Property::PaddingRight: return "PaddingRight";
        case Property::PaddingBottom: return "PaddingBottom";
        case Property::PaddingLeft: return "PaddingLeft";
        default: return "Unknown";
    }
}

// ============================================================================
// TweenAnimator Builder Implementation
// ============================================================================

struct TweenAnimator::Impl {
    Widget* widget = nullptr;
    std::vector<PropertyAnimation> properties;
    float duration = 300.0f;
    float delay = 0.0f;
    Easing easing = Easing::Linear;
    int loopCount = 1;
    bool yoyo = false;
    std::function<void()> onStartCallback;
    std::function<void()> onCompleteCallback;
    std::function<void(float)> onUpdateCallback;
};

TweenAnimator::TweenAnimator(Widget* widget) : m_impl(std::make_shared<Impl>()) {
    m_impl->widget = widget;
}

TweenAnimator TweenAnimator::create(Widget* widget) {
    return TweenAnimator(widget);
}

TweenAnimator& TweenAnimator::property(Property prop, float from, float to) {
    m_impl->properties.emplace_back(prop, from, to);
    return *this;
}

TweenAnimator& TweenAnimator::propertyTo(Property prop, float to) {
    if (m_impl->widget) {
        float from = getWidgetPropertyValue(*m_impl->widget, prop);
        m_impl->properties.emplace_back(prop, from, to);
    }
    return *this;
}

TweenAnimator& TweenAnimator::duration(float milliseconds) {
    m_impl->duration = milliseconds;
    return *this;
}

TweenAnimator& TweenAnimator::delay(float milliseconds) {
    m_impl->delay = milliseconds;
    return *this;
}

TweenAnimator& TweenAnimator::easing(Easing easing) {
    m_impl->easing = easing;
    return *this;
}

TweenAnimator& TweenAnimator::loop(int count) {
    m_impl->loopCount = count;
    return *this;
}

TweenAnimator& TweenAnimator::yoyo(bool enabled) {
    m_impl->yoyo = enabled;
    return *this;
}

TweenAnimator& TweenAnimator::onStart(std::function<void()> callback) {
    m_impl->onStartCallback = std::move(callback);
    return *this;
}

TweenAnimator& TweenAnimator::onComplete(std::function<void()> callback) {
    m_impl->onCompleteCallback = std::move(callback);
    return *this;
}

TweenAnimator& TweenAnimator::onUpdate(std::function<void(float)> callback) {
    m_impl->onUpdateCallback = std::move(callback);
    return *this;
}

std::shared_ptr<TweenAnimatorImpl> TweenAnimator::build() {
    auto animator = std::make_shared<TweenAnimatorImpl>(m_impl->widget);

    animator->setDuration(m_impl->duration);
    animator->setDelay(m_impl->delay);
    animator->setEasing(m_impl->easing);
    animator->setLoopCount(m_impl->loopCount);
    animator->setYoyo(m_impl->yoyo);

    for (const auto& prop : m_impl->properties) {
        animator->addProperty(prop.property, prop.fromValue, prop.toValue);
    }

    if (m_impl->onStartCallback) {
        animator->setOnStart(m_impl->onStartCallback);
    }
    if (m_impl->onCompleteCallback) {
        animator->setOnComplete(m_impl->onCompleteCallback);
    }
    if (m_impl->onUpdateCallback) {
        animator->setOnUpdate(m_impl->onUpdateCallback);
    }

    return animator;
}

// ============================================================================
// TweenAnimatorImpl Implementation
// ============================================================================

TweenAnimatorImpl::TweenAnimatorImpl(Widget* widget) : m_widget(widget) {}

void TweenAnimatorImpl::addProperty(Property prop, float from, float to) {
    m_properties.emplace_back(prop, from, to);
}

void TweenAnimatorImpl::start() {
    if (m_state == AnimationState::Running) return;

    m_state = AnimationState::Running;
    m_elapsedTime = 0.0f;
    m_progress = 0.0f;
    m_currentLoop = 0;
    m_reversed = false;

    // Initialize current values
    for (auto& prop : m_properties) {
        prop.currentValue = prop.fromValue;
    }

    // Apply initial values
    applyCurrentValues();

    if (m_onStart) {
        m_onStart();
    }
}

void TweenAnimatorImpl::pause() {
    if (m_state == AnimationState::Running) {
        m_state = AnimationState::Paused;
    }
}

void TweenAnimatorImpl::resume() {
    if (m_state == AnimationState::Paused) {
        m_state = AnimationState::Running;
    }
}

void TweenAnimatorImpl::stop() {
    m_state = AnimationState::Completed;
    if (m_onComplete) {
        m_onComplete();
    }
}

void TweenAnimatorImpl::reset() {
    m_state = AnimationState::Idle;
    m_elapsedTime = 0.0f;
    m_progress = 0.0f;
    m_currentLoop = 0;
    m_reversed = false;

    for (auto& prop : m_properties) {
        prop.currentValue = prop.fromValue;
    }

    applyCurrentValues();
}

bool TweenAnimatorImpl::update(float deltaTimeMs) {
    if (m_state != AnimationState::Running) {
        return m_state != AnimationState::Completed;
    }

    m_elapsedTime += deltaTimeMs;

    // Handle delay
    if (m_elapsedTime < m_delay) {
        return true;
    }

    float activeTime = m_elapsedTime - m_delay;

    // Calculate progress
    float rawProgress = m_duration > 0.0f ? activeTime / m_duration : 1.0f;

    // Handle looping
    if (rawProgress >= 1.0f) {
        m_currentLoop++;

        if (m_loopCount == -1 || m_currentLoop < m_loopCount) {
            // Continue looping
            if (m_yoyo) {
                m_reversed = !m_reversed;
            }
            m_elapsedTime = m_delay;
            rawProgress = 0.0f;
        } else {
            // Animation complete
            rawProgress = 1.0f;
            m_progress = m_reversed ? 0.0f : 1.0f;

            // Apply final values
            float easedProgress = applyEasing(m_easing, m_progress);
            for (auto& prop : m_properties) {
                prop.currentValue = lerp(prop.fromValue, prop.toValue, easedProgress);
            }
            applyCurrentValues();

            stop();
            return false;
        }
    }

    // Apply yoyo reversal
    m_progress = m_reversed ? (1.0f - rawProgress) : rawProgress;

    // Apply easing and interpolate
    float easedProgress = applyEasing(m_easing, m_progress);
    for (auto& prop : m_properties) {
        prop.currentValue = lerp(prop.fromValue, prop.toValue, easedProgress);
    }

    // Apply values to widget
    applyCurrentValues();

    if (m_onUpdate) {
        m_onUpdate(m_progress);
    }

    return true;
}

bool TweenAnimatorImpl::isRunning() const {
    return m_state == AnimationState::Running;
}

bool TweenAnimatorImpl::isCompleted() const {
    return m_state == AnimationState::Completed;
}

float TweenAnimatorImpl::getProgress() const {
    return m_progress;
}

void TweenAnimatorImpl::applyCurrentValues() {
    if (!m_widget) return;

    for (const auto& prop : m_properties) {
        setWidgetPropertyValue(*m_widget, prop.property, prop.currentValue);
    }
}


} // namespace KillerGK
