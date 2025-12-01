/**
 * @file KGKAudio.hpp
 * @brief Audio module for KillerGK - Complete audio playback, 3D audio, and effects
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <cstdint>

namespace KGKAudio {

// Forward declarations
class AudioImpl;
class AudioEngineImpl;
class AudioEffectImpl;

/**
 * @brief Handle to an audio source
 */
using AudioHandle = std::shared_ptr<AudioImpl>;

/**
 * @brief Audio format enumeration
 */
enum class AudioFormat {
    Unknown,
    WAV,
    MP3,
    OGG,
    FLAC
};

/**
 * @brief Audio state enumeration
 */
enum class AudioState {
    Stopped,
    Playing,
    Paused
};

/**
 * @brief Audio effect type enumeration
 */
enum class EffectType {
    None,
    Reverb,
    Echo,
    Equalizer
};

/**
 * @brief Reverb preset enumeration
 */
enum class ReverbPreset {
    None,
    SmallRoom,
    MediumRoom,
    LargeRoom,
    Hall,
    Cathedral,
    Cave
};

/**
 * @brief Audio buffer for raw audio data
 */
struct AudioBuffer {
    std::vector<float> data;
    uint32_t sampleRate = 44100;
    uint32_t channels = 2;
    float duration = 0.0f;
};

/**
 * @brief 3D audio position
 */
struct AudioPosition {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

/**
 * @brief 3D audio velocity
 */
struct AudioVelocity {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

/**
 * @brief Reverb effect parameters
 */
struct ReverbParams {
    float decay = 1.0f;        // Decay time in seconds (0.1 - 10.0)
    float density = 0.5f;      // Reverb density (0.0 - 1.0)
    float bandwidth = 0.5f;    // High frequency damping (0.0 - 1.0)
    float gain = 0.5f;         // Output gain (0.0 - 1.0)
};

/**
 * @brief Echo effect parameters
 */
struct EchoParams {
    float delay = 0.3f;        // Delay time in seconds (0.01 - 2.0)
    float decay = 0.5f;        // Decay factor (0.0 - 1.0)
    float wetDryMix = 0.5f;    // Wet/dry mix (0.0 - 1.0)
};

/**
 * @brief Equalizer band
 */
struct EqualizerBand {
    float frequency = 1000.0f; // Center frequency in Hz
    float gain = 0.0f;         // Gain in dB (-12 to +12)
    float bandwidth = 1.0f;    // Bandwidth in octaves
};

/**
 * @brief Equalizer parameters (10-band)
 */
struct EqualizerParams {
    std::vector<EqualizerBand> bands;
    
    EqualizerParams() {
        // Default 10-band equalizer frequencies
        float frequencies[] = {32, 64, 125, 250, 500, 1000, 2000, 4000, 8000, 16000};
        for (int i = 0; i < 10; ++i) {
            bands.push_back({frequencies[i], 0.0f, 1.0f});
        }
    }
};

/**
 * @class AudioEffect
 * @brief Audio effect processor
 */
class AudioEffect {
public:
    static AudioEffect reverb(const ReverbParams& params = {});
    static AudioEffect reverb(ReverbPreset preset);
    static AudioEffect echo(const EchoParams& params = {});
    static AudioEffect equalizer(const EqualizerParams& params = {});
    
    EffectType type() const;
    bool isEnabled() const;
    void setEnabled(bool enabled);
    
    // Reverb-specific
    void setReverbParams(const ReverbParams& params);
    ReverbParams getReverbParams() const;
    
    // Echo-specific
    void setEchoParams(const EchoParams& params);
    EchoParams getEchoParams() const;
    
    // Equalizer-specific
    void setEqualizerParams(const EqualizerParams& params);
    EqualizerParams getEqualizerParams() const;
    void setBandGain(int band, float gain);
    
private:
    AudioEffect();
    std::shared_ptr<AudioEffectImpl> m_impl;
    friend class Audio;
    friend class AudioEngine;
};

/**
 * @class Audio
 * @brief Audio playback class with Builder Pattern
 */
class Audio {
public:
    /**
     * @brief Load audio from file
     * @param path Path to audio file (WAV, MP3, OGG, FLAC)
     * @return Audio object for chaining
     */
    static Audio load(const std::string& path);
    
    /**
     * @brief Create audio from buffer
     * @param buffer Audio buffer with raw data
     * @return Audio object for chaining
     */
    static Audio fromBuffer(const AudioBuffer& buffer);

    // Playback controls
    void play();
    void pause();
    void stop();
    void seek(float seconds);
    
    // Builder pattern properties
    Audio& volume(float volume);
    Audio& pitch(float pitch);
    Audio& pan(float pan);  // -1.0 (left) to 1.0 (right)
    Audio& loop(bool enabled);
    Audio& fadeIn(float seconds);
    Audio& fadeOut(float seconds);

    // 3D audio
    Audio& position(float x, float y, float z);
    Audio& velocity(float x, float y, float z);
    Audio& minDistance(float distance);
    Audio& maxDistance(float distance);
    Audio& rolloff(float factor);
    Audio& spatialize(bool enabled);
    
    // Effects
    Audio& addEffect(const AudioEffect& effect);
    Audio& removeEffect(EffectType type);
    Audio& clearEffects();

    // State queries
    [[nodiscard]] bool isPlaying() const;
    [[nodiscard]] bool isPaused() const;
    [[nodiscard]] bool isStopped() const;
    [[nodiscard]] AudioState state() const;
    [[nodiscard]] float duration() const;
    [[nodiscard]] float currentTime() const;
    [[nodiscard]] float getVolume() const;
    [[nodiscard]] float getPitch() const;
    [[nodiscard]] bool isLooping() const;
    [[nodiscard]] bool isValid() const;
    [[nodiscard]] AudioFormat format() const;
    [[nodiscard]] std::string path() const;
    
    // Callbacks
    Audio& onEnd(std::function<void()> callback);
    Audio& onLoop(std::function<void()> callback);

    /**
     * @brief Build and return the audio handle
     * @return Shared pointer to audio implementation
     */
    AudioHandle build();

private:
    Audio();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
    friend class AudioEngine;
};

/**
 * @class AudioListener
 * @brief 3D audio listener (usually the camera/player position)
 */
class AudioListener {
public:
    static AudioListener& instance();

    void setPosition(float x, float y, float z);
    void setVelocity(float x, float y, float z);
    void setOrientation(float forwardX, float forwardY, float forwardZ,
                        float upX, float upY, float upZ);
    
    AudioPosition getPosition() const;
    AudioVelocity getVelocity() const;

private:
    AudioListener();
    ~AudioListener();
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

/**
 * @class AudioEngine
 * @brief Main audio engine singleton
 */
class AudioEngine {
public:
    static AudioEngine& instance();
    
    /**
     * @brief Initialize the audio engine
     * @return true if initialization succeeded
     */
    bool initialize();
    
    /**
     * @brief Shutdown the audio engine
     */
    void shutdown();
    
    /**
     * @brief Check if engine is initialized
     */
    [[nodiscard]] bool isInitialized() const;
    
    /**
     * @brief Set master volume
     * @param volume Volume level (0.0 - 1.0)
     */
    void setMasterVolume(float volume);
    
    /**
     * @brief Get master volume
     */
    [[nodiscard]] float getMasterVolume() const;
    
    /**
     * @brief Pause all audio
     */
    void pauseAll();
    
    /**
     * @brief Resume all audio
     */
    void resumeAll();
    
    /**
     * @brief Stop all audio
     */
    void stopAll();
    
    /**
     * @brief Get sample rate
     */
    [[nodiscard]] uint32_t getSampleRate() const;
    
    /**
     * @brief Get number of channels
     */
    [[nodiscard]] uint32_t getChannels() const;
    
    /**
     * @brief Update audio engine (call each frame for 3D audio updates)
     */
    void update();

private:
    AudioEngine();
    ~AudioEngine();
    std::unique_ptr<AudioEngineImpl> m_impl;
};

// Convenience functions
/**
 * @brief Initialize the audio system
 * @return true if initialization succeeded
 */
bool initialize();

/**
 * @brief Shutdown the audio system
 */
void shutdown();

/**
 * @brief Check if audio system is initialized
 */
bool isInitialized();

} // namespace KGKAudio
