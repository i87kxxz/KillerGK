/**
 * @file Platform.cpp
 * @brief Platform factory and common functionality for KillerGK
 */

#include "KillerGK/platform/Platform.hpp"

#ifdef _WIN32
#include "KillerGK/platform/WindowsPlatform.hpp"
#elif defined(__linux__) && !defined(__ANDROID__)
#include "KillerGK/platform/LinuxPlatform.hpp"
#elif defined(__APPLE__)
#include "KillerGK/platform/MacOSPlatform.hpp"
#endif

#include <memory>
#include <stdexcept>

namespace KillerGK {

// Global platform instance
static std::unique_ptr<IPlatform> s_platformInstance;

std::unique_ptr<IPlatform> createPlatform() {
#ifdef _WIN32
    return std::make_unique<WindowsPlatform>();
#elif defined(__linux__) && !defined(__ANDROID__)
    return std::make_unique<LinuxPlatform>();
#elif defined(__APPLE__)
    return std::make_unique<MacOSPlatform>();
#else
    throw std::runtime_error("Unsupported platform");
#endif
}

IPlatform& getPlatform() {
    if (!s_platformInstance) {
        s_platformInstance = createPlatform();
        if (!s_platformInstance->initialize()) {
            throw std::runtime_error("Failed to initialize platform");
        }
    }
    return *s_platformInstance;
}

} // namespace KillerGK
