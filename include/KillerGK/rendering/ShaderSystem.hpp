/**
 * @file ShaderSystem.hpp
 * @brief Shader loading, compilation, and pipeline management for KillerGK
 */

#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace KillerGK {

/**
 * @brief Shader stage types
 */
enum class ShaderStage {
    Vertex,
    Fragment,
    Geometry,
    Compute
};

/**
 * @brief Shader module wrapper
 */
struct ShaderModule {
    VkShaderModule module = VK_NULL_HANDLE;
    ShaderStage stage = ShaderStage::Vertex;
    std::string entryPoint = "main";
};

/**
 * @brief Pipeline configuration for 2D rendering
 */
struct Pipeline2DConfig {
    bool enableBlending = true;
    bool enableDepthTest = false;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    float lineWidth = 1.0f;
    VkCullModeFlags cullMode = VK_CULL_MODE_NONE;
};

/**
 * @brief Vertex input description for 2D rendering
 */
struct Vertex2D {
    float position[2];  // x, y
    float color[4];     // r, g, b, a
    float texCoord[2];  // u, v
    
    static VkVertexInputBindingDescription getBindingDescription();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
};


/**
 * @class ShaderSystem
 * @brief Manages shader loading, compilation, and pipeline creation
 */
class ShaderSystem {
public:
    /**
     * @brief Get the singleton instance
     */
    static ShaderSystem& instance();
    
    /**
     * @brief Initialize the shader system
     * @return true if initialization succeeded
     */
    bool initialize();
    
    /**
     * @brief Shutdown and cleanup
     */
    void shutdown();
    
    /**
     * @brief Check if initialized
     */
    [[nodiscard]] bool isInitialized() const;
    
    /**
     * @brief Load a shader from SPIR-V file
     * @param path Path to the .spv file
     * @param stage Shader stage
     * @return Shader module, or empty if failed
     */
    ShaderModule loadShader(const std::string& path, ShaderStage stage);
    
    /**
     * @brief Load a shader from SPIR-V binary data
     * @param data SPIR-V binary data
     * @param stage Shader stage
     * @return Shader module, or empty if failed
     */
    ShaderModule loadShaderFromMemory(const std::vector<uint32_t>& data, ShaderStage stage);
    
    /**
     * @brief Destroy a shader module
     */
    void destroyShader(ShaderModule& shader);
    
    /**
     * @brief Create a graphics pipeline for 2D rendering
     * @param vertexShader Vertex shader module
     * @param fragmentShader Fragment shader module
     * @param config Pipeline configuration
     * @return Pipeline handle, or VK_NULL_HANDLE on failure
     */
    VkPipeline createPipeline2D(const ShaderModule& vertexShader, 
                                 const ShaderModule& fragmentShader,
                                 const Pipeline2DConfig& config = Pipeline2DConfig{});
    
    /**
     * @brief Destroy a pipeline
     */
    void destroyPipeline(VkPipeline pipeline);
    
    /**
     * @brief Get the pipeline layout for 2D rendering
     */
    [[nodiscard]] VkPipelineLayout getPipelineLayout2D() const;
    
    /**
     * @brief Get the descriptor set layout for 2D rendering
     */
    [[nodiscard]] VkDescriptorSetLayout getDescriptorSetLayout2D() const;
    
    /**
     * @brief Get the built-in basic 2D pipeline
     */
    [[nodiscard]] VkPipeline getBasicPipeline2D() const;
    
    /**
     * @brief Get the pipeline cache
     */
    [[nodiscard]] VkPipelineCache getPipelineCache() const;
    
    /**
     * @brief Save pipeline cache to file
     */
    bool savePipelineCache(const std::string& path);
    
    /**
     * @brief Load pipeline cache from file
     */
    bool loadPipelineCache(const std::string& path);

private:
    ShaderSystem();
    ~ShaderSystem();
    ShaderSystem(const ShaderSystem&) = delete;
    ShaderSystem& operator=(const ShaderSystem&) = delete;
    
    bool createPipelineLayout();
    bool createDescriptorSetLayout();
    bool createPipelineCache();
    bool createBuiltInShaders();
    bool createBuiltInPipelines();
    
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace KillerGK
