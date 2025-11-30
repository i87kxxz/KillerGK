/**
 * @file VulkanBackend.hpp
 * @brief Vulkan rendering backend for KillerGK
 * 
 * Provides low-level Vulkan initialization, device management,
 * swap chain handling, and rendering infrastructure.
 */

#pragma once

#include "../core/Types.hpp"
#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <functional>

namespace KillerGK {

// Forward declarations
struct NativeWindowHandle;

/**
 * @brief Queue family indices for Vulkan device
 */
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;
    
    [[nodiscard]] bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

/**
 * @brief Swap chain support details
 */
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

/**
 * @brief Vulkan backend configuration
 */
struct VulkanConfig {
    bool enableValidationLayers = true;
    bool enableDebugMessenger = true;
    std::string applicationName = "KillerGK Application";
    uint32_t applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    uint32_t apiVersion = VK_API_VERSION_1_3;
    VkPresentModeKHR preferredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    uint32_t maxFramesInFlight = 2;
};


/**
 * @class VulkanBackend
 * @brief Manages Vulkan instance, device, and core rendering infrastructure
 * 
 * This class handles:
 * - Vulkan instance creation with validation layers
 * - Physical device selection and logical device creation
 * - Command pools and queues
 * - Swap chain management
 * - Render pass and framebuffer setup
 */
class VulkanBackend {
public:
    /**
     * @brief Get the singleton instance
     */
    static VulkanBackend& instance();
    
    /**
     * @brief Initialize Vulkan with the given configuration
     * @param config Vulkan configuration options
     * @return true if initialization succeeded
     */
    bool initialize(const VulkanConfig& config = VulkanConfig{});
    
    /**
     * @brief Shutdown and cleanup all Vulkan resources
     */
    void shutdown();
    
    /**
     * @brief Check if Vulkan is initialized
     */
    [[nodiscard]] bool isInitialized() const;
    
    /**
     * @brief Create a surface for a window
     * @param windowHandle Native window handle
     * @return true if surface creation succeeded
     */
    bool createSurface(void* windowHandle);
    
    /**
     * @brief Create or recreate the swap chain
     * @param width Window width
     * @param height Window height
     * @return true if swap chain creation succeeded
     */
    bool createSwapChain(uint32_t width, uint32_t height);
    
    /**
     * @brief Recreate swap chain (e.g., after window resize)
     * @param width New width
     * @param height New height
     * @return true if recreation succeeded
     */
    bool recreateSwapChain(uint32_t width, uint32_t height);
    
    /**
     * @brief Begin a new frame
     * @return Index of the acquired image, or -1 on failure
     */
    int beginFrame();
    
    /**
     * @brief End the current frame and present
     * @return true if presentation succeeded
     */
    bool endFrame();
    
    /**
     * @brief Wait for device to be idle
     */
    void waitIdle();
    
    // Getters for Vulkan handles
    [[nodiscard]] VkInstance getInstance() const;
    [[nodiscard]] VkPhysicalDevice getPhysicalDevice() const;
    [[nodiscard]] VkDevice getDevice() const;
    [[nodiscard]] VkQueue getGraphicsQueue() const;
    [[nodiscard]] VkQueue getPresentQueue() const;
    [[nodiscard]] VkCommandPool getCommandPool() const;
    [[nodiscard]] VkRenderPass getRenderPass() const;
    [[nodiscard]] VkSwapchainKHR getSwapChain() const;
    [[nodiscard]] VkExtent2D getSwapChainExtent() const;
    [[nodiscard]] VkFormat getSwapChainImageFormat() const;
    [[nodiscard]] const std::vector<VkFramebuffer>& getFramebuffers() const;
    [[nodiscard]] VkCommandBuffer getCurrentCommandBuffer() const;
    [[nodiscard]] uint32_t getCurrentFrameIndex() const;
    [[nodiscard]] uint32_t getCurrentImageIndex() const;
    
    /**
     * @brief Get device properties
     */
    [[nodiscard]] const VkPhysicalDeviceProperties& getDeviceProperties() const;
    
    /**
     * @brief Check if a device extension is supported
     */
    [[nodiscard]] bool isExtensionSupported(const std::string& extension) const;
    
    /**
     * @brief Allocate a command buffer
     * @param level Command buffer level (primary or secondary)
     * @return Allocated command buffer
     */
    VkCommandBuffer allocateCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    
    /**
     * @brief Free a command buffer
     */
    void freeCommandBuffer(VkCommandBuffer commandBuffer);
    
    /**
     * @brief Begin single-time command buffer
     */
    VkCommandBuffer beginSingleTimeCommands();
    
    /**
     * @brief End and submit single-time command buffer
     */
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

private:
    VulkanBackend();
    ~VulkanBackend();
    VulkanBackend(const VulkanBackend&) = delete;
    VulkanBackend& operator=(const VulkanBackend&) = delete;
    
    // Initialization helpers
    bool createInstance();
    bool setupDebugMessenger();
    bool pickPhysicalDevice();
    bool createLogicalDevice();
    bool createCommandPool();
    bool createSyncObjects();
    bool createRenderPass();
    bool createFramebuffers();
    
    // Query helpers
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;
    bool isDeviceSuitable(VkPhysicalDevice device) const;
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
    std::vector<const char*> getRequiredExtensions() const;
    bool checkValidationLayerSupport() const;
    
    // Swap chain helpers
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) const;
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes) const;
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height) const;
    
    // Cleanup helpers
    void cleanupSwapChain();
    
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace KillerGK
