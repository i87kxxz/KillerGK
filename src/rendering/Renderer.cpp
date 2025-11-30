/**
 * @file Renderer.cpp
 * @brief High-level renderer implementation using Vulkan backend
 */

#include "KillerGK/rendering/Renderer.hpp"
#include "KillerGK/rendering/VulkanBackend.hpp"
#include "KillerGK/rendering/ShaderSystem.hpp"
#include "KillerGK/rendering/Renderer2D.hpp"

namespace KillerGK {

struct Renderer::Impl {
    bool initialized = false;
    uint32_t viewportWidth = 800;
    uint32_t viewportHeight = 600;
};

Renderer& Renderer::instance() {
    static Renderer instance;
    return instance;
}

Renderer::Renderer() : m_impl(std::make_unique<Impl>()) {}
Renderer::~Renderer() = default;

bool Renderer::initialize() {
    if (m_impl->initialized) {
        return true;
    }
    
    // Note: VulkanBackend and ShaderSystem should be initialized
    // after a window/surface is created. This just marks the renderer
    // as ready to be used.
    m_impl->initialized = true;
    return true;
}

void Renderer::shutdown() {
    if (!m_impl->initialized) {
        return;
    }
    
    Renderer2D::instance().shutdown();
    ShaderSystem::instance().shutdown();
    VulkanBackend::instance().shutdown();
    
    m_impl->initialized = false;
}

void Renderer::beginFrame() {
    if (!m_impl->initialized) {
        return;
    }
    
    int imageIndex = VulkanBackend::instance().beginFrame();
    if (imageIndex < 0) {
        // Need to recreate swap chain
        return;
    }
    
    VkExtent2D extent = VulkanBackend::instance().getSwapChainExtent();
    m_impl->viewportWidth = extent.width;
    m_impl->viewportHeight = extent.height;
    
    Renderer2D::instance().beginBatch(
        static_cast<float>(m_impl->viewportWidth),
        static_cast<float>(m_impl->viewportHeight));
}

void Renderer::endFrame() {
    if (!m_impl->initialized) {
        return;
    }
    
    Renderer2D::instance().endBatch();
    VulkanBackend::instance().endFrame();
}

void Renderer::drawRect(const Rect& rect, const Color& color) {
    Renderer2D::instance().drawRect(rect, color);
}

void Renderer::drawRoundRect(const Rect& rect, float radius, const Color& color) {
    Renderer2D::instance().drawRoundRect(rect, radius, color);
}

void Renderer::drawCircle(float cx, float cy, float radius, const Color& color) {
    Renderer2D::instance().drawCircle(cx, cy, radius, color);
}

void Renderer::drawLine(float x1, float y1, float x2, float y2, 
                        const Color& color, float thickness) {
    Renderer2D::instance().drawLine(x1, y1, x2, y2, color, thickness);
}

bool Renderer::isInitialized() const {
    return m_impl->initialized;
}

} // namespace KillerGK
