/**
 * @file ShaderSystem.cpp
 * @brief Shader system implementation
 */

#include "KillerGK/rendering/ShaderSystem.hpp"
#include "KillerGK/rendering/VulkanBackend.hpp"
#include <fstream>
#include <iostream>
#include <array>

namespace KillerGK {

// Built-in vertex shader for 2D rendering (SPIR-V)
// Compiled from:
// #version 450
// layout(location = 0) in vec2 inPosition;
// layout(location = 1) in vec4 inColor;
// layout(location = 2) in vec2 inTexCoord;
// layout(push_constant) uniform PushConstants { mat4 transform; } pc;
// layout(location = 0) out vec4 fragColor;
// layout(location = 1) out vec2 fragTexCoord;
// void main() {
//     gl_Position = pc.transform * vec4(inPosition, 0.0, 1.0);
//     fragColor = inColor;
//     fragTexCoord = inTexCoord;
// }
static const std::vector<uint32_t> builtInVertexShader = {
    0x07230203, 0x00010000, 0x0008000b, 0x00000030, 0x00000000, 0x00020011,
    0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x000a000f, 0x00000000,
    0x00000004, 0x6e69616d, 0x00000000, 0x0000000d, 0x00000012, 0x0000001c,
    0x00000026, 0x00000029, 0x00030003, 0x00000002, 0x000001c2, 0x00040005,
    0x00000004, 0x6e69616d, 0x00000000, 0x00060005, 0x0000000b, 0x505f6c67,
    0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x0000000b, 0x00000000,
    0x505f6c67, 0x7469736f, 0x006e6f69, 0x00070006, 0x0000000b, 0x00000001,
    0x505f6c67, 0x746e696f, 0x657a6953, 0x00000000, 0x00070006, 0x0000000b,
    0x00000002, 0x435f6c67, 0x4470696c, 0x61747369, 0x0065636e, 0x00070006,
    0x0000000b, 0x00000003, 0x435f6c67, 0x446c6c75, 0x61747369, 0x0065636e,
    0x00030005, 0x0000000d, 0x00000000, 0x00060005, 0x00000010, 0x68737550,
    0x736e6f43, 0x746e6174, 0x00000073, 0x00060006, 0x00000010, 0x00000000,
    0x6e617274, 0x726f6673, 0x0000006d, 0x00030005, 0x00000012, 0x00006370,
    0x00050005, 0x00000016, 0x6f506e69, 0x69746973, 0x00006e6f, 0x00050005,
    0x0000001c, 0x67617266, 0x6f6c6f43, 0x00000072, 0x00040005, 0x0000001e,
    0x6f436e69, 0x00726f6c, 0x00060005, 0x00000026, 0x67617266, 0x43786554,
    0x64726f6f, 0x00000000, 0x00050005, 0x00000029, 0x65546e69, 0x6f6f4378,
    0x00006472, 0x00050048, 0x0000000b, 0x00000000, 0x0000000b, 0x00000000,
    0x00050048, 0x0000000b, 0x00000001, 0x0000000b, 0x00000001, 0x00050048,
    0x0000000b, 0x00000002, 0x0000000b, 0x00000003, 0x00050048, 0x0000000b,
    0x00000003, 0x0000000b, 0x00000004, 0x00030047, 0x0000000b, 0x00000002,
    0x00040048, 0x00000010, 0x00000000, 0x00000005, 0x00050048, 0x00000010,
    0x00000000, 0x00000023, 0x00000000, 0x00050048, 0x00000010, 0x00000000,
    0x00000007, 0x00000010, 0x00030047, 0x00000010, 0x00000002, 0x00040047,
    0x00000016, 0x0000001e, 0x00000000, 0x00040047, 0x0000001c, 0x0000001e,
    0x00000000, 0x00040047, 0x0000001e, 0x0000001e, 0x00000001, 0x00040047,
    0x00000026, 0x0000001e, 0x00000001, 0x00040047, 0x00000029, 0x0000001e,
    0x00000002, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002,
    0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006,
    0x00000004, 0x00040015, 0x00000008, 0x00000020, 0x00000000, 0x0004002b,
    0x00000008, 0x00000009, 0x00000001, 0x0004001c, 0x0000000a, 0x00000006,
    0x00000009, 0x0006001e, 0x0000000b, 0x00000007, 0x00000006, 0x0000000a,
    0x0000000a, 0x00040020, 0x0000000c, 0x00000003, 0x0000000b, 0x0004003b,
    0x0000000c, 0x0000000d, 0x00000003, 0x00040015, 0x0000000e, 0x00000020,
    0x00000001, 0x0004002b, 0x0000000e, 0x0000000f, 0x00000000, 0x00040018,
    0x00000011, 0x00000007, 0x00000004, 0x0003001e, 0x00000010, 0x00000011,
    0x00040020, 0x00000013, 0x00000009, 0x00000010, 0x0004003b, 0x00000013,
    0x00000012, 0x00000009, 0x00040020, 0x00000014, 0x00000009, 0x00000011,
    0x00040017, 0x00000015, 0x00000006, 0x00000002, 0x00040020, 0x00000017,
    0x00000001, 0x00000015, 0x0004003b, 0x00000017, 0x00000016, 0x00000001,
    0x0004002b, 0x00000006, 0x00000019, 0x00000000, 0x0004002b, 0x00000006,
    0x0000001a, 0x3f800000, 0x00040020, 0x0000001b, 0x00000003, 0x00000007,
    0x0004003b, 0x0000001b, 0x0000001c, 0x00000003, 0x00040020, 0x0000001d,
    0x00000001, 0x00000007, 0x0004003b, 0x0000001d, 0x0000001e, 0x00000001,
    0x00040020, 0x00000025, 0x00000003, 0x00000015, 0x0004003b, 0x00000025,
    0x00000026, 0x00000003, 0x0004003b, 0x00000017, 0x00000029, 0x00000001,
    0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
    0x00000005, 0x00050041, 0x00000014, 0x00000018, 0x00000012, 0x0000000f,
    0x0004003d, 0x00000011, 0x0000001f, 0x00000018, 0x0004003d, 0x00000015,
    0x00000020, 0x00000016, 0x00050051, 0x00000006, 0x00000021, 0x00000020,
    0x00000000, 0x00050051, 0x00000006, 0x00000022, 0x00000020, 0x00000001,
    0x00070050, 0x00000007, 0x00000023, 0x00000021, 0x00000022, 0x00000019,
    0x0000001a, 0x00050091, 0x00000007, 0x00000024, 0x0000001f, 0x00000023,
    0x00050041, 0x0000001b, 0x00000027, 0x0000000d, 0x0000000f, 0x0003003e,
    0x00000027, 0x00000024, 0x0004003d, 0x00000007, 0x00000028, 0x0000001e,
    0x0003003e, 0x0000001c, 0x00000028, 0x0004003d, 0x00000015, 0x0000002a,
    0x00000029, 0x0003003e, 0x00000026, 0x0000002a, 0x000100fd, 0x00010038
};


// Built-in fragment shader for 2D rendering (SPIR-V)
// Compiled from:
// #version 450
// layout(location = 0) in vec4 fragColor;
// layout(location = 1) in vec2 fragTexCoord;
// layout(location = 0) out vec4 outColor;
// layout(binding = 0) uniform sampler2D texSampler;
// layout(push_constant) uniform PushConstants { mat4 transform; int useTexture; } pc;
// void main() {
//     if (pc.useTexture != 0) {
//         outColor = fragColor * texture(texSampler, fragTexCoord);
//     } else {
//         outColor = fragColor;
//     }
// }
static const std::vector<uint32_t> builtInFragmentShader = {
    0x07230203, 0x00010000, 0x0008000b, 0x00000020, 0x00000000, 0x00020011,
    0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e,
    0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0008000f, 0x00000004,
    0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000b, 0x00000014,
    0x00030010, 0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2,
    0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00050005, 0x00000009,
    0x4374756f, 0x726f6c6f, 0x00000000, 0x00050005, 0x0000000b, 0x67617266,
    0x6f6c6f43, 0x00000072, 0x00050005, 0x00000014, 0x67617266, 0x43786554,
    0x64726f6f, 0x00000000, 0x00040047, 0x00000009, 0x0000001e, 0x00000000,
    0x00040047, 0x0000000b, 0x0000001e, 0x00000000, 0x00040047, 0x00000014,
    0x0000001e, 0x00000001, 0x00020013, 0x00000002, 0x00030021, 0x00000003,
    0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007,
    0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003, 0x00000007,
    0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040020, 0x0000000a,
    0x00000001, 0x00000007, 0x0004003b, 0x0000000a, 0x0000000b, 0x00000001,
    0x00040017, 0x00000013, 0x00000006, 0x00000002, 0x00040020, 0x00000015,
    0x00000001, 0x00000013, 0x0004003b, 0x00000015, 0x00000014, 0x00000001,
    0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8,
    0x00000005, 0x0004003d, 0x00000007, 0x0000000c, 0x0000000b, 0x0003003e,
    0x00000009, 0x0000000c, 0x000100fd, 0x00010038
};

struct ShaderSystem::Impl {
    bool initialized = false;
    
    VkPipelineCache pipelineCache = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout2D = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout2D = VK_NULL_HANDLE;
    
    // Built-in shaders
    ShaderModule builtInVertShader;
    ShaderModule builtInFragShader;
    
    // Built-in pipelines
    VkPipeline basicPipeline2D = VK_NULL_HANDLE;
    
    // Cached pipelines
    std::unordered_map<size_t, VkPipeline> pipelineCache2D;
};

// Vertex2D implementation
VkVertexInputBindingDescription Vertex2D::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex2D);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Vertex2D::getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);
    
    // Position
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex2D, position);
    
    // Color
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex2D, color);
    
    // Texture coordinates
    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex2D, texCoord);
    
    return attributeDescriptions;
}

ShaderSystem& ShaderSystem::instance() {
    static ShaderSystem instance;
    return instance;
}

ShaderSystem::ShaderSystem() : m_impl(std::make_unique<Impl>()) {}

ShaderSystem::~ShaderSystem() {
    if (m_impl && m_impl->initialized) {
        shutdown();
    }
}

bool ShaderSystem::initialize() {
    if (m_impl->initialized) {
        return true;
    }
    
    if (!VulkanBackend::instance().isInitialized()) {
        std::cerr << "[ShaderSystem] VulkanBackend not initialized" << std::endl;
        return false;
    }
    
    if (!createPipelineCache()) {
        std::cerr << "[ShaderSystem] Failed to create pipeline cache" << std::endl;
        return false;
    }
    
    if (!createDescriptorSetLayout()) {
        std::cerr << "[ShaderSystem] Failed to create descriptor set layout" << std::endl;
        return false;
    }
    
    if (!createPipelineLayout()) {
        std::cerr << "[ShaderSystem] Failed to create pipeline layout" << std::endl;
        return false;
    }
    
    if (!createBuiltInShaders()) {
        std::cerr << "[ShaderSystem] Failed to create built-in shaders" << std::endl;
        return false;
    }
    
    if (!createBuiltInPipelines()) {
        std::cerr << "[ShaderSystem] Failed to create built-in pipelines" << std::endl;
        return false;
    }
    
    m_impl->initialized = true;
    return true;
}


void ShaderSystem::shutdown() {
    if (!m_impl->initialized) {
        return;
    }
    
    VkDevice device = VulkanBackend::instance().getDevice();
    
    // Destroy cached pipelines
    for (auto& [hash, pipeline] : m_impl->pipelineCache2D) {
        if (pipeline != VK_NULL_HANDLE) {
            vkDestroyPipeline(device, pipeline, nullptr);
        }
    }
    m_impl->pipelineCache2D.clear();
    
    // Destroy built-in pipeline
    if (m_impl->basicPipeline2D != VK_NULL_HANDLE) {
        vkDestroyPipeline(device, m_impl->basicPipeline2D, nullptr);
        m_impl->basicPipeline2D = VK_NULL_HANDLE;
    }
    
    // Destroy built-in shaders
    destroyShader(m_impl->builtInVertShader);
    destroyShader(m_impl->builtInFragShader);
    
    // Destroy pipeline layout
    if (m_impl->pipelineLayout2D != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device, m_impl->pipelineLayout2D, nullptr);
        m_impl->pipelineLayout2D = VK_NULL_HANDLE;
    }
    
    // Destroy descriptor set layout
    if (m_impl->descriptorSetLayout2D != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device, m_impl->descriptorSetLayout2D, nullptr);
        m_impl->descriptorSetLayout2D = VK_NULL_HANDLE;
    }
    
    // Destroy pipeline cache
    if (m_impl->pipelineCache != VK_NULL_HANDLE) {
        vkDestroyPipelineCache(device, m_impl->pipelineCache, nullptr);
        m_impl->pipelineCache = VK_NULL_HANDLE;
    }
    
    m_impl->initialized = false;
}

bool ShaderSystem::isInitialized() const {
    return m_impl->initialized;
}

bool ShaderSystem::createPipelineCache() {
    VkPipelineCacheCreateInfo cacheInfo{};
    cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    
    VkResult result = vkCreatePipelineCache(
        VulkanBackend::instance().getDevice(), 
        &cacheInfo, nullptr, &m_impl->pipelineCache);
    
    return result == VK_SUCCESS;
}

bool ShaderSystem::createDescriptorSetLayout() {
    // Texture sampler binding
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    
    std::array<VkDescriptorSetLayoutBinding, 1> bindings = {samplerLayoutBinding};
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    
    VkResult result = vkCreateDescriptorSetLayout(
        VulkanBackend::instance().getDevice(),
        &layoutInfo, nullptr, &m_impl->descriptorSetLayout2D);
    
    return result == VK_SUCCESS;
}

bool ShaderSystem::createPipelineLayout() {
    // Push constant for transform matrix
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(float) * 16 + sizeof(int);  // mat4 + useTexture flag
    
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_impl->descriptorSetLayout2D;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    
    VkResult result = vkCreatePipelineLayout(
        VulkanBackend::instance().getDevice(),
        &pipelineLayoutInfo, nullptr, &m_impl->pipelineLayout2D);
    
    return result == VK_SUCCESS;
}

bool ShaderSystem::createBuiltInShaders() {
    m_impl->builtInVertShader = loadShaderFromMemory(builtInVertexShader, ShaderStage::Vertex);
    if (m_impl->builtInVertShader.module == VK_NULL_HANDLE) {
        return false;
    }
    
    m_impl->builtInFragShader = loadShaderFromMemory(builtInFragmentShader, ShaderStage::Fragment);
    if (m_impl->builtInFragShader.module == VK_NULL_HANDLE) {
        return false;
    }
    
    return true;
}

bool ShaderSystem::createBuiltInPipelines() {
    Pipeline2DConfig config;
    config.enableBlending = true;
    
    m_impl->basicPipeline2D = createPipeline2D(
        m_impl->builtInVertShader, 
        m_impl->builtInFragShader, 
        config);
    
    return m_impl->basicPipeline2D != VK_NULL_HANDLE;
}


ShaderModule ShaderSystem::loadShader(const std::string& path, ShaderStage stage) {
    ShaderModule shader;
    shader.stage = stage;
    
    // Read file
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[ShaderSystem] Failed to open shader file: " << path << std::endl;
        return shader;
    }
    
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
    
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    file.close();
    
    return loadShaderFromMemory(buffer, stage);
}

ShaderModule ShaderSystem::loadShaderFromMemory(const std::vector<uint32_t>& data, ShaderStage stage) {
    ShaderModule shader;
    shader.stage = stage;
    
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = data.size() * sizeof(uint32_t);
    createInfo.pCode = data.data();
    
    VkResult result = vkCreateShaderModule(
        VulkanBackend::instance().getDevice(),
        &createInfo, nullptr, &shader.module);
    
    if (result != VK_SUCCESS) {
        std::cerr << "[ShaderSystem] Failed to create shader module: " << result << std::endl;
        shader.module = VK_NULL_HANDLE;
    }
    
    return shader;
}

void ShaderSystem::destroyShader(ShaderModule& shader) {
    if (shader.module != VK_NULL_HANDLE) {
        vkDestroyShaderModule(VulkanBackend::instance().getDevice(), shader.module, nullptr);
        shader.module = VK_NULL_HANDLE;
    }
}

VkPipeline ShaderSystem::createPipeline2D(const ShaderModule& vertexShader,
                                           const ShaderModule& fragmentShader,
                                           const Pipeline2DConfig& config) {
    VkDevice device = VulkanBackend::instance().getDevice();
    
    // Shader stages
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertexShader.module;
    vertShaderStageInfo.pName = vertexShader.entryPoint.c_str();
    
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragmentShader.module;
    fragShaderStageInfo.pName = fragmentShader.entryPoint.c_str();
    
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
    
    // Vertex input
    auto bindingDescription = Vertex2D::getBindingDescription();
    auto attributeDescriptions = Vertex2D::getAttributeDescriptions();
    
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    
    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = config.topology;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    
    // Viewport and scissor (dynamic)
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    
    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = config.polygonMode;
    rasterizer.lineWidth = config.lineWidth;
    rasterizer.cullMode = config.cullMode;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    
    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    // Depth stencil
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = config.enableDepthTest ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = config.enableDepthTest ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    
    // Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = 
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    
    if (config.enableBlending) {
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    } else {
        colorBlendAttachment.blendEnable = VK_FALSE;
    }
    
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    
    // Dynamic state
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
    
    // Create pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_impl->pipelineLayout2D;
    pipelineInfo.renderPass = VulkanBackend::instance().getRenderPass();
    pipelineInfo.subpass = 0;
    
    VkPipeline pipeline;
    VkResult result = vkCreateGraphicsPipelines(
        device, m_impl->pipelineCache, 1, &pipelineInfo, nullptr, &pipeline);
    
    if (result != VK_SUCCESS) {
        std::cerr << "[ShaderSystem] Failed to create graphics pipeline: " << result << std::endl;
        return VK_NULL_HANDLE;
    }
    
    return pipeline;
}

void ShaderSystem::destroyPipeline(VkPipeline pipeline) {
    if (pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(VulkanBackend::instance().getDevice(), pipeline, nullptr);
    }
}

VkPipelineLayout ShaderSystem::getPipelineLayout2D() const {
    return m_impl->pipelineLayout2D;
}

VkDescriptorSetLayout ShaderSystem::getDescriptorSetLayout2D() const {
    return m_impl->descriptorSetLayout2D;
}

VkPipeline ShaderSystem::getBasicPipeline2D() const {
    return m_impl->basicPipeline2D;
}

VkPipelineCache ShaderSystem::getPipelineCache() const {
    return m_impl->pipelineCache;
}

bool ShaderSystem::savePipelineCache(const std::string& path) {
    VkDevice device = VulkanBackend::instance().getDevice();
    
    size_t dataSize;
    vkGetPipelineCacheData(device, m_impl->pipelineCache, &dataSize, nullptr);
    
    std::vector<char> data(dataSize);
    vkGetPipelineCacheData(device, m_impl->pipelineCache, &dataSize, data.data());
    
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file.write(data.data(), dataSize);
    return true;
}

bool ShaderSystem::loadPipelineCache(const std::string& path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> data(fileSize);
    
    file.seekg(0);
    file.read(data.data(), fileSize);
    file.close();
    
    VkDevice device = VulkanBackend::instance().getDevice();
    
    // Destroy old cache
    if (m_impl->pipelineCache != VK_NULL_HANDLE) {
        vkDestroyPipelineCache(device, m_impl->pipelineCache, nullptr);
    }
    
    VkPipelineCacheCreateInfo cacheInfo{};
    cacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    cacheInfo.initialDataSize = fileSize;
    cacheInfo.pInitialData = data.data();
    
    VkResult result = vkCreatePipelineCache(device, &cacheInfo, nullptr, &m_impl->pipelineCache);
    return result == VK_SUCCESS;
}

} // namespace KillerGK
