/**
 * @file VulkanBackend.cpp
 * @brief Vulkan rendering backend implementation
 */

#include "KillerGK/rendering/VulkanBackend.hpp"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <set>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <limits>

namespace KillerGK {

// Validation layers
static const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

// Required device extensions
static const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Debug callback
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        std::cerr << "[Vulkan] " << pCallbackData->pMessage << std::endl;
    }
    return VK_FALSE;
}

// Helper to load debug messenger functions
static VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
    
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator) {
    
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}


/**
 * @brief Implementation details for VulkanBackend
 */
struct VulkanBackend::Impl {
    // Configuration
    VulkanConfig config;
    bool initialized = false;
    
    // Instance and debug
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    
    // Surface
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    
    // Device
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties deviceProperties{};
    std::vector<std::string> supportedExtensions;
    
    // Queues
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    QueueFamilyIndices queueFamilyIndices;
    
    // Command pool
    VkCommandPool commandPool = VK_NULL_HANDLE;
    
    // Swap chain
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D swapChainExtent{};
    std::vector<VkImageView> swapChainImageViews;
    
    // Render pass and framebuffers
    VkRenderPass renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers;
    
    // Synchronization
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    
    // Command buffers
    std::vector<VkCommandBuffer> commandBuffers;
    
    // Frame tracking
    uint32_t currentFrame = 0;
    uint32_t currentImageIndex = 0;
    bool framebufferResized = false;
};

VulkanBackend& VulkanBackend::instance() {
    static VulkanBackend instance;
    return instance;
}

VulkanBackend::VulkanBackend() : m_impl(std::make_unique<Impl>()) {}

VulkanBackend::~VulkanBackend() {
    if (m_impl && m_impl->initialized) {
        shutdown();
    }
}

bool VulkanBackend::initialize(const VulkanConfig& config) {
    if (m_impl->initialized) {
        return true;
    }
    
    m_impl->config = config;
    
    // Initialize GLFW for Vulkan
    if (!glfwInit()) {
        std::cerr << "[VulkanBackend] Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    if (!glfwVulkanSupported()) {
        std::cerr << "[VulkanBackend] Vulkan not supported by GLFW" << std::endl;
        return false;
    }
    
    // Create Vulkan instance
    if (!createInstance()) {
        std::cerr << "[VulkanBackend] Failed to create Vulkan instance" << std::endl;
        return false;
    }
    
    // Setup debug messenger
    if (config.enableValidationLayers && config.enableDebugMessenger) {
        if (!setupDebugMessenger()) {
            std::cerr << "[VulkanBackend] Failed to setup debug messenger" << std::endl;
            // Non-fatal, continue
        }
    }
    
    m_impl->initialized = true;
    return true;
}

bool VulkanBackend::createInstance() {
    // Check validation layer support
    if (m_impl->config.enableValidationLayers && !checkValidationLayerSupport()) {
        std::cerr << "[VulkanBackend] Validation layers requested but not available" << std::endl;
        m_impl->config.enableValidationLayers = false;
    }
    
    // Application info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = m_impl->config.applicationName.c_str();
    appInfo.applicationVersion = m_impl->config.applicationVersion;
    appInfo.pEngineName = "KillerGK";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = m_impl->config.apiVersion;
    
    // Instance create info
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    
    // Get required extensions
    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    
    // Debug messenger create info for instance creation/destruction
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (m_impl->config.enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = 
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;
        createInfo.pNext = &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }
    
    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_impl->instance);
    if (result != VK_SUCCESS) {
        std::cerr << "[VulkanBackend] vkCreateInstance failed: " << result << std::endl;
        return false;
    }
    
    return true;
}


bool VulkanBackend::setupDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = 
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
    
    VkResult result = CreateDebugUtilsMessengerEXT(
        m_impl->instance, &createInfo, nullptr, &m_impl->debugMessenger);
    return result == VK_SUCCESS;
}

bool VulkanBackend::createSurface(void* windowHandle) {
    GLFWwindow* window = static_cast<GLFWwindow*>(windowHandle);
    VkResult result = glfwCreateWindowSurface(m_impl->instance, window, nullptr, &m_impl->surface);
    if (result != VK_SUCCESS) {
        std::cerr << "[VulkanBackend] Failed to create window surface: " << result << std::endl;
        return false;
    }
    
    // Now we can pick physical device and create logical device
    if (!pickPhysicalDevice()) {
        std::cerr << "[VulkanBackend] Failed to pick physical device" << std::endl;
        return false;
    }
    
    if (!createLogicalDevice()) {
        std::cerr << "[VulkanBackend] Failed to create logical device" << std::endl;
        return false;
    }
    
    if (!createCommandPool()) {
        std::cerr << "[VulkanBackend] Failed to create command pool" << std::endl;
        return false;
    }
    
    return true;
}

bool VulkanBackend::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_impl->instance, &deviceCount, nullptr);
    
    if (deviceCount == 0) {
        std::cerr << "[VulkanBackend] No GPUs with Vulkan support found" << std::endl;
        return false;
    }
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_impl->instance, &deviceCount, devices.data());
    
    // Find a suitable device
    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            m_impl->physicalDevice = device;
            break;
        }
    }
    
    if (m_impl->physicalDevice == VK_NULL_HANDLE) {
        std::cerr << "[VulkanBackend] No suitable GPU found" << std::endl;
        return false;
    }
    
    // Get device properties
    vkGetPhysicalDeviceProperties(m_impl->physicalDevice, &m_impl->deviceProperties);
    std::cout << "[VulkanBackend] Selected GPU: " << m_impl->deviceProperties.deviceName << std::endl;
    
    // Cache supported extensions
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(m_impl->physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(m_impl->physicalDevice, nullptr, &extensionCount, availableExtensions.data());
    
    for (const auto& ext : availableExtensions) {
        m_impl->supportedExtensions.push_back(ext.extensionName);
    }
    
    // Store queue family indices
    m_impl->queueFamilyIndices = findQueueFamilies(m_impl->physicalDevice);
    
    return true;
}

bool VulkanBackend::isDeviceSuitable(VkPhysicalDevice device) const {
    QueueFamilyIndices indices = findQueueFamilies(device);
    
    bool extensionsSupported = checkDeviceExtensionSupport(device);
    
    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }
    
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
    
    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool VulkanBackend::checkDeviceExtensionSupport(VkPhysicalDevice device) const {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
    
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }
    
    return requiredExtensions.empty();
}

QueueFamilyIndices VulkanBackend::findQueueFamilies(VkPhysicalDevice device) const {
    QueueFamilyIndices indices;
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    
    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_impl->surface, &presentSupport);
        
        if (presentSupport) {
            indices.presentFamily = i;
        }
        
        if (indices.isComplete()) {
            break;
        }
        
        i++;
    }
    
    return indices;
}


bool VulkanBackend::createLogicalDevice() {
    QueueFamilyIndices indices = m_impl->queueFamilyIndices;
    
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value()
    };
    
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    
    // Device features
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.fillModeNonSolid = VK_TRUE;  // For wireframe rendering
    deviceFeatures.wideLines = VK_TRUE;         // For line width > 1
    
    // Create device
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    
    // Validation layers (deprecated but still set for compatibility)
    if (m_impl->config.enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }
    
    VkResult result = vkCreateDevice(m_impl->physicalDevice, &createInfo, nullptr, &m_impl->device);
    if (result != VK_SUCCESS) {
        std::cerr << "[VulkanBackend] Failed to create logical device: " << result << std::endl;
        return false;
    }
    
    // Get queue handles
    vkGetDeviceQueue(m_impl->device, indices.graphicsFamily.value(), 0, &m_impl->graphicsQueue);
    vkGetDeviceQueue(m_impl->device, indices.presentFamily.value(), 0, &m_impl->presentQueue);
    
    return true;
}

bool VulkanBackend::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_impl->queueFamilyIndices.graphicsFamily.value();
    
    VkResult result = vkCreateCommandPool(m_impl->device, &poolInfo, nullptr, &m_impl->commandPool);
    if (result != VK_SUCCESS) {
        std::cerr << "[VulkanBackend] Failed to create command pool: " << result << std::endl;
        return false;
    }
    
    return true;
}

bool VulkanBackend::createSwapChain(uint32_t width, uint32_t height) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_impl->physicalDevice);
    
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, width, height);
    
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && 
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_impl->surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    QueueFamilyIndices indices = m_impl->queueFamilyIndices;
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    
    VkResult result = vkCreateSwapchainKHR(m_impl->device, &createInfo, nullptr, &m_impl->swapChain);
    if (result != VK_SUCCESS) {
        std::cerr << "[VulkanBackend] Failed to create swap chain: " << result << std::endl;
        return false;
    }
    
    // Get swap chain images
    vkGetSwapchainImagesKHR(m_impl->device, m_impl->swapChain, &imageCount, nullptr);
    m_impl->swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_impl->device, m_impl->swapChain, &imageCount, m_impl->swapChainImages.data());
    
    m_impl->swapChainImageFormat = surfaceFormat.format;
    m_impl->swapChainExtent = extent;
    
    // Create image views
    m_impl->swapChainImageViews.resize(m_impl->swapChainImages.size());
    for (size_t i = 0; i < m_impl->swapChainImages.size(); i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_impl->swapChainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_impl->swapChainImageFormat;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        
        result = vkCreateImageView(m_impl->device, &viewInfo, nullptr, &m_impl->swapChainImageViews[i]);
        if (result != VK_SUCCESS) {
            std::cerr << "[VulkanBackend] Failed to create image view: " << result << std::endl;
            return false;
        }
    }
    
    // Create render pass and framebuffers
    if (!createRenderPass()) {
        return false;
    }
    
    if (!createFramebuffers()) {
        return false;
    }
    
    if (!createSyncObjects()) {
        return false;
    }
    
    return true;
}


bool VulkanBackend::createRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_impl->swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
    VkResult result = vkCreateRenderPass(m_impl->device, &renderPassInfo, nullptr, &m_impl->renderPass);
    if (result != VK_SUCCESS) {
        std::cerr << "[VulkanBackend] Failed to create render pass: " << result << std::endl;
        return false;
    }
    
    return true;
}

bool VulkanBackend::createFramebuffers() {
    m_impl->framebuffers.resize(m_impl->swapChainImageViews.size());
    
    for (size_t i = 0; i < m_impl->swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            m_impl->swapChainImageViews[i]
        };
        
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_impl->renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_impl->swapChainExtent.width;
        framebufferInfo.height = m_impl->swapChainExtent.height;
        framebufferInfo.layers = 1;
        
        VkResult result = vkCreateFramebuffer(m_impl->device, &framebufferInfo, nullptr, &m_impl->framebuffers[i]);
        if (result != VK_SUCCESS) {
            std::cerr << "[VulkanBackend] Failed to create framebuffer: " << result << std::endl;
            return false;
        }
    }
    
    return true;
}

bool VulkanBackend::createSyncObjects() {
    uint32_t maxFrames = m_impl->config.maxFramesInFlight;
    
    m_impl->imageAvailableSemaphores.resize(maxFrames);
    m_impl->renderFinishedSemaphores.resize(maxFrames);
    m_impl->inFlightFences.resize(maxFrames);
    m_impl->commandBuffers.resize(maxFrames);
    
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    // Allocate command buffers
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_impl->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = maxFrames;
    
    VkResult result = vkAllocateCommandBuffers(m_impl->device, &allocInfo, m_impl->commandBuffers.data());
    if (result != VK_SUCCESS) {
        std::cerr << "[VulkanBackend] Failed to allocate command buffers: " << result << std::endl;
        return false;
    }
    
    for (uint32_t i = 0; i < maxFrames; i++) {
        result = vkCreateSemaphore(m_impl->device, &semaphoreInfo, nullptr, &m_impl->imageAvailableSemaphores[i]);
        if (result != VK_SUCCESS) {
            std::cerr << "[VulkanBackend] Failed to create semaphore: " << result << std::endl;
            return false;
        }
        
        result = vkCreateSemaphore(m_impl->device, &semaphoreInfo, nullptr, &m_impl->renderFinishedSemaphores[i]);
        if (result != VK_SUCCESS) {
            std::cerr << "[VulkanBackend] Failed to create semaphore: " << result << std::endl;
            return false;
        }
        
        result = vkCreateFence(m_impl->device, &fenceInfo, nullptr, &m_impl->inFlightFences[i]);
        if (result != VK_SUCCESS) {
            std::cerr << "[VulkanBackend] Failed to create fence: " << result << std::endl;
            return false;
        }
    }
    
    return true;
}


SwapChainSupportDetails VulkanBackend::querySwapChainSupport(VkPhysicalDevice device) const {
    SwapChainSupportDetails details;
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_impl->surface, &details.capabilities);
    
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_impl->surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_impl->surface, &formatCount, details.formats.data());
    }
    
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_impl->surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_impl->surface, &presentModeCount, details.presentModes.data());
    }
    
    return details;
}

VkSurfaceFormatKHR VulkanBackend::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) const {
    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && 
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return formats[0];
}

VkPresentModeKHR VulkanBackend::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& modes) const {
    for (const auto& mode : modes) {
        if (mode == m_impl->config.preferredPresentMode) {
            return mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;  // Guaranteed to be available
}

VkExtent2D VulkanBackend::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, 
                                           uint32_t width, uint32_t height) const {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    
    VkExtent2D actualExtent = {width, height};
    actualExtent.width = std::clamp(actualExtent.width, 
        capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, 
        capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    
    return actualExtent;
}

bool VulkanBackend::recreateSwapChain(uint32_t width, uint32_t height) {
    waitIdle();
    cleanupSwapChain();
    return createSwapChain(width, height);
}

void VulkanBackend::cleanupSwapChain() {
    for (auto framebuffer : m_impl->framebuffers) {
        vkDestroyFramebuffer(m_impl->device, framebuffer, nullptr);
    }
    m_impl->framebuffers.clear();
    
    if (m_impl->renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_impl->device, m_impl->renderPass, nullptr);
        m_impl->renderPass = VK_NULL_HANDLE;
    }
    
    for (auto imageView : m_impl->swapChainImageViews) {
        vkDestroyImageView(m_impl->device, imageView, nullptr);
    }
    m_impl->swapChainImageViews.clear();
    
    if (m_impl->swapChain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(m_impl->device, m_impl->swapChain, nullptr);
        m_impl->swapChain = VK_NULL_HANDLE;
    }
}

int VulkanBackend::beginFrame() {
    uint32_t currentFrame = m_impl->currentFrame;
    
    // Wait for previous frame
    vkWaitForFences(m_impl->device, 1, &m_impl->inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    
    // Acquire next image
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_impl->device, m_impl->swapChain, UINT64_MAX,
        m_impl->imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return -1;  // Need to recreate swap chain
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        std::cerr << "[VulkanBackend] Failed to acquire swap chain image: " << result << std::endl;
        return -1;
    }
    
    vkResetFences(m_impl->device, 1, &m_impl->inFlightFences[currentFrame]);
    
    m_impl->currentImageIndex = imageIndex;
    
    // Reset and begin command buffer
    VkCommandBuffer cmd = m_impl->commandBuffers[currentFrame];
    vkResetCommandBuffer(cmd, 0);
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    
    result = vkBeginCommandBuffer(cmd, &beginInfo);
    if (result != VK_SUCCESS) {
        std::cerr << "[VulkanBackend] Failed to begin command buffer: " << result << std::endl;
        return -1;
    }
    
    // Begin render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_impl->renderPass;
    renderPassInfo.framebuffer = m_impl->framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_impl->swapChainExtent;
    
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;
    
    vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    return static_cast<int>(imageIndex);
}

bool VulkanBackend::endFrame() {
    uint32_t currentFrame = m_impl->currentFrame;
    VkCommandBuffer cmd = m_impl->commandBuffers[currentFrame];
    
    vkCmdEndRenderPass(cmd);
    
    VkResult result = vkEndCommandBuffer(cmd);
    if (result != VK_SUCCESS) {
        std::cerr << "[VulkanBackend] Failed to end command buffer: " << result << std::endl;
        return false;
    }
    
    // Submit command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    
    VkSemaphore waitSemaphores[] = {m_impl->imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    
    VkSemaphore signalSemaphores[] = {m_impl->renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    result = vkQueueSubmit(m_impl->graphicsQueue, 1, &submitInfo, m_impl->inFlightFences[currentFrame]);
    if (result != VK_SUCCESS) {
        std::cerr << "[VulkanBackend] Failed to submit draw command buffer: " << result << std::endl;
        return false;
    }
    
    // Present
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    
    VkSwapchainKHR swapChains[] = {m_impl->swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &m_impl->currentImageIndex;
    
    result = vkQueuePresentKHR(m_impl->presentQueue, &presentInfo);
    
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        m_impl->framebufferResized = true;
    } else if (result != VK_SUCCESS) {
        std::cerr << "[VulkanBackend] Failed to present swap chain image: " << result << std::endl;
        return false;
    }
    
    m_impl->currentFrame = (currentFrame + 1) % m_impl->config.maxFramesInFlight;
    return true;
}


void VulkanBackend::shutdown() {
    if (!m_impl->initialized) {
        return;
    }
    
    waitIdle();
    
    // Destroy sync objects
    for (size_t i = 0; i < m_impl->config.maxFramesInFlight; i++) {
        if (m_impl->renderFinishedSemaphores.size() > i && m_impl->renderFinishedSemaphores[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(m_impl->device, m_impl->renderFinishedSemaphores[i], nullptr);
        }
        if (m_impl->imageAvailableSemaphores.size() > i && m_impl->imageAvailableSemaphores[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(m_impl->device, m_impl->imageAvailableSemaphores[i], nullptr);
        }
        if (m_impl->inFlightFences.size() > i && m_impl->inFlightFences[i] != VK_NULL_HANDLE) {
            vkDestroyFence(m_impl->device, m_impl->inFlightFences[i], nullptr);
        }
    }
    m_impl->renderFinishedSemaphores.clear();
    m_impl->imageAvailableSemaphores.clear();
    m_impl->inFlightFences.clear();
    
    // Cleanup swap chain
    cleanupSwapChain();
    
    // Destroy command pool
    if (m_impl->commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_impl->device, m_impl->commandPool, nullptr);
        m_impl->commandPool = VK_NULL_HANDLE;
    }
    
    // Destroy device
    if (m_impl->device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_impl->device, nullptr);
        m_impl->device = VK_NULL_HANDLE;
    }
    
    // Destroy surface
    if (m_impl->surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(m_impl->instance, m_impl->surface, nullptr);
        m_impl->surface = VK_NULL_HANDLE;
    }
    
    // Destroy debug messenger
    if (m_impl->debugMessenger != VK_NULL_HANDLE) {
        DestroyDebugUtilsMessengerEXT(m_impl->instance, m_impl->debugMessenger, nullptr);
        m_impl->debugMessenger = VK_NULL_HANDLE;
    }
    
    // Destroy instance
    if (m_impl->instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_impl->instance, nullptr);
        m_impl->instance = VK_NULL_HANDLE;
    }
    
    m_impl->initialized = false;
}

void VulkanBackend::waitIdle() {
    if (m_impl->device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_impl->device);
    }
}

std::vector<const char*> VulkanBackend::getRequiredExtensions() const {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    
    if (m_impl->config.enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    
    return extensions;
}

bool VulkanBackend::checkValidationLayerSupport() const {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    
    for (const char* layerName : validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }
    
    return true;
}

// Getters
bool VulkanBackend::isInitialized() const { return m_impl->initialized; }
VkInstance VulkanBackend::getInstance() const { return m_impl->instance; }
VkPhysicalDevice VulkanBackend::getPhysicalDevice() const { return m_impl->physicalDevice; }
VkDevice VulkanBackend::getDevice() const { return m_impl->device; }
VkQueue VulkanBackend::getGraphicsQueue() const { return m_impl->graphicsQueue; }
VkQueue VulkanBackend::getPresentQueue() const { return m_impl->presentQueue; }
VkCommandPool VulkanBackend::getCommandPool() const { return m_impl->commandPool; }
VkRenderPass VulkanBackend::getRenderPass() const { return m_impl->renderPass; }
VkSwapchainKHR VulkanBackend::getSwapChain() const { return m_impl->swapChain; }
VkExtent2D VulkanBackend::getSwapChainExtent() const { return m_impl->swapChainExtent; }
VkFormat VulkanBackend::getSwapChainImageFormat() const { return m_impl->swapChainImageFormat; }
const std::vector<VkFramebuffer>& VulkanBackend::getFramebuffers() const { return m_impl->framebuffers; }
uint32_t VulkanBackend::getCurrentFrameIndex() const { return m_impl->currentFrame; }
uint32_t VulkanBackend::getCurrentImageIndex() const { return m_impl->currentImageIndex; }

VkCommandBuffer VulkanBackend::getCurrentCommandBuffer() const {
    return m_impl->commandBuffers[m_impl->currentFrame];
}

const VkPhysicalDeviceProperties& VulkanBackend::getDeviceProperties() const {
    return m_impl->deviceProperties;
}

bool VulkanBackend::isExtensionSupported(const std::string& extension) const {
    return std::find(m_impl->supportedExtensions.begin(), 
                     m_impl->supportedExtensions.end(), 
                     extension) != m_impl->supportedExtensions.end();
}

VkCommandBuffer VulkanBackend::allocateCommandBuffer(VkCommandBufferLevel level) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_impl->commandPool;
    allocInfo.level = level;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_impl->device, &allocInfo, &commandBuffer);
    return commandBuffer;
}

void VulkanBackend::freeCommandBuffer(VkCommandBuffer commandBuffer) {
    vkFreeCommandBuffers(m_impl->device, m_impl->commandPool, 1, &commandBuffer);
}

VkCommandBuffer VulkanBackend::beginSingleTimeCommands() {
    VkCommandBuffer commandBuffer = allocateCommandBuffer();
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void VulkanBackend::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    vkQueueSubmit(m_impl->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_impl->graphicsQueue);
    
    freeCommandBuffer(commandBuffer);
}

} // namespace KillerGK
