/**
 * @file KGKMedia.cpp
 * @brief Media module implementation - video playback, screenshot capture, and image processing
 */

// Prevent Windows min/max macros from conflicting with std::min/max
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "KillerGK/kgkmedia/KGKMedia.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>

// stb_image for loading
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// stb_image_write for saving
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

namespace KGKMedia {

// ============================================================================
// Video Implementation
// ============================================================================

struct Video::Impl {
    std::string path;
    float volumeLevel = 1.0f;
    bool looping = false;
    float rate = 1.0f;
    VideoState currentState = VideoState::Stopped;
    float videoDuration = 0.0f;
    float currentTimePos = 0.0f;
    int videoWidth = 0;
    int videoHeight = 0;
    float fps = 30.0f;
    bool valid = false;
    
    VideoFrame currentFrame;
    
    // Callbacks
    FrameCallback frameCallback;
    StateCallback stateCallback;
    std::function<void()> endCallback;
    
    void setState(VideoState newState) {
        if (currentState != newState) {
            currentState = newState;
            if (stateCallback) {
                stateCallback(newState);
            }
        }
    }
};

Video::Video() : m_impl(std::make_shared<Impl>()) {}

Video Video::load(const std::string& path) {
    Video video;
    video.m_impl->path = path;
    
    // Check if file exists
    std::ifstream file(path, std::ios::binary);
    if (file.good()) {
        video.m_impl->valid = true;
        // In a real implementation, we would use a video decoder library
        // like FFmpeg to extract video metadata
        // For now, set some default values
        video.m_impl->videoWidth = 1920;
        video.m_impl->videoHeight = 1080;
        video.m_impl->fps = 30.0f;
        video.m_impl->videoDuration = 0.0f;  // Would be extracted from video
        
        // Initialize current frame
        video.m_impl->currentFrame.width = video.m_impl->videoWidth;
        video.m_impl->currentFrame.height = video.m_impl->videoHeight;
        video.m_impl->currentFrame.data.resize(
            static_cast<size_t>(video.m_impl->videoWidth) * 
            static_cast<size_t>(video.m_impl->videoHeight) * 4, 0);
        video.m_impl->currentFrame.valid = true;
    }
    
    return video;
}

bool Video::isValid() const {
    return m_impl->valid;
}

void Video::play() {
    if (m_impl->valid) {
        m_impl->setState(VideoState::Playing);
    }
}

void Video::pause() {
    if (m_impl->currentState == VideoState::Playing) {
        m_impl->setState(VideoState::Paused);
    }
}

void Video::stop() {
    m_impl->currentTimePos = 0.0f;
    m_impl->setState(VideoState::Stopped);
}

void Video::seek(float seconds) {
    if (m_impl->valid) {
        m_impl->currentTimePos = std::clamp(seconds, 0.0f, m_impl->videoDuration);
    }
}

Video& Video::volume(float vol) {
    m_impl->volumeLevel = std::clamp(vol, 0.0f, 1.0f);
    return *this;
}

Video& Video::loop(bool enabled) {
    m_impl->looping = enabled;
    return *this;
}

Video& Video::playbackRate(float rate) {
    m_impl->rate = std::clamp(rate, 0.1f, 4.0f);
    return *this;
}

Video& Video::onFrame(FrameCallback callback) {
    m_impl->frameCallback = std::move(callback);
    return *this;
}

Video& Video::onStateChange(StateCallback callback) {
    m_impl->stateCallback = std::move(callback);
    return *this;
}

Video& Video::onEnd(std::function<void()> callback) {
    m_impl->endCallback = std::move(callback);
    return *this;
}

VideoState Video::state() const {
    return m_impl->currentState;
}

bool Video::isPlaying() const {
    return m_impl->currentState == VideoState::Playing;
}

bool Video::isPaused() const {
    return m_impl->currentState == VideoState::Paused;
}

bool Video::isStopped() const {
    return m_impl->currentState == VideoState::Stopped;
}

float Video::duration() const {
    return m_impl->videoDuration;
}

float Video::currentTime() const {
    return m_impl->currentTimePos;
}

int Video::width() const {
    return m_impl->videoWidth;
}

int Video::height() const {
    return m_impl->videoHeight;
}

float Video::frameRate() const {
    return m_impl->fps;
}

float Video::getVolume() const {
    return m_impl->volumeLevel;
}

bool Video::isLooping() const {
    return m_impl->looping;
}

VideoFrame Video::getCurrentFrame() const {
    return m_impl->currentFrame;
}

void Video::update(float deltaTime) {
    if (m_impl->currentState != VideoState::Playing) {
        return;
    }
    
    m_impl->currentTimePos += deltaTime * m_impl->rate;
    
    // Check for end of video
    if (m_impl->videoDuration > 0 && m_impl->currentTimePos >= m_impl->videoDuration) {
        if (m_impl->looping) {
            m_impl->currentTimePos = 0.0f;
        } else {
            m_impl->currentTimePos = m_impl->videoDuration;
            m_impl->setState(VideoState::Stopped);
            if (m_impl->endCallback) {
                m_impl->endCallback();
            }
        }
    }
    
    // Update current frame timestamp
    m_impl->currentFrame.timestamp = m_impl->currentTimePos;
    
    // Notify frame callback
    if (m_impl->frameCallback) {
        m_impl->frameCallback(m_impl->currentFrame);
    }
}

VideoHandle Video::build() {
    // In a full implementation, this would return a handle to the video player
    return nullptr;
}

// ============================================================================
// Screenshot Implementation
// ============================================================================

ImageFormat Screenshot::detectFormat(const std::string& path) {
    size_t dotPos = path.rfind('.');
    if (dotPos == std::string::npos) {
        return ImageFormat::PNG;
    }
    
    std::string ext = path.substr(dotPos + 1);
    // Convert to lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == "jpg" || ext == "jpeg") return ImageFormat::JPG;
    if (ext == "bmp") return ImageFormat::BMP;
    if (ext == "tga") return ImageFormat::TGA;
    return ImageFormat::PNG;
}

bool Screenshot::saveToFile(const uint8_t* data, int width, int height,
                           const std::string& path, ImageFormat format) {
    if (!data || width <= 0 || height <= 0) {
        return false;
    }
    
    int result = 0;
    switch (format) {
        case ImageFormat::PNG:
            result = stbi_write_png(path.c_str(), width, height, 4, data, width * 4);
            break;
        case ImageFormat::JPG:
            result = stbi_write_jpg(path.c_str(), width, height, 4, data, 90);
            break;
        case ImageFormat::BMP:
            result = stbi_write_bmp(path.c_str(), width, height, 4, data);
            break;
        case ImageFormat::TGA:
            result = stbi_write_tga(path.c_str(), width, height, 4, data);
            break;
    }
    
    return result != 0;
}

#ifdef _WIN32
// Windows-specific screenshot implementation

CaptureResult Screenshot::captureScreenToMemory() {
    CaptureResult result;
    
    // Get screen dimensions
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    // Create device contexts
    HDC screenDC = GetDC(nullptr);
    HDC memDC = CreateCompatibleDC(screenDC);
    
    if (!screenDC || !memDC) {
        result.errorMessage = "Failed to create device context";
        if (memDC) DeleteDC(memDC);
        if (screenDC) ReleaseDC(nullptr, screenDC);
        return result;
    }
    
    // Create bitmap
    HBITMAP bitmap = CreateCompatibleBitmap(screenDC, screenWidth, screenHeight);
    if (!bitmap) {
        result.errorMessage = "Failed to create bitmap";
        DeleteDC(memDC);
        ReleaseDC(nullptr, screenDC);
        return result;
    }
    
    // Select bitmap into memory DC and copy screen
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);
    BitBlt(memDC, 0, 0, screenWidth, screenHeight, screenDC, 0, 0, SRCCOPY);
    SelectObject(memDC, oldBitmap);
    
    // Get bitmap data
    BITMAPINFOHEADER bi = {};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = screenWidth;
    bi.biHeight = -screenHeight;  // Negative for top-down
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    
    result.data.resize(static_cast<size_t>(screenWidth) * screenHeight * 4);
    
    if (GetDIBits(memDC, bitmap, 0, screenHeight, result.data.data(),
                  (BITMAPINFO*)&bi, DIB_RGB_COLORS)) {
        // Convert BGRA to RGBA
        for (size_t i = 0; i < result.data.size(); i += 4) {
            std::swap(result.data[i], result.data[i + 2]);
        }
        
        result.width = screenWidth;
        result.height = screenHeight;
        result.success = true;
    } else {
        result.errorMessage = "Failed to get bitmap data";
    }
    
    // Cleanup
    DeleteObject(bitmap);
    DeleteDC(memDC);
    ReleaseDC(nullptr, screenDC);
    
    return result;
}

CaptureResult Screenshot::captureWindowToMemory(void* windowHandle) {
    CaptureResult result;
    HWND hwnd = static_cast<HWND>(windowHandle);
    
    if (!hwnd || !IsWindow(hwnd)) {
        result.errorMessage = "Invalid window handle";
        return result;
    }
    
    // Get window dimensions
    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    
    if (width <= 0 || height <= 0) {
        result.errorMessage = "Invalid window dimensions";
        return result;
    }
    
    // Create device contexts
    HDC windowDC = GetDC(hwnd);
    HDC memDC = CreateCompatibleDC(windowDC);
    
    if (!windowDC || !memDC) {
        result.errorMessage = "Failed to create device context";
        if (memDC) DeleteDC(memDC);
        if (windowDC) ReleaseDC(hwnd, windowDC);
        return result;
    }
    
    // Create bitmap
    HBITMAP bitmap = CreateCompatibleBitmap(windowDC, width, height);
    if (!bitmap) {
        result.errorMessage = "Failed to create bitmap";
        DeleteDC(memDC);
        ReleaseDC(hwnd, windowDC);
        return result;
    }
    
    // Select bitmap and copy window content
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);
    BitBlt(memDC, 0, 0, width, height, windowDC, 0, 0, SRCCOPY);
    SelectObject(memDC, oldBitmap);
    
    // Get bitmap data
    BITMAPINFOHEADER bi = {};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    
    result.data.resize(static_cast<size_t>(width) * height * 4);
    
    if (GetDIBits(memDC, bitmap, 0, height, result.data.data(),
                  (BITMAPINFO*)&bi, DIB_RGB_COLORS)) {
        // Convert BGRA to RGBA
        for (size_t i = 0; i < result.data.size(); i += 4) {
            std::swap(result.data[i], result.data[i + 2]);
        }
        
        result.width = width;
        result.height = height;
        result.success = true;
    } else {
        result.errorMessage = "Failed to get bitmap data";
    }
    
    // Cleanup
    DeleteObject(bitmap);
    DeleteDC(memDC);
    ReleaseDC(hwnd, windowDC);
    
    return result;
}

CaptureResult Screenshot::captureRegionToMemory(const Rect& rect) {
    CaptureResult result;
    
    int x = static_cast<int>(rect.x);
    int y = static_cast<int>(rect.y);
    int width = static_cast<int>(rect.width);
    int height = static_cast<int>(rect.height);
    
    if (width <= 0 || height <= 0) {
        result.errorMessage = "Invalid region dimensions";
        return result;
    }
    
    // Create device contexts
    HDC screenDC = GetDC(nullptr);
    HDC memDC = CreateCompatibleDC(screenDC);
    
    if (!screenDC || !memDC) {
        result.errorMessage = "Failed to create device context";
        if (memDC) DeleteDC(memDC);
        if (screenDC) ReleaseDC(nullptr, screenDC);
        return result;
    }
    
    // Create bitmap
    HBITMAP bitmap = CreateCompatibleBitmap(screenDC, width, height);
    if (!bitmap) {
        result.errorMessage = "Failed to create bitmap";
        DeleteDC(memDC);
        ReleaseDC(nullptr, screenDC);
        return result;
    }
    
    // Select bitmap and copy region
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);
    BitBlt(memDC, 0, 0, width, height, screenDC, x, y, SRCCOPY);
    SelectObject(memDC, oldBitmap);
    
    // Get bitmap data
    BITMAPINFOHEADER bi = {};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    
    result.data.resize(static_cast<size_t>(width) * height * 4);
    
    if (GetDIBits(memDC, bitmap, 0, height, result.data.data(),
                  (BITMAPINFO*)&bi, DIB_RGB_COLORS)) {
        // Convert BGRA to RGBA
        for (size_t i = 0; i < result.data.size(); i += 4) {
            std::swap(result.data[i], result.data[i + 2]);
        }
        
        result.width = width;
        result.height = height;
        result.success = true;
    } else {
        result.errorMessage = "Failed to get bitmap data";
    }
    
    // Cleanup
    DeleteObject(bitmap);
    DeleteDC(memDC);
    ReleaseDC(nullptr, screenDC);
    
    return result;
}

bool Screenshot::captureActiveWindow(const std::string& path) {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) {
        return false;
    }
    return captureWindow(hwnd, path);
}

#else
// Non-Windows stub implementations

CaptureResult Screenshot::captureScreenToMemory() {
    CaptureResult result;
    result.errorMessage = "Screenshot capture not implemented for this platform";
    return result;
}

CaptureResult Screenshot::captureWindowToMemory(void* /*windowHandle*/) {
    CaptureResult result;
    result.errorMessage = "Screenshot capture not implemented for this platform";
    return result;
}

CaptureResult Screenshot::captureRegionToMemory(const Rect& /*rect*/) {
    CaptureResult result;
    result.errorMessage = "Screenshot capture not implemented for this platform";
    return result;
}

bool Screenshot::captureActiveWindow(const std::string& /*path*/) {
    return false;
}

#endif

bool Screenshot::captureWindow(void* windowHandle, const std::string& path) {
    CaptureResult result = captureWindowToMemory(windowHandle);
    if (!result.success) {
        return false;
    }
    return saveToFile(result.data.data(), result.width, result.height, 
                     path, detectFormat(path));
}

bool Screenshot::captureRegion(const Rect& rect, const std::string& path) {
    CaptureResult result = captureRegionToMemory(rect);
    if (!result.success) {
        return false;
    }
    return saveToFile(result.data.data(), result.width, result.height,
                     path, detectFormat(path));
}

bool Screenshot::captureScreen(const std::string& path) {
    CaptureResult result = captureScreenToMemory();
    if (!result.success) {
        return false;
    }
    return saveToFile(result.data.data(), result.width, result.height,
                     path, detectFormat(path));
}


// ============================================================================
// Image Implementation
// ============================================================================

struct Image::Impl {
    int imageWidth = 0;
    int imageHeight = 0;
    std::vector<uint8_t> pixels;  // RGBA format
    bool valid = false;
    
    size_t pixelIndex(int x, int y) const {
        return (static_cast<size_t>(y) * imageWidth + x) * 4;
    }
    
    bool inBounds(int x, int y) const {
        return x >= 0 && x < imageWidth && y >= 0 && y < imageHeight;
    }
};

Image::Image() : m_impl(std::make_shared<Impl>()) {}

Image Image::load(const std::string& path) {
    Image image;
    
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    
    if (data) {
        image.m_impl->imageWidth = width;
        image.m_impl->imageHeight = height;
        image.m_impl->pixels.assign(data, data + (static_cast<size_t>(width) * height * 4));
        image.m_impl->valid = true;
        stbi_image_free(data);
    }
    
    return image;
}

Image Image::create(int width, int height, const Color& fillColor) {
    Image image;
    
    if (width > 0 && height > 0) {
        image.m_impl->imageWidth = width;
        image.m_impl->imageHeight = height;
        image.m_impl->pixels.resize(static_cast<size_t>(width) * height * 4);
        image.m_impl->valid = true;
        
        // Fill with color
        uint8_t r = static_cast<uint8_t>(fillColor.r * 255);
        uint8_t g = static_cast<uint8_t>(fillColor.g * 255);
        uint8_t b = static_cast<uint8_t>(fillColor.b * 255);
        uint8_t a = static_cast<uint8_t>(fillColor.a * 255);
        
        for (size_t i = 0; i < image.m_impl->pixels.size(); i += 4) {
            image.m_impl->pixels[i] = r;
            image.m_impl->pixels[i + 1] = g;
            image.m_impl->pixels[i + 2] = b;
            image.m_impl->pixels[i + 3] = a;
        }
    }
    
    return image;
}

Image Image::fromData(const uint8_t* data, int width, int height) {
    Image image;
    
    if (data && width > 0 && height > 0) {
        image.m_impl->imageWidth = width;
        image.m_impl->imageHeight = height;
        image.m_impl->pixels.assign(data, data + (static_cast<size_t>(width) * height * 4));
        image.m_impl->valid = true;
    }
    
    return image;
}

bool Image::isValid() const {
    return m_impl->valid;
}

Image& Image::resize(int newWidth, int newHeight, ResizeMode mode) {
    if (!m_impl->valid || newWidth <= 0 || newHeight <= 0) {
        return *this;
    }
    
    std::vector<uint8_t> newPixels(static_cast<size_t>(newWidth) * newHeight * 4);
    
    float xRatio = static_cast<float>(m_impl->imageWidth) / newWidth;
    float yRatio = static_cast<float>(m_impl->imageHeight) / newHeight;
    
    for (int y = 0; y < newHeight; ++y) {
        for (int x = 0; x < newWidth; ++x) {
            size_t dstIdx = (static_cast<size_t>(y) * newWidth + x) * 4;
            
            if (mode == ResizeMode::NearestNeighbor) {
                int srcX = static_cast<int>(x * xRatio);
                int srcY = static_cast<int>(y * yRatio);
                srcX = std::min(srcX, m_impl->imageWidth - 1);
                srcY = std::min(srcY, m_impl->imageHeight - 1);
                
                size_t srcIdx = m_impl->pixelIndex(srcX, srcY);
                std::memcpy(&newPixels[dstIdx], &m_impl->pixels[srcIdx], 4);
            } else {
                // Bilinear interpolation
                float srcXf = x * xRatio;
                float srcYf = y * yRatio;
                
                int x0 = static_cast<int>(srcXf);
                int y0 = static_cast<int>(srcYf);
                int x1 = std::min(x0 + 1, m_impl->imageWidth - 1);
                int y1 = std::min(y0 + 1, m_impl->imageHeight - 1);
                
                float xFrac = srcXf - x0;
                float yFrac = srcYf - y0;
                
                for (int c = 0; c < 4; ++c) {
                    float v00 = m_impl->pixels[m_impl->pixelIndex(x0, y0) + c];
                    float v10 = m_impl->pixels[m_impl->pixelIndex(x1, y0) + c];
                    float v01 = m_impl->pixels[m_impl->pixelIndex(x0, y1) + c];
                    float v11 = m_impl->pixels[m_impl->pixelIndex(x1, y1) + c];
                    
                    float v0 = v00 + (v10 - v00) * xFrac;
                    float v1 = v01 + (v11 - v01) * xFrac;
                    float v = v0 + (v1 - v0) * yFrac;
                    
                    newPixels[dstIdx + c] = static_cast<uint8_t>(std::clamp(v, 0.0f, 255.0f));
                }
            }
        }
    }
    
    m_impl->pixels = std::move(newPixels);
    m_impl->imageWidth = newWidth;
    m_impl->imageHeight = newHeight;
    
    return *this;
}

Image& Image::crop(const Rect& rect) {
    if (!m_impl->valid) {
        return *this;
    }
    
    int x = static_cast<int>(rect.x);
    int y = static_cast<int>(rect.y);
    int w = static_cast<int>(rect.width);
    int h = static_cast<int>(rect.height);
    
    // Clamp to image bounds
    x = std::max(0, x);
    y = std::max(0, y);
    w = std::min(w, m_impl->imageWidth - x);
    h = std::min(h, m_impl->imageHeight - y);
    
    if (w <= 0 || h <= 0) {
        return *this;
    }
    
    std::vector<uint8_t> newPixels(static_cast<size_t>(w) * h * 4);
    
    for (int row = 0; row < h; ++row) {
        size_t srcIdx = m_impl->pixelIndex(x, y + row);
        size_t dstIdx = static_cast<size_t>(row) * w * 4;
        std::memcpy(&newPixels[dstIdx], &m_impl->pixels[srcIdx], w * 4);
    }
    
    m_impl->pixels = std::move(newPixels);
    m_impl->imageWidth = w;
    m_impl->imageHeight = h;
    
    return *this;
}

Image& Image::rotate(float degrees) {
    if (!m_impl->valid) {
        return *this;
    }
    
    // Normalize angle
    while (degrees < 0) degrees += 360;
    while (degrees >= 360) degrees -= 360;
    
    // Handle 90-degree rotations efficiently
    if (std::abs(degrees - 90) < 0.01f) return rotate90();
    if (std::abs(degrees - 180) < 0.01f) return rotate180();
    if (std::abs(degrees - 270) < 0.01f) return rotate270();
    if (std::abs(degrees) < 0.01f) return *this;
    
    // General rotation
    float radians = degrees * 3.14159265358979f / 180.0f;
    float cosA = std::cos(radians);
    float sinA = std::sin(radians);
    
    // Calculate new dimensions
    int oldW = m_impl->imageWidth;
    int oldH = m_impl->imageHeight;
    
    float corners[4][2] = {
        {0, 0}, {static_cast<float>(oldW), 0},
        {0, static_cast<float>(oldH)}, {static_cast<float>(oldW), static_cast<float>(oldH)}
    };
    
    float minX = 0, maxX = 0, minY = 0, maxY = 0;
    for (int i = 0; i < 4; ++i) {
        float rx = corners[i][0] * cosA - corners[i][1] * sinA;
        float ry = corners[i][0] * sinA + corners[i][1] * cosA;
        if (i == 0 || rx < minX) minX = rx;
        if (i == 0 || rx > maxX) maxX = rx;
        if (i == 0 || ry < minY) minY = ry;
        if (i == 0 || ry > maxY) maxY = ry;
    }
    
    int newW = static_cast<int>(std::ceil(maxX - minX));
    int newH = static_cast<int>(std::ceil(maxY - minY));
    
    std::vector<uint8_t> newPixels(static_cast<size_t>(newW) * newH * 4, 0);
    
    float centerX = oldW / 2.0f;
    float centerY = oldH / 2.0f;
    float newCenterX = newW / 2.0f;
    float newCenterY = newH / 2.0f;
    
    for (int y = 0; y < newH; ++y) {
        for (int x = 0; x < newW; ++x) {
            float dx = x - newCenterX;
            float dy = y - newCenterY;
            
            float srcX = dx * cosA + dy * sinA + centerX;
            float srcY = -dx * sinA + dy * cosA + centerY;
            
            int sx = static_cast<int>(srcX);
            int sy = static_cast<int>(srcY);
            
            if (sx >= 0 && sx < oldW && sy >= 0 && sy < oldH) {
                size_t srcIdx = m_impl->pixelIndex(sx, sy);
                size_t dstIdx = (static_cast<size_t>(y) * newW + x) * 4;
                std::memcpy(&newPixels[dstIdx], &m_impl->pixels[srcIdx], 4);
            }
        }
    }
    
    m_impl->pixels = std::move(newPixels);
    m_impl->imageWidth = newW;
    m_impl->imageHeight = newH;
    
    return *this;
}

Image& Image::rotate90() {
    if (!m_impl->valid) return *this;
    
    int oldW = m_impl->imageWidth;
    int oldH = m_impl->imageHeight;
    int newW = oldH;
    int newH = oldW;
    
    std::vector<uint8_t> newPixels(static_cast<size_t>(newW) * newH * 4);
    
    for (int y = 0; y < oldH; ++y) {
        for (int x = 0; x < oldW; ++x) {
            size_t srcIdx = m_impl->pixelIndex(x, y);
            size_t dstIdx = (static_cast<size_t>(x) * newW + (newW - 1 - y)) * 4;
            std::memcpy(&newPixels[dstIdx], &m_impl->pixels[srcIdx], 4);
        }
    }
    
    m_impl->pixels = std::move(newPixels);
    m_impl->imageWidth = newW;
    m_impl->imageHeight = newH;
    
    return *this;
}

Image& Image::rotate180() {
    if (!m_impl->valid) return *this;
    
    int w = m_impl->imageWidth;
    int h = m_impl->imageHeight;
    
    std::vector<uint8_t> newPixels(m_impl->pixels.size());
    
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            size_t srcIdx = m_impl->pixelIndex(x, y);
            size_t dstIdx = m_impl->pixelIndex(w - 1 - x, h - 1 - y);
            std::memcpy(&newPixels[dstIdx], &m_impl->pixels[srcIdx], 4);
        }
    }
    
    m_impl->pixels = std::move(newPixels);
    return *this;
}

Image& Image::rotate270() {
    if (!m_impl->valid) return *this;
    
    int oldW = m_impl->imageWidth;
    int oldH = m_impl->imageHeight;
    int newW = oldH;
    int newH = oldW;
    
    std::vector<uint8_t> newPixels(static_cast<size_t>(newW) * newH * 4);
    
    for (int y = 0; y < oldH; ++y) {
        for (int x = 0; x < oldW; ++x) {
            size_t srcIdx = m_impl->pixelIndex(x, y);
            size_t dstIdx = (static_cast<size_t>(oldW - 1 - x) * newW + y) * 4;
            std::memcpy(&newPixels[dstIdx], &m_impl->pixels[srcIdx], 4);
        }
    }
    
    m_impl->pixels = std::move(newPixels);
    m_impl->imageWidth = newW;
    m_impl->imageHeight = newH;
    
    return *this;
}

Image& Image::flipHorizontal() {
    if (!m_impl->valid) return *this;
    
    int w = m_impl->imageWidth;
    int h = m_impl->imageHeight;
    
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w / 2; ++x) {
            size_t idx1 = m_impl->pixelIndex(x, y);
            size_t idx2 = m_impl->pixelIndex(w - 1 - x, y);
            
            for (int c = 0; c < 4; ++c) {
                std::swap(m_impl->pixels[idx1 + c], m_impl->pixels[idx2 + c]);
            }
        }
    }
    
    return *this;
}

Image& Image::flipVertical() {
    if (!m_impl->valid) return *this;
    
    int w = m_impl->imageWidth;
    int h = m_impl->imageHeight;
    size_t rowSize = static_cast<size_t>(w) * 4;
    
    std::vector<uint8_t> tempRow(rowSize);
    
    for (int y = 0; y < h / 2; ++y) {
        size_t idx1 = static_cast<size_t>(y) * rowSize;
        size_t idx2 = static_cast<size_t>(h - 1 - y) * rowSize;
        
        std::memcpy(tempRow.data(), &m_impl->pixels[idx1], rowSize);
        std::memcpy(&m_impl->pixels[idx1], &m_impl->pixels[idx2], rowSize);
        std::memcpy(&m_impl->pixels[idx2], tempRow.data(), rowSize);
    }
    
    return *this;
}


// Filter implementations

Image& Image::grayscale() {
    if (!m_impl->valid) return *this;
    
    for (size_t i = 0; i < m_impl->pixels.size(); i += 4) {
        // Use luminance formula
        float gray = 0.299f * m_impl->pixels[i] + 
                     0.587f * m_impl->pixels[i + 1] + 
                     0.114f * m_impl->pixels[i + 2];
        uint8_t g = static_cast<uint8_t>(std::clamp(gray, 0.0f, 255.0f));
        m_impl->pixels[i] = g;
        m_impl->pixels[i + 1] = g;
        m_impl->pixels[i + 2] = g;
        // Alpha unchanged
    }
    
    return *this;
}

Image& Image::sepia() {
    if (!m_impl->valid) return *this;
    
    for (size_t i = 0; i < m_impl->pixels.size(); i += 4) {
        float r = m_impl->pixels[i];
        float g = m_impl->pixels[i + 1];
        float b = m_impl->pixels[i + 2];
        
        float newR = 0.393f * r + 0.769f * g + 0.189f * b;
        float newG = 0.349f * r + 0.686f * g + 0.168f * b;
        float newB = 0.272f * r + 0.534f * g + 0.131f * b;
        
        m_impl->pixels[i] = static_cast<uint8_t>(std::clamp(newR, 0.0f, 255.0f));
        m_impl->pixels[i + 1] = static_cast<uint8_t>(std::clamp(newG, 0.0f, 255.0f));
        m_impl->pixels[i + 2] = static_cast<uint8_t>(std::clamp(newB, 0.0f, 255.0f));
    }
    
    return *this;
}

Image& Image::invert() {
    if (!m_impl->valid) return *this;
    
    for (size_t i = 0; i < m_impl->pixels.size(); i += 4) {
        m_impl->pixels[i] = 255 - m_impl->pixels[i];
        m_impl->pixels[i + 1] = 255 - m_impl->pixels[i + 1];
        m_impl->pixels[i + 2] = 255 - m_impl->pixels[i + 2];
        // Alpha unchanged
    }
    
    return *this;
}

Image& Image::blur(float radius) {
    if (!m_impl->valid || radius <= 0) return *this;
    
    int r = static_cast<int>(std::ceil(radius));
    int w = m_impl->imageWidth;
    int h = m_impl->imageHeight;
    
    std::vector<uint8_t> temp(m_impl->pixels.size());
    
    // Horizontal pass
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float sumR = 0, sumG = 0, sumB = 0, sumA = 0;
            int count = 0;
            
            for (int dx = -r; dx <= r; ++dx) {
                int nx = x + dx;
                if (nx >= 0 && nx < w) {
                    size_t idx = m_impl->pixelIndex(nx, y);
                    sumR += m_impl->pixels[idx];
                    sumG += m_impl->pixels[idx + 1];
                    sumB += m_impl->pixels[idx + 2];
                    sumA += m_impl->pixels[idx + 3];
                    ++count;
                }
            }
            
            size_t dstIdx = m_impl->pixelIndex(x, y);
            temp[dstIdx] = static_cast<uint8_t>(sumR / count);
            temp[dstIdx + 1] = static_cast<uint8_t>(sumG / count);
            temp[dstIdx + 2] = static_cast<uint8_t>(sumB / count);
            temp[dstIdx + 3] = static_cast<uint8_t>(sumA / count);
        }
    }
    
    // Vertical pass
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float sumR = 0, sumG = 0, sumB = 0, sumA = 0;
            int count = 0;
            
            for (int dy = -r; dy <= r; ++dy) {
                int ny = y + dy;
                if (ny >= 0 && ny < h) {
                    size_t idx = (static_cast<size_t>(ny) * w + x) * 4;
                    sumR += temp[idx];
                    sumG += temp[idx + 1];
                    sumB += temp[idx + 2];
                    sumA += temp[idx + 3];
                    ++count;
                }
            }
            
            size_t dstIdx = m_impl->pixelIndex(x, y);
            m_impl->pixels[dstIdx] = static_cast<uint8_t>(sumR / count);
            m_impl->pixels[dstIdx + 1] = static_cast<uint8_t>(sumG / count);
            m_impl->pixels[dstIdx + 2] = static_cast<uint8_t>(sumB / count);
            m_impl->pixels[dstIdx + 3] = static_cast<uint8_t>(sumA / count);
        }
    }
    
    return *this;
}

Image& Image::gaussianBlur(float radius, float sigma) {
    if (!m_impl->valid || radius <= 0) return *this;
    
    if (sigma <= 0) {
        sigma = radius / 3.0f;
    }
    
    int r = static_cast<int>(std::ceil(radius));
    int kernelSize = 2 * r + 1;
    
    // Generate Gaussian kernel
    std::vector<float> kernel(kernelSize);
    float sum = 0;
    for (int i = 0; i < kernelSize; ++i) {
        float x = static_cast<float>(i - r);
        kernel[i] = std::exp(-(x * x) / (2 * sigma * sigma));
        sum += kernel[i];
    }
    // Normalize
    for (int i = 0; i < kernelSize; ++i) {
        kernel[i] /= sum;
    }
    
    int w = m_impl->imageWidth;
    int h = m_impl->imageHeight;
    std::vector<uint8_t> temp(m_impl->pixels.size());
    
    // Horizontal pass
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float sumR = 0, sumG = 0, sumB = 0, sumA = 0;
            
            for (int i = 0; i < kernelSize; ++i) {
                int nx = x + i - r;
                nx = std::clamp(nx, 0, w - 1);
                size_t idx = m_impl->pixelIndex(nx, y);
                sumR += m_impl->pixels[idx] * kernel[i];
                sumG += m_impl->pixels[idx + 1] * kernel[i];
                sumB += m_impl->pixels[idx + 2] * kernel[i];
                sumA += m_impl->pixels[idx + 3] * kernel[i];
            }
            
            size_t dstIdx = m_impl->pixelIndex(x, y);
            temp[dstIdx] = static_cast<uint8_t>(std::clamp(sumR, 0.0f, 255.0f));
            temp[dstIdx + 1] = static_cast<uint8_t>(std::clamp(sumG, 0.0f, 255.0f));
            temp[dstIdx + 2] = static_cast<uint8_t>(std::clamp(sumB, 0.0f, 255.0f));
            temp[dstIdx + 3] = static_cast<uint8_t>(std::clamp(sumA, 0.0f, 255.0f));
        }
    }
    
    // Vertical pass
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float sumR = 0, sumG = 0, sumB = 0, sumA = 0;
            
            for (int i = 0; i < kernelSize; ++i) {
                int ny = y + i - r;
                ny = std::clamp(ny, 0, h - 1);
                size_t idx = (static_cast<size_t>(ny) * w + x) * 4;
                sumR += temp[idx] * kernel[i];
                sumG += temp[idx + 1] * kernel[i];
                sumB += temp[idx + 2] * kernel[i];
                sumA += temp[idx + 3] * kernel[i];
            }
            
            size_t dstIdx = m_impl->pixelIndex(x, y);
            m_impl->pixels[dstIdx] = static_cast<uint8_t>(std::clamp(sumR, 0.0f, 255.0f));
            m_impl->pixels[dstIdx + 1] = static_cast<uint8_t>(std::clamp(sumG, 0.0f, 255.0f));
            m_impl->pixels[dstIdx + 2] = static_cast<uint8_t>(std::clamp(sumB, 0.0f, 255.0f));
            m_impl->pixels[dstIdx + 3] = static_cast<uint8_t>(std::clamp(sumA, 0.0f, 255.0f));
        }
    }
    
    return *this;
}

Image& Image::brightness(float amount) {
    if (!m_impl->valid) return *this;
    
    float factor = 1.0f + amount;
    
    for (size_t i = 0; i < m_impl->pixels.size(); i += 4) {
        m_impl->pixels[i] = static_cast<uint8_t>(std::clamp(m_impl->pixels[i] * factor, 0.0f, 255.0f));
        m_impl->pixels[i + 1] = static_cast<uint8_t>(std::clamp(m_impl->pixels[i + 1] * factor, 0.0f, 255.0f));
        m_impl->pixels[i + 2] = static_cast<uint8_t>(std::clamp(m_impl->pixels[i + 2] * factor, 0.0f, 255.0f));
    }
    
    return *this;
}

Image& Image::contrast(float amount) {
    if (!m_impl->valid) return *this;
    
    float factor = (1.0f + amount);
    factor = factor * factor;  // Make the effect more pronounced
    
    for (size_t i = 0; i < m_impl->pixels.size(); i += 4) {
        for (int c = 0; c < 3; ++c) {
            float value = m_impl->pixels[i + c] / 255.0f;
            value = (value - 0.5f) * factor + 0.5f;
            m_impl->pixels[i + c] = static_cast<uint8_t>(std::clamp(value * 255.0f, 0.0f, 255.0f));
        }
    }
    
    return *this;
}

Image& Image::saturation(float amount) {
    if (!m_impl->valid) return *this;
    
    float factor = 1.0f + amount;
    
    for (size_t i = 0; i < m_impl->pixels.size(); i += 4) {
        float r = m_impl->pixels[i];
        float g = m_impl->pixels[i + 1];
        float b = m_impl->pixels[i + 2];
        
        float gray = 0.299f * r + 0.587f * g + 0.114f * b;
        
        m_impl->pixels[i] = static_cast<uint8_t>(std::clamp(gray + (r - gray) * factor, 0.0f, 255.0f));
        m_impl->pixels[i + 1] = static_cast<uint8_t>(std::clamp(gray + (g - gray) * factor, 0.0f, 255.0f));
        m_impl->pixels[i + 2] = static_cast<uint8_t>(std::clamp(gray + (b - gray) * factor, 0.0f, 255.0f));
    }
    
    return *this;
}

Image& Image::hue(float degrees) {
    if (!m_impl->valid) return *this;
    
    float radians = degrees * 3.14159265358979f / 180.0f;
    float cosA = std::cos(radians);
    float sinA = std::sin(radians);
    
    // Hue rotation matrix
    float matrix[3][3] = {
        {0.213f + cosA * 0.787f - sinA * 0.213f, 0.715f - cosA * 0.715f - sinA * 0.715f, 0.072f - cosA * 0.072f + sinA * 0.928f},
        {0.213f - cosA * 0.213f + sinA * 0.143f, 0.715f + cosA * 0.285f + sinA * 0.140f, 0.072f - cosA * 0.072f - sinA * 0.283f},
        {0.213f - cosA * 0.213f - sinA * 0.787f, 0.715f - cosA * 0.715f + sinA * 0.715f, 0.072f + cosA * 0.928f + sinA * 0.072f}
    };
    
    for (size_t i = 0; i < m_impl->pixels.size(); i += 4) {
        float r = m_impl->pixels[i];
        float g = m_impl->pixels[i + 1];
        float b = m_impl->pixels[i + 2];
        
        float newR = matrix[0][0] * r + matrix[0][1] * g + matrix[0][2] * b;
        float newG = matrix[1][0] * r + matrix[1][1] * g + matrix[1][2] * b;
        float newB = matrix[2][0] * r + matrix[2][1] * g + matrix[2][2] * b;
        
        m_impl->pixels[i] = static_cast<uint8_t>(std::clamp(newR, 0.0f, 255.0f));
        m_impl->pixels[i + 1] = static_cast<uint8_t>(std::clamp(newG, 0.0f, 255.0f));
        m_impl->pixels[i + 2] = static_cast<uint8_t>(std::clamp(newB, 0.0f, 255.0f));
    }
    
    return *this;
}

Image& Image::sharpen(float amount) {
    float kernel[9] = {
        0, -amount, 0,
        -amount, 1 + 4 * amount, -amount,
        0, -amount, 0
    };
    return convolve(kernel);
}

Image& Image::edgeDetect() {
    float kernel[9] = {
        -1, -1, -1,
        -1,  8, -1,
        -1, -1, -1
    };
    return convolve(kernel);
}

Image& Image::emboss() {
    float kernel[9] = {
        -2, -1, 0,
        -1,  1, 1,
         0,  1, 2
    };
    return convolve(kernel);
}

Image& Image::applyFilter(FilterType type) {
    switch (type) {
        case FilterType::Grayscale: return grayscale();
        case FilterType::Sepia: return sepia();
        case FilterType::Invert: return invert();
        case FilterType::Sharpen: return sharpen();
        case FilterType::EdgeDetect: return edgeDetect();
        case FilterType::Emboss: return emboss();
    }
    return *this;
}

Image& Image::convolve(const float kernel[9]) {
    if (!m_impl->valid) return *this;
    
    int w = m_impl->imageWidth;
    int h = m_impl->imageHeight;
    std::vector<uint8_t> result(m_impl->pixels.size());
    
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float sumR = 0, sumG = 0, sumB = 0;
            
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int nx = std::clamp(x + kx, 0, w - 1);
                    int ny = std::clamp(y + ky, 0, h - 1);
                    
                    size_t idx = m_impl->pixelIndex(nx, ny);
                    int ki = (ky + 1) * 3 + (kx + 1);
                    
                    sumR += m_impl->pixels[idx] * kernel[ki];
                    sumG += m_impl->pixels[idx + 1] * kernel[ki];
                    sumB += m_impl->pixels[idx + 2] * kernel[ki];
                }
            }
            
            size_t dstIdx = m_impl->pixelIndex(x, y);
            result[dstIdx] = static_cast<uint8_t>(std::clamp(sumR, 0.0f, 255.0f));
            result[dstIdx + 1] = static_cast<uint8_t>(std::clamp(sumG, 0.0f, 255.0f));
            result[dstIdx + 2] = static_cast<uint8_t>(std::clamp(sumB, 0.0f, 255.0f));
            result[dstIdx + 3] = m_impl->pixels[dstIdx + 3];  // Preserve alpha
        }
    }
    
    m_impl->pixels = std::move(result);
    return *this;
}


// Pixel access and utility methods

Color Image::getPixel(int x, int y) const {
    if (!m_impl->valid || !m_impl->inBounds(x, y)) {
        return Color::Transparent;
    }
    
    size_t idx = m_impl->pixelIndex(x, y);
    return Color{
        m_impl->pixels[idx] / 255.0f,
        m_impl->pixels[idx + 1] / 255.0f,
        m_impl->pixels[idx + 2] / 255.0f,
        m_impl->pixels[idx + 3] / 255.0f
    };
}

Image& Image::setPixel(int x, int y, const Color& color) {
    if (!m_impl->valid || !m_impl->inBounds(x, y)) {
        return *this;
    }
    
    size_t idx = m_impl->pixelIndex(x, y);
    m_impl->pixels[idx] = static_cast<uint8_t>(color.r * 255);
    m_impl->pixels[idx + 1] = static_cast<uint8_t>(color.g * 255);
    m_impl->pixels[idx + 2] = static_cast<uint8_t>(color.b * 255);
    m_impl->pixels[idx + 3] = static_cast<uint8_t>(color.a * 255);
    
    return *this;
}

Image& Image::fill(const Color& color) {
    if (!m_impl->valid) return *this;
    
    uint8_t r = static_cast<uint8_t>(color.r * 255);
    uint8_t g = static_cast<uint8_t>(color.g * 255);
    uint8_t b = static_cast<uint8_t>(color.b * 255);
    uint8_t a = static_cast<uint8_t>(color.a * 255);
    
    for (size_t i = 0; i < m_impl->pixels.size(); i += 4) {
        m_impl->pixels[i] = r;
        m_impl->pixels[i + 1] = g;
        m_impl->pixels[i + 2] = b;
        m_impl->pixels[i + 3] = a;
    }
    
    return *this;
}

const uint8_t* Image::data() const {
    return m_impl->pixels.data();
}

uint8_t* Image::data() {
    return m_impl->pixels.data();
}

bool Image::save(const std::string& path, int quality) {
    return save(path, Screenshot::detectFormat(path), quality);
}

bool Image::save(const std::string& path, ImageFormat format, int quality) {
    if (!m_impl->valid) {
        return false;
    }
    
    int result = 0;
    switch (format) {
        case ImageFormat::PNG:
            result = stbi_write_png(path.c_str(), m_impl->imageWidth, m_impl->imageHeight, 
                                   4, m_impl->pixels.data(), m_impl->imageWidth * 4);
            break;
        case ImageFormat::JPG:
            result = stbi_write_jpg(path.c_str(), m_impl->imageWidth, m_impl->imageHeight,
                                   4, m_impl->pixels.data(), quality);
            break;
        case ImageFormat::BMP:
            result = stbi_write_bmp(path.c_str(), m_impl->imageWidth, m_impl->imageHeight,
                                   4, m_impl->pixels.data());
            break;
        case ImageFormat::TGA:
            result = stbi_write_tga(path.c_str(), m_impl->imageWidth, m_impl->imageHeight,
                                   4, m_impl->pixels.data());
            break;
    }
    
    return result != 0;
}

int Image::width() const {
    return m_impl->imageWidth;
}

int Image::height() const {
    return m_impl->imageHeight;
}

int Image::channels() const {
    return 4;  // Always RGBA
}

size_t Image::dataSize() const {
    return m_impl->pixels.size();
}

Image Image::clone() const {
    Image copy;
    copy.m_impl->imageWidth = m_impl->imageWidth;
    copy.m_impl->imageHeight = m_impl->imageHeight;
    copy.m_impl->pixels = m_impl->pixels;
    copy.m_impl->valid = m_impl->valid;
    return copy;
}

ImageHandle Image::build() {
    // In a full implementation, this would return a handle to the image
    return nullptr;
}

} // namespace KGKMedia
