/**
 * @file KGKAudio.hpp
 * @brief Audio module for KillerGK
 */

#pragma once

#include <string>
#include <memory>

namespace KGKAudio {

/**
 * @brief Handle to an audio source
 */
using AudioHandle = std::shared_ptr<class AudioImpl>;

/**
 * @class Audio
 * @brief Audio playback class
 */
class Audio {
public:
    /**
     * @brief Load audio from file
     * @param path Path to audio file (WAV, MP3, OGG, FLAC)
     */
    static Audio load(const std::string& path);

    // Playback controls
    void play();
    void pause();
    void stop();
    void seek(float seconds);

    // Properties
    Audio& volume(float volume);
    Audio& pitch(float pitch);
    Audio& loop(bool enabled);

    // 3D audio
    Audio& position(float x, float y, float z);
    Audio& velocity(float x, float y, float z);

    // State
    [[nodiscard]] bool isPlaying() const;
    [[nodiscard]] float duration() const;
    [[nodiscard]] float currentTime() const;

    AudioHandle build();

private:
    Audio();
    struct Impl;
    std::shared_ptr<Impl> m_impl;
};

/**
 * @class AudioListener
 * @brief 3D audio listener (usually the camera)
 */
class AudioListener {
public:
    static AudioListener& instance();

    void setPosition(float x, float y, float z);
    void setOrientation(float forwardX, float forwardY, float forwardZ,
                        float upX, float upY, float upZ);

private:
    AudioListener();
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

/**
 * @brief Initialize the audio system
 */
bool initialize();

/**
 * @brief Shutdown the audio system
 */
void shutdown();

} // namespace KGKAudio
