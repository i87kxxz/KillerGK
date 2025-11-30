/**
 * @file Image.cpp
 * @brief Image widget implementation
 */

#include "KillerGK/widgets/Image.hpp"
#include <algorithm>
#include <cmath>

namespace KillerGK {

// =============================================================================
// ImageData - Internal data structure
// =============================================================================

struct Image::ImageData {
    std::string sourcePath;
    TextureHandle texture;
    bool loaded = false;
    
    // Scaling and alignment
    ImageScaleMode scaleMode = ImageScaleMode::Fit;
    ImageAlignment alignment = ImageAlignment::Center;
    
    // Visual properties
    Color tint = Color::White;
    bool grayscale = false;
    float rotation = 0.0f;
    bool flipH = false;
    bool flipV = false;
    
    // Source rectangle
    Rect sourceRect;
    bool useSourceRect = false;
    
    // Computed display rect
    Rect displayRect;
    
    // Loading
    std::function<void(bool)> onLoadCallback;
    std::string fallbackPath;
};

// =============================================================================
// Image Implementation
// =============================================================================

Image::Image() 
    : Widget()
    , m_imageData(std::make_shared<ImageData>()) 
{
    // Images are transparent by default
    backgroundColor(Color::Transparent);
}

Image Image::create() {
    return Image();
}

// Image Source
Image& Image::source(const std::string& path) {
    m_imageData->sourcePath = path;
    m_imageData->loaded = false;
    m_imageData->texture = nullptr;
    return *this;
}

const std::string& Image::getSource() const {
    return m_imageData->sourcePath;
}

Image& Image::texture(TextureHandle tex) {
    m_imageData->texture = tex;
    m_imageData->loaded = (tex != nullptr);
    if (m_imageData->loaded) {
        calculateDisplayRect();
    }
    return *this;
}

TextureHandle Image::getTexture() const {
    return m_imageData->texture;
}

bool Image::isLoaded() const {
    return m_imageData->loaded && m_imageData->texture != nullptr;
}

int Image::getImageWidth() const {
    if (m_imageData->texture) {
        return m_imageData->texture->getWidth();
    }
    return 0;
}

int Image::getImageHeight() const {
    if (m_imageData->texture) {
        return m_imageData->texture->getHeight();
    }
    return 0;
}

// Scaling and Alignment
Image& Image::scaleMode(ImageScaleMode mode) {
    m_imageData->scaleMode = mode;
    calculateDisplayRect();
    return *this;
}

ImageScaleMode Image::getScaleMode() const {
    return m_imageData->scaleMode;
}

Image& Image::alignment(ImageAlignment align) {
    m_imageData->alignment = align;
    calculateDisplayRect();
    return *this;
}

ImageAlignment Image::getAlignment() const {
    return m_imageData->alignment;
}

// Visual Properties
Image& Image::tint(const Color& color) {
    m_imageData->tint = color;
    return *this;
}

const Color& Image::getTint() const {
    return m_imageData->tint;
}

Image& Image::grayscale(bool enabled) {
    m_imageData->grayscale = enabled;
    return *this;
}

bool Image::isGrayscale() const {
    return m_imageData->grayscale;
}

Image& Image::rotation(float degrees) {
    m_imageData->rotation = degrees;
    return *this;
}

float Image::getRotation() const {
    return m_imageData->rotation;
}

Image& Image::flipHorizontal(bool flip) {
    m_imageData->flipH = flip;
    return *this;
}

bool Image::isFlippedHorizontal() const {
    return m_imageData->flipH;
}

Image& Image::flipVertical(bool flip) {
    m_imageData->flipV = flip;
    return *this;
}

bool Image::isFlippedVertical() const {
    return m_imageData->flipV;
}

// Source Rectangle
Image& Image::sourceRect(const Rect& rect) {
    m_imageData->sourceRect = rect;
    m_imageData->useSourceRect = true;
    calculateDisplayRect();
    return *this;
}

const Rect& Image::getSourceRect() const {
    return m_imageData->sourceRect;
}

bool Image::hasSourceRect() const {
    return m_imageData->useSourceRect;
}

Image& Image::clearSourceRect() {
    m_imageData->useSourceRect = false;
    m_imageData->sourceRect = Rect();
    calculateDisplayRect();
    return *this;
}

// Loading
bool Image::load() {
    if (m_imageData->sourcePath.empty()) {
        return false;
    }
    
    m_imageData->texture = Texture::loadFromFile(m_imageData->sourcePath);
    m_imageData->loaded = (m_imageData->texture != nullptr);
    
    if (!m_imageData->loaded && !m_imageData->fallbackPath.empty()) {
        // Try fallback
        m_imageData->texture = Texture::loadFromFile(m_imageData->fallbackPath);
        m_imageData->loaded = (m_imageData->texture != nullptr);
    }
    
    if (m_imageData->loaded) {
        calculateDisplayRect();
    }
    
    if (m_imageData->onLoadCallback) {
        m_imageData->onLoadCallback(m_imageData->loaded);
    }
    
    return m_imageData->loaded;
}

void Image::unload() {
    m_imageData->texture = nullptr;
    m_imageData->loaded = false;
}

Image& Image::onLoad(std::function<void(bool)> callback) {
    m_imageData->onLoadCallback = std::move(callback);
    return *this;
}

Image& Image::fallback(const std::string& path) {
    m_imageData->fallbackPath = path;
    return *this;
}

const std::string& Image::getFallback() const {
    return m_imageData->fallbackPath;
}

// Computed Properties
Rect Image::getDisplayRect() const {
    return m_imageData->displayRect;
}

float Image::getAspectRatio() const {
    if (!m_imageData->texture) {
        return 1.0f;
    }
    
    int imgWidth = m_imageData->texture->getWidth();
    int imgHeight = m_imageData->texture->getHeight();
    
    if (m_imageData->useSourceRect) {
        imgWidth = static_cast<int>(m_imageData->sourceRect.width);
        imgHeight = static_cast<int>(m_imageData->sourceRect.height);
    }
    
    if (imgHeight == 0) {
        return 1.0f;
    }
    
    return static_cast<float>(imgWidth) / static_cast<float>(imgHeight);
}

void Image::calculateDisplayRect() {
    float widgetW = getWidth();
    float widgetH = getHeight();
    
    if (widgetW <= 0 || widgetH <= 0) {
        m_imageData->displayRect = Rect(0, 0, 0, 0);
        return;
    }
    
    float imgW, imgH;
    
    if (m_imageData->texture) {
        if (m_imageData->useSourceRect) {
            imgW = m_imageData->sourceRect.width;
            imgH = m_imageData->sourceRect.height;
        } else {
            imgW = static_cast<float>(m_imageData->texture->getWidth());
            imgH = static_cast<float>(m_imageData->texture->getHeight());
        }
    } else {
        imgW = widgetW;
        imgH = widgetH;
    }
    
    if (imgW <= 0 || imgH <= 0) {
        m_imageData->displayRect = Rect(0, 0, 0, 0);
        return;
    }
    
    float displayW, displayH;
    
    switch (m_imageData->scaleMode) {
        case ImageScaleMode::Fit: {
            float scaleX = widgetW / imgW;
            float scaleY = widgetH / imgH;
            float scale = std::min(scaleX, scaleY);
            displayW = imgW * scale;
            displayH = imgH * scale;
            break;
        }
        case ImageScaleMode::Fill: {
            float scaleX = widgetW / imgW;
            float scaleY = widgetH / imgH;
            float scale = std::max(scaleX, scaleY);
            displayW = imgW * scale;
            displayH = imgH * scale;
            break;
        }
        case ImageScaleMode::Stretch:
            displayW = widgetW;
            displayH = widgetH;
            break;
        case ImageScaleMode::None:
            displayW = imgW;
            displayH = imgH;
            break;
        case ImageScaleMode::Tile:
            displayW = widgetW;
            displayH = widgetH;
            break;
    }
    
    // Calculate position based on alignment
    float x = 0, y = 0;
    
    switch (m_imageData->alignment) {
        case ImageAlignment::TopLeft:
            x = 0;
            y = 0;
            break;
        case ImageAlignment::TopCenter:
            x = (widgetW - displayW) / 2;
            y = 0;
            break;
        case ImageAlignment::TopRight:
            x = widgetW - displayW;
            y = 0;
            break;
        case ImageAlignment::CenterLeft:
            x = 0;
            y = (widgetH - displayH) / 2;
            break;
        case ImageAlignment::Center:
            x = (widgetW - displayW) / 2;
            y = (widgetH - displayH) / 2;
            break;
        case ImageAlignment::CenterRight:
            x = widgetW - displayW;
            y = (widgetH - displayH) / 2;
            break;
        case ImageAlignment::BottomLeft:
            x = 0;
            y = widgetH - displayH;
            break;
        case ImageAlignment::BottomCenter:
            x = (widgetW - displayW) / 2;
            y = widgetH - displayH;
            break;
        case ImageAlignment::BottomRight:
            x = widgetW - displayW;
            y = widgetH - displayH;
            break;
    }
    
    m_imageData->displayRect = Rect(x, y, displayW, displayH);
}

} // namespace KillerGK
