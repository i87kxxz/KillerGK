/**
 * @file KGKAudio.cpp
 * @brief Audio module implementation using miniaudio
 */

#define MINIAUDIO_IMPLEMENTATION
#include "../../external/miniaudio/miniaudio.h"

#include "KillerGK/kgkaudio/KGKAudio.hpp"
#include <unordered_map>
#include <mutex>
#include <cmath>
#include <algorithm>
#include <atomic>

namespace KGKAudio {

// ============================================================================
// Audio Effect Implementation
// ============================================================================

class AudioEffectImpl {
public:
    EffectType type = EffectType::None;
    bool enabled = true;
    
    ReverbParams reverbParams;
    EchoParams echoParams;
    EqualizerParams eqParams;
    
    // Echo delay buffer
    std::vector<float> echoBuffer;
    size_t echoWritePos = 0;
    
    // Process audio through effect
    void process(float* data, uint32_t frameCount, uint32_t channels, uint32_t sampleRate) {
        if (!enabled) return;
        
        switch (type) {
            case EffectType::Reverb:
                processReverb(data, frameCount, channels, sampleRate);
                break;
            case EffectType::Echo:
                processEcho(data, frameCount, channels, sampleRate);
                break;
            case EffectType::Equalizer:
                processEqualizer(data, frameCount, channels, sampleRate);
                break;
            default:
                break;
        }
    }
    
private:
    void processReverb(float* data, uint32_t frameCount, uint32_t channels, uint32_t sampleRate) {
        // Simple reverb using comb filters
        const float decay = reverbParams.decay;
        const float gain = reverbParams.gain;
        
        static std::vector<float> reverbBuffer;
        static size_t reverbPos = 0;
        
        size_t delaySize = static_cast<size_t>(sampleRate * decay * 0.1f);
        if (reverbBuffer.size() != delaySize * channels) {
            reverbBuffer.resize(delaySize * channels, 0.0f);
            reverbPos = 0;
        }
        
        for (uint32_t i = 0; i < frameCount * channels; ++i) {
            float delayed = reverbBuffer[reverbPos];
            float input = data[i];
            
            reverbBuffer[reverbPos] = input + delayed * decay * 0.5f;
            data[i] = input + delayed * gain;
            
            reverbPos = (reverbPos + 1) % reverbBuffer.size();
        }
    }
    
    void processEcho(float* data, uint32_t frameCount, uint32_t channels, uint32_t sampleRate) {
        size_t delaySamples = static_cast<size_t>(echoParams.delay * sampleRate * channels);
        
        if (echoBuffer.size() != delaySamples) {
            echoBuffer.resize(delaySamples, 0.0f);
            echoWritePos = 0;
        }
        
        for (uint32_t i = 0; i < frameCount * channels; ++i) {
            size_t readPos = (echoWritePos + 1) % echoBuffer.size();
            float delayed = echoBuffer[readPos];
            float input = data[i];
            
            echoBuffer[echoWritePos] = input + delayed * echoParams.decay;
            data[i] = input * (1.0f - echoParams.wetDryMix) + delayed * echoParams.wetDryMix;
            
            echoWritePos = (echoWritePos + 1) % echoBuffer.size();
        }
    }
    
    void processEqualizer(float* data, uint32_t frameCount, uint32_t channels, uint32_t sampleRate) {
        // Simple gain-based EQ (proper implementation would use biquad filters)
        // For now, apply overall gain based on band settings
        float totalGain = 0.0f;
        for (const auto& band : eqParams.bands) {
            totalGain += band.gain;
        }
        totalGain = totalGain / static_cast<float>(eqParams.bands.size());
        float linearGain = std::pow(10.0f, totalGain / 20.0f);
        
        for (uint32_t i = 0; i < frameCount * channels; ++i) {
            data[i] *= linearGain;
            data[i] = std::clamp(data[i], -1.0f, 1.0f);
        }
    }
};

// ============================================================================
// Audio Effect
// ============================================================================

AudioEffect::AudioEffect() : m_impl(std::make_shared<AudioEffectImpl>()) {}

AudioEffect AudioEffect::reverb(const ReverbParams& params) {
    AudioEffect effect;
    effect.m_impl->type = EffectType::Reverb;
    effect.m_impl->reverbParams = params;
    return effect;
}

AudioEffect AudioEffect::reverb(ReverbPreset preset) {
    ReverbParams params;
    switch (preset) {
        case ReverbPreset::SmallRoom:
            params = {0.3f, 0.3f, 0.7f, 0.3f};
            break;
        case ReverbPreset::MediumRoom:
            params = {0.6f, 0.5f, 0.6f, 0.4f};
            break;
        case ReverbPreset::LargeRoom:
            params = {1.0f, 0.6f, 0.5f, 0.5f};
            break;
        case ReverbPreset::Hall:
            params = {1.5f, 0.7f, 0.4f, 0.5f};
            break;
        case ReverbPreset::Cathedral:
            params = {3.0f, 0.8f, 0.3f, 0.6f};
            break;
        case ReverbPreset::Cave:
            params = {2.0f, 0.9f, 0.2f, 0.7f};
            break;
        default:
            break;
    }
    return reverb(params);
}

AudioEffect AudioEffect::echo(const EchoParams& params) {
    AudioEffect effect;
    effect.m_impl->type = EffectType::Echo;
    effect.m_impl->echoParams = params;
    return effect;
}

AudioEffect AudioEffect::equalizer(const EqualizerParams& params) {
    AudioEffect effect;
    effect.m_impl->type = EffectType::Equalizer;
    effect.m_impl->eqParams = params;
    return effect;
}

EffectType AudioEffect::type() const { return m_impl->type; }
bool AudioEffect::isEnabled() const { return m_impl->enabled; }
void AudioEffect::setEnabled(bool enabled) { m_impl->enabled = enabled; }

void AudioEffect::setReverbParams(const ReverbParams& params) { m_impl->reverbParams = params; }
ReverbParams AudioEffect::getReverbParams() const { return m_impl->reverbParams; }

void AudioEffect::setEchoParams(const EchoParams& params) { m_impl->echoParams = params; }
EchoParams AudioEffect::getEchoParams() const { return m_impl->echoParams; }

void AudioEffect::setEqualizerParams(const EqualizerParams& params) { m_impl->eqParams = params; }
EqualizerParams AudioEffect::getEqualizerParams() const { return m_impl->eqParams; }

void AudioEffect::setBandGain(int band, float gain) {
    if (band >= 0 && band < static_cast<int>(m_impl->eqParams.bands.size())) {
        m_impl->eqParams.bands[band].gain = std::clamp(gain, -12.0f, 12.0f);
    }
}

// ============================================================================
// Audio Engine Implementation
// ============================================================================

// Forward declaration for friend access
class AudioEngineImpl;
static AudioEngineImpl* g_engineImpl = nullptr;

class AudioEngineImpl {
public:
    ma_engine engine;
    bool initialized = false;
    float masterVolume = 1.0f;
    std::mutex mutex;
    
    ma_engine* getEngine() { return initialized ? &engine : nullptr; }
    
    bool init() {
        ma_engine_config config = ma_engine_config_init();
        config.channels = 2;
        config.sampleRate = 44100;
        
        ma_result result = ma_engine_init(&config, &engine);
        if (result != MA_SUCCESS) {
            return false;
        }
        
        initialized = true;
        g_engineImpl = this;
        return true;
    }
    
    void uninit() {
        if (initialized) {
            ma_engine_uninit(&engine);
            initialized = false;
            g_engineImpl = nullptr;
        }
    }
    
    uint32_t getSampleRate() const {
        return initialized ? ma_engine_get_sample_rate(&engine) : 44100;
    }
    
    uint32_t getChannels() const {
        return initialized ? ma_engine_get_channels(&engine) : 2;
    }
};

// ============================================================================
// Audio Implementation
// ============================================================================

class AudioImpl {
public:
    ma_sound sound;
    bool soundInitialized = false;
    std::string filePath;
    AudioFormat audioFormat = AudioFormat::Unknown;
    
    // Properties
    float volume = 1.0f;
    float pitch = 1.0f;
    float pan = 0.0f;
    bool looping = false;
    bool spatialized = false;
    
    // 3D audio
    AudioPosition position;
    AudioVelocity velocity;
    float minDistance = 1.0f;
    float maxDistance = 100.0f;
    float rolloffFactor = 1.0f;
    
    // Effects
    std::vector<std::shared_ptr<AudioEffectImpl>> effects;
    
    // Callbacks
    std::function<void()> onEndCallback;
    std::function<void()> onLoopCallback;
    
    // Fade
    float fadeInDuration = 0.0f;
    float fadeOutDuration = 0.0f;
    
    ~AudioImpl() {
        if (soundInitialized) {
            ma_sound_uninit(&sound);
        }
    }
    
    bool loadFromFile(const std::string& path, ma_engine* enginePtr) {
        if (!enginePtr) {
            return false;
        }
        
        filePath = path;
        audioFormat = detectFormat(path);
        
        ma_uint32 flags = 0;
        if (!spatialized) {
            flags |= MA_SOUND_FLAG_NO_SPATIALIZATION;
        }
        
        ma_result result = ma_sound_init_from_file(
            enginePtr,
            path.c_str(),
            flags,
            nullptr,
            nullptr,
            &sound
        );
        
        if (result != MA_SUCCESS) {
            return false;
        }
        
        soundInitialized = true;
        applyProperties();
        return true;
    }
    
    void applyProperties() {
        if (!soundInitialized) return;
        
        ma_sound_set_volume(&sound, volume);
        ma_sound_set_pitch(&sound, pitch);
        ma_sound_set_pan(&sound, pan);
        ma_sound_set_looping(&sound, looping);
        
        if (spatialized) {
            ma_sound_set_position(&sound, position.x, position.y, position.z);
            ma_sound_set_velocity(&sound, velocity.x, velocity.y, velocity.z);
            ma_sound_set_min_distance(&sound, minDistance);
            ma_sound_set_max_distance(&sound, maxDistance);
            ma_sound_set_rolloff(&sound, rolloffFactor);
        }
        
        if (fadeInDuration > 0.0f) {
            ma_sound_set_fade_in_milliseconds(&sound, 0.0f, volume, 
                static_cast<ma_uint64>(fadeInDuration * 1000));
        }
    }
    
    AudioFormat detectFormat(const std::string& path) {
        size_t dotPos = path.rfind('.');
        if (dotPos == std::string::npos) return AudioFormat::Unknown;
        
        std::string ext = path.substr(dotPos + 1);
        for (auto& c : ext) c = static_cast<char>(std::tolower(c));
        
        if (ext == "wav") return AudioFormat::WAV;
        if (ext == "mp3") return AudioFormat::MP3;
        if (ext == "ogg") return AudioFormat::OGG;
        if (ext == "flac") return AudioFormat::FLAC;
        
        return AudioFormat::Unknown;
    }
    
    void play() {
        if (soundInitialized) {
            ma_sound_start(&sound);
        }
    }
    
    void pause() {
        if (soundInitialized) {
            ma_sound_stop(&sound);
        }
    }
    
    void stop() {
        if (soundInitialized) {
            ma_sound_stop(&sound);
            ma_sound_seek_to_pcm_frame(&sound, 0);
        }
    }
    
    void seek(float seconds) {
        if (soundInitialized) {
            auto& engine = AudioEngine::instance();
            uint32_t sampleRate = engine.getSampleRate();
            ma_uint64 frame = static_cast<ma_uint64>(seconds * sampleRate);
            ma_sound_seek_to_pcm_frame(&sound, frame);
        }
    }
    
    bool isPlaying() const {
        return soundInitialized && ma_sound_is_playing(&sound);
    }
    
    float getDuration() const {
        if (!soundInitialized) return 0.0f;
        
        float length = 0.0f;
        ma_sound_get_length_in_seconds(&sound, &length);
        return length;
    }
    
    float getCurrentTime() const {
        if (!soundInitialized) return 0.0f;
        
        float cursor = 0.0f;
        ma_sound_get_cursor_in_seconds(&sound, &cursor);
        return cursor;
    }
};

// ============================================================================
// Audio
// ============================================================================

struct Audio::Impl {
    std::shared_ptr<AudioImpl> audioImpl;
    
    Impl() : audioImpl(std::make_shared<AudioImpl>()) {}
};

Audio::Audio() : m_impl(std::make_shared<Impl>()) {}

Audio Audio::load(const std::string& path) {
    Audio audio;
    if (g_engineImpl) {
        audio.m_impl->audioImpl->loadFromFile(path, g_engineImpl->getEngine());
    }
    return audio;
}

Audio Audio::fromBuffer(const AudioBuffer& buffer) {
    Audio audio;
    // Buffer-based audio would require custom decoding
    // For now, store the buffer data
    audio.m_impl->audioImpl->audioFormat = AudioFormat::Unknown;
    return audio;
}

void Audio::play() { m_impl->audioImpl->play(); }
void Audio::pause() { m_impl->audioImpl->pause(); }
void Audio::stop() { m_impl->audioImpl->stop(); }
void Audio::seek(float seconds) { m_impl->audioImpl->seek(seconds); }

Audio& Audio::volume(float vol) {
    m_impl->audioImpl->volume = std::clamp(vol, 0.0f, 1.0f);
    if (m_impl->audioImpl->soundInitialized) {
        ma_sound_set_volume(&m_impl->audioImpl->sound, m_impl->audioImpl->volume);
    }
    return *this;
}

Audio& Audio::pitch(float p) {
    m_impl->audioImpl->pitch = std::clamp(p, 0.1f, 4.0f);
    if (m_impl->audioImpl->soundInitialized) {
        ma_sound_set_pitch(&m_impl->audioImpl->sound, m_impl->audioImpl->pitch);
    }
    return *this;
}

Audio& Audio::pan(float p) {
    m_impl->audioImpl->pan = std::clamp(p, -1.0f, 1.0f);
    if (m_impl->audioImpl->soundInitialized) {
        ma_sound_set_pan(&m_impl->audioImpl->sound, m_impl->audioImpl->pan);
    }
    return *this;
}

Audio& Audio::loop(bool enabled) {
    m_impl->audioImpl->looping = enabled;
    if (m_impl->audioImpl->soundInitialized) {
        ma_sound_set_looping(&m_impl->audioImpl->sound, enabled);
    }
    return *this;
}

Audio& Audio::fadeIn(float seconds) {
    m_impl->audioImpl->fadeInDuration = seconds;
    return *this;
}

Audio& Audio::fadeOut(float seconds) {
    m_impl->audioImpl->fadeOutDuration = seconds;
    return *this;
}

Audio& Audio::position(float x, float y, float z) {
    m_impl->audioImpl->position = {x, y, z};
    m_impl->audioImpl->spatialized = true;
    if (m_impl->audioImpl->soundInitialized) {
        ma_sound_set_position(&m_impl->audioImpl->sound, x, y, z);
    }
    return *this;
}

Audio& Audio::velocity(float x, float y, float z) {
    m_impl->audioImpl->velocity = {x, y, z};
    if (m_impl->audioImpl->soundInitialized) {
        ma_sound_set_velocity(&m_impl->audioImpl->sound, x, y, z);
    }
    return *this;
}

Audio& Audio::minDistance(float distance) {
    m_impl->audioImpl->minDistance = distance;
    if (m_impl->audioImpl->soundInitialized) {
        ma_sound_set_min_distance(&m_impl->audioImpl->sound, distance);
    }
    return *this;
}

Audio& Audio::maxDistance(float distance) {
    m_impl->audioImpl->maxDistance = distance;
    if (m_impl->audioImpl->soundInitialized) {
        ma_sound_set_max_distance(&m_impl->audioImpl->sound, distance);
    }
    return *this;
}

Audio& Audio::rolloff(float factor) {
    m_impl->audioImpl->rolloffFactor = factor;
    if (m_impl->audioImpl->soundInitialized) {
        ma_sound_set_rolloff(&m_impl->audioImpl->sound, factor);
    }
    return *this;
}

Audio& Audio::spatialize(bool enabled) {
    m_impl->audioImpl->spatialized = enabled;
    if (m_impl->audioImpl->soundInitialized) {
        ma_sound_set_spatialization_enabled(&m_impl->audioImpl->sound, enabled);
    }
    return *this;
}

Audio& Audio::addEffect(const AudioEffect& effect) {
    m_impl->audioImpl->effects.push_back(effect.m_impl);
    return *this;
}

Audio& Audio::removeEffect(EffectType type) {
    auto& effects = m_impl->audioImpl->effects;
    effects.erase(
        std::remove_if(effects.begin(), effects.end(),
            [type](const auto& e) { return e->type == type; }),
        effects.end()
    );
    return *this;
}

Audio& Audio::clearEffects() {
    m_impl->audioImpl->effects.clear();
    return *this;
}

bool Audio::isPlaying() const { return m_impl->audioImpl->isPlaying(); }
bool Audio::isPaused() const { 
    return m_impl->audioImpl->soundInitialized && 
           !m_impl->audioImpl->isPlaying() && 
           m_impl->audioImpl->getCurrentTime() > 0.0f;
}
bool Audio::isStopped() const { return !isPlaying() && !isPaused(); }

AudioState Audio::state() const {
    if (isPlaying()) return AudioState::Playing;
    if (isPaused()) return AudioState::Paused;
    return AudioState::Stopped;
}

float Audio::duration() const { return m_impl->audioImpl->getDuration(); }
float Audio::currentTime() const { return m_impl->audioImpl->getCurrentTime(); }
float Audio::getVolume() const { return m_impl->audioImpl->volume; }
float Audio::getPitch() const { return m_impl->audioImpl->pitch; }
bool Audio::isLooping() const { return m_impl->audioImpl->looping; }
bool Audio::isValid() const { return m_impl->audioImpl->soundInitialized; }
AudioFormat Audio::format() const { return m_impl->audioImpl->audioFormat; }
std::string Audio::path() const { return m_impl->audioImpl->filePath; }

Audio& Audio::onEnd(std::function<void()> callback) {
    m_impl->audioImpl->onEndCallback = std::move(callback);
    return *this;
}

Audio& Audio::onLoop(std::function<void()> callback) {
    m_impl->audioImpl->onLoopCallback = std::move(callback);
    return *this;
}

AudioHandle Audio::build() {
    return m_impl->audioImpl;
}

// ============================================================================
// AudioListener
// ============================================================================

struct AudioListener::Impl {
    AudioPosition position;
    AudioVelocity velocity;
    float forwardX = 0, forwardY = 0, forwardZ = -1;
    float upX = 0, upY = 1, upZ = 0;
};

AudioListener::AudioListener() : m_impl(std::make_unique<Impl>()) {}
AudioListener::~AudioListener() = default;

AudioListener& AudioListener::instance() {
    static AudioListener listener;
    return listener;
}

void AudioListener::setPosition(float x, float y, float z) {
    m_impl->position = {x, y, z};
    
    if (g_engineImpl && g_engineImpl->initialized) {
        ma_engine_listener_set_position(&g_engineImpl->engine, 0, x, y, z);
    }
}

void AudioListener::setVelocity(float x, float y, float z) {
    m_impl->velocity = {x, y, z};
    
    if (g_engineImpl && g_engineImpl->initialized) {
        ma_engine_listener_set_velocity(&g_engineImpl->engine, 0, x, y, z);
    }
}

void AudioListener::setOrientation(float forwardX, float forwardY, float forwardZ,
                                   float upX, float upY, float upZ) {
    m_impl->forwardX = forwardX;
    m_impl->forwardY = forwardY;
    m_impl->forwardZ = forwardZ;
    m_impl->upX = upX;
    m_impl->upY = upY;
    m_impl->upZ = upZ;
    
    if (g_engineImpl && g_engineImpl->initialized) {
        ma_engine_listener_set_direction(&g_engineImpl->engine, 0, forwardX, forwardY, forwardZ);
        ma_engine_listener_set_world_up(&g_engineImpl->engine, 0, upX, upY, upZ);
    }
}

AudioPosition AudioListener::getPosition() const { return m_impl->position; }
AudioVelocity AudioListener::getVelocity() const { return m_impl->velocity; }

// ============================================================================
// AudioEngine
// ============================================================================

AudioEngine::AudioEngine() : m_impl(std::make_unique<AudioEngineImpl>()) {}
AudioEngine::~AudioEngine() { shutdown(); }

AudioEngine& AudioEngine::instance() {
    static AudioEngine engine;
    return engine;
}

bool AudioEngine::initialize() {
    return m_impl->init();
}

void AudioEngine::shutdown() {
    m_impl->uninit();
}

bool AudioEngine::isInitialized() const {
    return m_impl->initialized;
}

void AudioEngine::setMasterVolume(float volume) {
    m_impl->masterVolume = std::clamp(volume, 0.0f, 1.0f);
    if (m_impl->initialized) {
        ma_engine_set_volume(&m_impl->engine, m_impl->masterVolume);
    }
}

float AudioEngine::getMasterVolume() const {
    return m_impl->masterVolume;
}

void AudioEngine::pauseAll() {
    // miniaudio doesn't have a direct pause all, but we can stop the device
    if (m_impl->initialized) {
        ma_engine_stop(&m_impl->engine);
    }
}

void AudioEngine::resumeAll() {
    if (m_impl->initialized) {
        ma_engine_start(&m_impl->engine);
    }
}

void AudioEngine::stopAll() {
    // Stop all sounds - would need to track all sounds
    // For now, just stop the engine
    if (m_impl->initialized) {
        ma_engine_stop(&m_impl->engine);
    }
}

uint32_t AudioEngine::getSampleRate() const {
    return m_impl->getSampleRate();
}

uint32_t AudioEngine::getChannels() const {
    return m_impl->getChannels();
}

void AudioEngine::update() {
    // Update 3D audio calculations - miniaudio handles this internally
}

// ============================================================================
// Convenience Functions
// ============================================================================

bool initialize() {
    return AudioEngine::instance().initialize();
}

void shutdown() {
    AudioEngine::instance().shutdown();
}

bool isInitialized() {
    return AudioEngine::instance().isInitialized();
}

} // namespace KGKAudio
