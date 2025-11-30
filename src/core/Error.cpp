/**
 * @file Error.cpp
 * @brief Implementation of error handling
 */

#include "KillerGK/core/Error.hpp"
#include <iostream>
#include <mutex>

namespace KillerGK {

namespace {
    ErrorCallback g_errorCallback;
    LogLevel g_logLevel = LogLevel::Info;
    std::mutex g_logMutex;
}

void setErrorCallback(ErrorCallback callback) {
    g_errorCallback = std::move(callback);
}

void setLogLevel(LogLevel level) {
    g_logLevel = level;
}

void log(LogLevel level, const std::string& message) {
    if (level < g_logLevel) {
        return;
    }

    std::lock_guard<std::mutex> lock(g_logMutex);

    const char* levelStr = "";
    switch (level) {
        case LogLevel::Debug:   levelStr = "[DEBUG]"; break;
        case LogLevel::Info:    levelStr = "[INFO]"; break;
        case LogLevel::Warning: levelStr = "[WARN]"; break;
        case LogLevel::Error:   levelStr = "[ERROR]"; break;
        case LogLevel::Fatal:   levelStr = "[FATAL]"; break;
    }

    std::cerr << levelStr << " " << message << std::endl;
}

} // namespace KillerGK
