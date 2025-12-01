/**
 * @file Renderer2D.cpp
 * @brief 2D rendering primitives implementation
 */

#include "KillerGK/rendering/Renderer2D.hpp"
#include "KillerGK/rendering/VulkanBackend.hpp"
#include "KillerGK/rendering/ShaderSystem.hpp"
#include "KillerGK/rendering/Texture.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include <cmath>
#include <iostream>

namespace KillerGK {

// Vertex structure matching Vertex2D in ShaderSystem
struct Vertex {
    float position[2];
    float color[4];
    float texCoord[2];
};

struct Renderer2D::Impl {
    bool initialized = false;
    Renderer2DConfig config;
    
    // Vertex and index buffers
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
    
    // Staging buffers for CPU-side data
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    
    // Current batch state
    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;
    bool batchActive = false;
    
    // Projection matrix
    glm::mat4 projection{1.0f};
    
    // Statistics
    Stats stats;
};

// Helper to find memory type
static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, 
                                VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && 
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    
    return 0;
}

Renderer2D& Renderer2D::instance() {
    static Renderer2D instance;
    return instance;
}

Renderer2D::Renderer2D() : m_impl(std::make_unique<Impl>()) {}

Renderer2D::~Renderer2D() {
    if (m_impl && m_impl->initialized) {
        shutdown();
    }
}

bool Renderer2D::initialize(const Renderer2DConfig& config) {
    if (m_impl->initialized) {
        return true;
    }
    
    if (!VulkanBackend::instance().isInitialized()) {
        std::cerr << "[Renderer2D] VulkanBackend not initialized" << std::endl;
        return false;
    }
    
    m_impl->config = config;
    m_impl->vertices.reserve(config.maxVertices);
    m_impl->indices.reserve(config.maxIndices);
    
    if (!createVertexBuffer()) {
        std::cerr << "[Renderer2D] Failed to create vertex buffer" << std::endl;
        return false;
    }
    
    if (!createIndexBuffer()) {
        std::cerr << "[Renderer2D] Failed to create index buffer" << std::endl;
        return false;
    }
    
    m_impl->initialized = true;
    return true;
}

void Renderer2D::shutdown() {
    if (!m_impl->initialized) {
        return;
    }
    
    VkDevice device = VulkanBackend::instance().getDevice();
    VulkanBackend::instance().waitIdle();
    
    if (m_impl->indexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, m_impl->indexBuffer, nullptr);
        m_impl->indexBuffer = VK_NULL_HANDLE;
    }
    if (m_impl->indexBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, m_impl->indexBufferMemory, nullptr);
        m_impl->indexBufferMemory = VK_NULL_HANDLE;
    }
    if (m_impl->vertexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, m_impl->vertexBuffer, nullptr);
        m_impl->vertexBuffer = VK_NULL_HANDLE;
    }
    if (m_impl->vertexBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, m_impl->vertexBufferMemory, nullptr);
        m_impl->vertexBufferMemory = VK_NULL_HANDLE;
    }
    
    m_impl->vertices.clear();
    m_impl->indices.clear();
    m_impl->initialized = false;
}

bool Renderer2D::isInitialized() const {
    return m_impl->initialized;
}


bool Renderer2D::createVertexBuffer() {
    VkDevice device = VulkanBackend::instance().getDevice();
    VkPhysicalDevice physicalDevice = VulkanBackend::instance().getPhysicalDevice();
    
    VkDeviceSize bufferSize = sizeof(Vertex) * m_impl->config.maxVertices;
    
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &m_impl->vertexBuffer);
    if (result != VK_SUCCESS) {
        return false;
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, m_impl->vertexBuffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    result = vkAllocateMemory(device, &allocInfo, nullptr, &m_impl->vertexBufferMemory);
    if (result != VK_SUCCESS) {
        return false;
    }
    
    vkBindBufferMemory(device, m_impl->vertexBuffer, m_impl->vertexBufferMemory, 0);
    return true;
}

bool Renderer2D::createIndexBuffer() {
    VkDevice device = VulkanBackend::instance().getDevice();
    VkPhysicalDevice physicalDevice = VulkanBackend::instance().getPhysicalDevice();
    
    VkDeviceSize bufferSize = sizeof(uint32_t) * m_impl->config.maxIndices;
    
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    
    VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &m_impl->indexBuffer);
    if (result != VK_SUCCESS) {
        return false;
    }
    
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, m_impl->indexBuffer, &memRequirements);
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    result = vkAllocateMemory(device, &allocInfo, nullptr, &m_impl->indexBufferMemory);
    if (result != VK_SUCCESS) {
        return false;
    }
    
    vkBindBufferMemory(device, m_impl->indexBuffer, m_impl->indexBufferMemory, 0);
    return true;
}

void Renderer2D::updateProjectionMatrix(float width, float height) {
    // Orthographic projection: origin at top-left, Y down
    m_impl->projection = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
}

void Renderer2D::beginBatch(float width, float height) {
    m_impl->vertices.clear();
    m_impl->indices.clear();
    m_impl->vertexCount = 0;
    m_impl->indexCount = 0;
    m_impl->batchActive = true;
    
    updateProjectionMatrix(width, height);
}

void Renderer2D::endBatch() {
    flush();
    m_impl->batchActive = false;
}

void Renderer2D::flush() {
    if (m_impl->vertexCount == 0 || m_impl->indexCount == 0) {
        return;
    }
    
    VkDevice device = VulkanBackend::instance().getDevice();
    VkCommandBuffer cmd = VulkanBackend::instance().getCurrentCommandBuffer();
    
    // Upload vertex data
    void* vertexData;
    vkMapMemory(device, m_impl->vertexBufferMemory, 0, 
                sizeof(Vertex) * m_impl->vertexCount, 0, &vertexData);
    memcpy(vertexData, m_impl->vertices.data(), sizeof(Vertex) * m_impl->vertexCount);
    vkUnmapMemory(device, m_impl->vertexBufferMemory);
    
    // Upload index data
    void* indexData;
    vkMapMemory(device, m_impl->indexBufferMemory, 0, 
                sizeof(uint32_t) * m_impl->indexCount, 0, &indexData);
    memcpy(indexData, m_impl->indices.data(), sizeof(uint32_t) * m_impl->indexCount);
    vkUnmapMemory(device, m_impl->indexBufferMemory);
    
    // Bind pipeline
    VkPipeline pipeline = ShaderSystem::instance().getBasicPipeline2D();
    VkPipelineLayout layout = ShaderSystem::instance().getPipelineLayout2D();
    
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    
    // Set viewport and scissor
    VkExtent2D extent = VulkanBackend::instance().getSwapChainExtent();
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);
    
    // Push constants (projection matrix)
    vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(glm::mat4), &m_impl->projection);
    
    // Bind vertex buffer
    VkBuffer vertexBuffers[] = {m_impl->vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
    
    // Bind index buffer
    vkCmdBindIndexBuffer(cmd, m_impl->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    
    // Draw
    vkCmdDrawIndexed(cmd, m_impl->indexCount, 1, 0, 0, 0);
    
    // Update stats
    m_impl->stats.drawCalls++;
    m_impl->stats.vertexCount += m_impl->vertexCount;
    m_impl->stats.indexCount += m_impl->indexCount;
    
    // Clear batch
    m_impl->vertices.clear();
    m_impl->indices.clear();
    m_impl->vertexCount = 0;
    m_impl->indexCount = 0;
}

void Renderer2D::addVertex(float x, float y, const Color& color, float u, float v) {
    if (m_impl->vertexCount >= m_impl->config.maxVertices) {
        flush();
    }
    
    Vertex vertex;
    vertex.position[0] = x;
    vertex.position[1] = y;
    vertex.color[0] = color.r;
    vertex.color[1] = color.g;
    vertex.color[2] = color.b;
    vertex.color[3] = color.a;
    vertex.texCoord[0] = u;
    vertex.texCoord[1] = v;
    
    m_impl->vertices.push_back(vertex);
    m_impl->vertexCount++;
}

void Renderer2D::addIndex(uint32_t index) {
    if (m_impl->indexCount >= m_impl->config.maxIndices) {
        flush();
    }
    
    m_impl->indices.push_back(index);
    m_impl->indexCount++;
}

void Renderer2D::addTriangleIndices(uint32_t i0, uint32_t i1, uint32_t i2) {
    addIndex(i0);
    addIndex(i1);
    addIndex(i2);
}


void Renderer2D::drawRect(const Rect& rect, const Color& color) {
    uint32_t baseIndex = m_impl->vertexCount;
    
    // Add 4 vertices (top-left, top-right, bottom-right, bottom-left)
    addVertex(rect.x, rect.y, color, 0.0f, 0.0f);
    addVertex(rect.x + rect.width, rect.y, color, 1.0f, 0.0f);
    addVertex(rect.x + rect.width, rect.y + rect.height, color, 1.0f, 1.0f);
    addVertex(rect.x, rect.y + rect.height, color, 0.0f, 1.0f);
    
    // Add 2 triangles (6 indices)
    addTriangleIndices(baseIndex, baseIndex + 1, baseIndex + 2);
    addTriangleIndices(baseIndex, baseIndex + 2, baseIndex + 3);
}

void Renderer2D::drawRectGradient(const Rect& rect,
                                   const Color& topLeft, const Color& topRight,
                                   const Color& bottomRight, const Color& bottomLeft) {
    uint32_t baseIndex = m_impl->vertexCount;
    
    addVertex(rect.x, rect.y, topLeft, 0.0f, 0.0f);
    addVertex(rect.x + rect.width, rect.y, topRight, 1.0f, 0.0f);
    addVertex(rect.x + rect.width, rect.y + rect.height, bottomRight, 1.0f, 1.0f);
    addVertex(rect.x, rect.y + rect.height, bottomLeft, 0.0f, 1.0f);
    
    addTriangleIndices(baseIndex, baseIndex + 1, baseIndex + 2);
    addTriangleIndices(baseIndex, baseIndex + 2, baseIndex + 3);
}

void Renderer2D::drawRoundRect(const Rect& rect, float radius, const Color& color, int segments) {
    // Clamp radius to half the smallest dimension
    float maxRadius = std::min(rect.width, rect.height) / 2.0f;
    radius = std::min(radius, maxRadius);
    
    if (radius <= 0.0f) {
        drawRect(rect, color);
        return;
    }
    
    uint32_t centerIndex = m_impl->vertexCount;
    float cx = rect.x + rect.width / 2.0f;
    float cy = rect.y + rect.height / 2.0f;
    
    // Center vertex
    addVertex(cx, cy, color);
    
    // Generate corner vertices
    float angleStep = (3.14159265f / 2.0f) / segments;
    
    // Corner centers
    float corners[4][2] = {
        {rect.x + radius, rect.y + radius},                           // Top-left
        {rect.x + rect.width - radius, rect.y + radius},              // Top-right
        {rect.x + rect.width - radius, rect.y + rect.height - radius}, // Bottom-right
        {rect.x + radius, rect.y + rect.height - radius}              // Bottom-left
    };
    
    // Starting angles for each corner
    float startAngles[4] = {
        3.14159265f,           // Top-left: 180°
        3.14159265f * 1.5f,    // Top-right: 270°
        0.0f,                  // Bottom-right: 0°
        3.14159265f * 0.5f     // Bottom-left: 90°
    };
    
    uint32_t firstVertexIndex = m_impl->vertexCount;
    
    for (int corner = 0; corner < 4; corner++) {
        for (int i = 0; i <= segments; i++) {
            float angle = startAngles[corner] + i * angleStep;
            float x = corners[corner][0] + radius * std::cos(angle);
            float y = corners[corner][1] + radius * std::sin(angle);
            addVertex(x, y, color);
        }
    }
    
    // Create triangles from center to perimeter
    uint32_t totalPerimeterVertices = 4 * (segments + 1);
    for (uint32_t i = 0; i < totalPerimeterVertices; i++) {
        uint32_t next = (i + 1) % totalPerimeterVertices;
        addTriangleIndices(centerIndex, firstVertexIndex + i, firstVertexIndex + next);
    }
}

void Renderer2D::drawCircle(float cx, float cy, float radius, const Color& color, int segments) {
    drawEllipse(cx, cy, radius, radius, color, segments);
}

void Renderer2D::drawEllipse(float cx, float cy, float rx, float ry, const Color& color, int segments) {
    uint32_t centerIndex = m_impl->vertexCount;
    
    // Center vertex
    addVertex(cx, cy, color);
    
    // Perimeter vertices
    float angleStep = 2.0f * 3.14159265f / segments;
    uint32_t firstPerimeterIndex = m_impl->vertexCount;
    
    for (int i = 0; i < segments; i++) {
        float angle = i * angleStep;
        float x = cx + rx * std::cos(angle);
        float y = cy + ry * std::sin(angle);
        addVertex(x, y, color);
    }
    
    // Create triangles
    for (int i = 0; i < segments; i++) {
        uint32_t next = (i + 1) % segments;
        addTriangleIndices(centerIndex, firstPerimeterIndex + i, firstPerimeterIndex + next);
    }
}

void Renderer2D::drawLine(float x1, float y1, float x2, float y2, const Color& color, float thickness) {
    // Calculate perpendicular direction
    float dx = x2 - x1;
    float dy = y2 - y1;
    float length = std::sqrt(dx * dx + dy * dy);
    
    if (length < 0.0001f) {
        return;
    }
    
    // Normalize and get perpendicular
    float nx = -dy / length * thickness * 0.5f;
    float ny = dx / length * thickness * 0.5f;
    
    uint32_t baseIndex = m_impl->vertexCount;
    
    // Create quad for line
    addVertex(x1 + nx, y1 + ny, color);
    addVertex(x1 - nx, y1 - ny, color);
    addVertex(x2 - nx, y2 - ny, color);
    addVertex(x2 + nx, y2 + ny, color);
    
    addTriangleIndices(baseIndex, baseIndex + 1, baseIndex + 2);
    addTriangleIndices(baseIndex, baseIndex + 2, baseIndex + 3);
}

void Renderer2D::drawPolyline(const std::vector<Point>& points, const Color& color, 
                               float thickness, bool closed) {
    if (points.size() < 2) {
        return;
    }
    
    size_t count = points.size();
    for (size_t i = 0; i < count - 1; i++) {
        drawLine(points[i].x, points[i].y, points[i + 1].x, points[i + 1].y, color, thickness);
    }
    
    if (closed && count > 2) {
        drawLine(points[count - 1].x, points[count - 1].y, points[0].x, points[0].y, color, thickness);
    }
}

void Renderer2D::drawPolygon(const std::vector<Point>& points, const Color& color) {
    if (points.size() < 3) {
        return;
    }
    
    // Simple fan triangulation (works for convex polygons)
    uint32_t baseIndex = m_impl->vertexCount;
    
    for (const auto& point : points) {
        addVertex(point.x, point.y, color);
    }
    
    for (size_t i = 1; i < points.size() - 1; i++) {
        addTriangleIndices(baseIndex, baseIndex + static_cast<uint32_t>(i), 
                          baseIndex + static_cast<uint32_t>(i + 1));
    }
}

void Renderer2D::drawTriangle(const Point& p1, const Point& p2, const Point& p3, const Color& color) {
    uint32_t baseIndex = m_impl->vertexCount;
    
    addVertex(p1.x, p1.y, color);
    addVertex(p2.x, p2.y, color);
    addVertex(p3.x, p3.y, color);
    
    addTriangleIndices(baseIndex, baseIndex + 1, baseIndex + 2);
}

void Renderer2D::drawRectOutline(const Rect& rect, const Color& color, float thickness) {
    // Top
    drawLine(rect.x, rect.y, rect.x + rect.width, rect.y, color, thickness);
    // Right
    drawLine(rect.x + rect.width, rect.y, rect.x + rect.width, rect.y + rect.height, color, thickness);
    // Bottom
    drawLine(rect.x + rect.width, rect.y + rect.height, rect.x, rect.y + rect.height, color, thickness);
    // Left
    drawLine(rect.x, rect.y + rect.height, rect.x, rect.y, color, thickness);
}

void Renderer2D::drawCircleOutline(float cx, float cy, float radius, const Color& color, 
                                    float thickness, int segments) {
    float angleStep = 2.0f * 3.14159265f / segments;
    
    for (int i = 0; i < segments; i++) {
        float angle1 = i * angleStep;
        float angle2 = (i + 1) * angleStep;
        
        float x1 = cx + radius * std::cos(angle1);
        float y1 = cy + radius * std::sin(angle1);
        float x2 = cx + radius * std::cos(angle2);
        float y2 = cy + radius * std::sin(angle2);
        
        drawLine(x1, y1, x2, y2, color, thickness);
    }
}

void Renderer2D::drawTexturedRect(const Rect& dstRect, std::shared_ptr<Texture> texture,
                                   const Rect& srcRect, const Color& tint) {
    if (!texture) return;
    
    // Calculate normalized texture coordinates
    float texWidth = static_cast<float>(texture->getWidth());
    float texHeight = static_cast<float>(texture->getHeight());
    
    float u0 = srcRect.x / texWidth;
    float v0 = srcRect.y / texHeight;
    float u1 = (srcRect.x + srcRect.width) / texWidth;
    float v1 = (srcRect.y + srcRect.height) / texHeight;
    
    uint32_t baseIndex = m_impl->vertexCount;
    
    // Add 4 vertices with texture coordinates
    addVertex(dstRect.x, dstRect.y, tint, u0, v0);
    addVertex(dstRect.x + dstRect.width, dstRect.y, tint, u1, v0);
    addVertex(dstRect.x + dstRect.width, dstRect.y + dstRect.height, tint, u1, v1);
    addVertex(dstRect.x, dstRect.y + dstRect.height, tint, u0, v1);
    
    // Add 2 triangles
    addTriangleIndices(baseIndex, baseIndex + 1, baseIndex + 2);
    addTriangleIndices(baseIndex, baseIndex + 2, baseIndex + 3);
    
    // Note: In a full implementation, we would bind the texture here
    // For now, this sets up the geometry with texture coordinates
}

void Renderer2D::drawTexturedRect(const Rect& dstRect, std::shared_ptr<Texture> texture,
                                   const Color& tint) {
    if (!texture) return;
    
    Rect srcRect(0, 0, static_cast<float>(texture->getWidth()), 
                 static_cast<float>(texture->getHeight()));
    drawTexturedRect(dstRect, texture, srcRect, tint);
}

Renderer2D::Stats Renderer2D::getStats() const {
    return m_impl->stats;
}

void Renderer2D::resetStats() {
    m_impl->stats = Stats{};
}

} // namespace KillerGK
