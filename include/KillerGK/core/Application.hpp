/**
 * @file Application.hpp
 * @brief Application class for KillerGK
 */

#pragma once

#include <string>
#include <functional>
#include <memory>

namespace KillerGK {

// Forward declarations
class Theme;
class ResourceManager;
struct PerformanceStats;

/**
 * @class Application
 * @brief Main application class using Builder Pattern
 */
class Application {
public:
    /**
     * @brief Get the singleton application instance
     */
    static Application& instance();

    // Builder Pattern methods
    Application& title(const std::string& title);
    Application& size(int width, int height);
    Application& theme(const Theme& theme);
    Application& vsync(bool enabled);

    /**
     * @brief Run the application with a setup callback
     */
    void run(std::function<void()> setup);

    /**
     * @brief Quit the application
     */
    void quit();

    /**
     * @brief Get the resource manager
     */
    ResourceManager& resources();

    /**
     * @brief Get performance statistics
     */
    PerformanceStats stats() const;

private:
    Application();
    ~Application();
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

/**
 * @struct PerformanceStats
 * @brief Performance statistics
 */
struct PerformanceStats {
    float fps = 0.0f;
    float frameTime = 0.0f;
    float cpuTime = 0.0f;
    float gpuTime = 0.0f;
    size_t memoryUsage = 0;
    size_t gpuMemoryUsage = 0;
    int drawCalls = 0;
    int triangles = 0;
    int textureBinds = 0;
};

} // namespace KillerGK
