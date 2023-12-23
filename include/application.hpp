#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include "stb_image/stb_image.h"
#include "tiny_obj_loader/tiny_obj_loader.h"

#include <iostream>
#include <exception>
#include <stdexcept>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>
#include <fstream>
#include <array>
#include <chrono>
#include <unordered_map>

class Application
{
public:
    void Run();

private:
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec2 textureCoordinates;

        static vk::VertexInputBindingDescription getBindingDescription()
        {
            vk::VertexInputBindingDescription bindingDescription = vk::VertexInputBindingDescription()
                                                                       .setBinding(0)
                                                                       .setStride(sizeof(Vertex))
                                                                       .setInputRate(vk::VertexInputRate::eVertex);

            return bindingDescription;
        }

        static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions()
        {
            std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions{};

            attributeDescriptions[0]
                .setBinding(0)
                .setLocation(0)
                .setFormat(vk::Format::eR32G32B32Sfloat)
                .setOffset(offsetof(Vertex, position));

            attributeDescriptions[1]
                .setBinding(0)
                .setLocation(1)
                .setFormat(vk::Format::eR32G32B32Sfloat)
                .setOffset(offsetof(Vertex, color));

            attributeDescriptions[2]
            .setBinding(0)
            .setLocation(2)
            .setFormat(vk::Format::eR32G32Sfloat)
            .setOffset(offsetof(Vertex, textureCoordinates));

            return attributeDescriptions;
        }

        bool operator==(const Vertex& other) const
        {
            return position == other.position && color == other.color && textureCoordinates == other.textureCoordinates;
        }

        friend struct std::hash<Vertex>;
    };

    struct VertexHasher {
        size_t operator()(Vertex const& vertex) const {
            return ((std::hash<glm::vec3>()(vertex.position) ^
                     (std::hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (std::hash<glm::vec2>()(vertex.textureCoordinates) << 1);
        }
    };

    struct UniformBufferObject 
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 projection;
    };

    void init();
    void update();
    void shutdown();

    void initWindow();
    void initVulkan();

    void createVulkanInstance();

    std::vector<const char *> getRequiredInstanceExtensions();
    std::vector<vk::ExtensionProperties> getAvailableInstanceExtensions();
    bool checkInstanceExtensionSupport();
    bool checkValidationLayerSupport();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData);

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createDebugInfo);
    void setupDebugMessenger();

    void pickPhysicalDevice();
    bool isDeviceSuitable(vk::PhysicalDevice device);

    QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);

    void createLogicalDevice();

    void createSurface();

    bool checkDeviceExtensionSupport(vk::PhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);
    void createSwapChain();

    void createImageViews();

    void createGraphicsPipeline();
    static std::vector<char> readFile(const std::string &fileName);
    vk::ShaderModule createShaderModule(const std::vector<char> &code);

    void createRenderPass();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex);

    void drawFrame();

    void createSyncObjects();

    void recreateSwapChain();
    void cleanupSwapChain();

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

    void createVertexBuffer();
    void createIndexBuffer();
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

    void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory);
    void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSets();

    void createUniformBuffers();
    void updateUniformBuffer(uint32_t currentImages);

    void createTextureImage();
    void createTextureImageView();
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image &image, vk::DeviceMemory &imageMemory);
    vk::CommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

    void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);
    void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);

    vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);
    void createTextureSampler();

    void createDepthResources();
    vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
    vk::Format findDepthFormat();
    bool hasStencilComponent(vk::Format format);

    void loadModel();

    void generateMipmaps(vk::Image image, vk::Format imageFormat, int32_t textureWidth, int32_t textureHeight, uint32_t mipLevels);

    vk::SampleCountFlagBits getMaxUsableSampleCount();
    void createColorResources();

    const int MAX_FRAMES_IN_FLIGHT = 2;
    GLFWwindow *window = nullptr;

    vk::Instance instance;
    vk::InstanceCreateFlags flags;
    vk::ApplicationInfo appInfo{};
    vk::InstanceCreateInfo createInfo{};

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = nullptr;
    std::vector<const char *> requiredExtensions;
    uint32_t extensionCount = 0;
    std::vector<vk::ExtensionProperties> availableExtensions;

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    uint32_t layerCount = 0;
    std::vector<vk::LayerProperties> availableLayers;

    VkDebugUtilsMessengerEXT debugMessenger;
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

    vk::PhysicalDevice physicalDevice;
    uint32_t physicalDeviceCount = 0;
    std::vector<vk::PhysicalDevice> physicalDevices;
    vk::PhysicalDeviceProperties physicalDeviceProperties;
    vk::PhysicalDeviceFeatures physicalDeviceFeatures{};

    vk::Device logicalDevice;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo{};
    vk::DeviceCreateInfo logicalDeviceCreateInfo{};
    vk::Queue graphicsQueue;

#ifdef __APPLE__
    const std::vector<const char *> logicalDeviceExtensions = {"VK_KHR_portability_subset", "VK_KHR_swapchain"};
#else
    const std::vector<const char *> logicalDeviceExtensions = {"VK_KHR_swapchain"};
#endif

    vk::SurfaceKHR surface;
    vk::Queue presentQueue;

    vk::SwapchainKHR swapChain;
    std::vector<vk::Image> swapChainImages;
    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;

    std::vector<vk::ImageView> swapChainImageViews;

    vk::RenderPass renderPass;

    vk::DescriptorSetLayout descriptorSetLayout;
    vk::PipelineLayout pipelineLayout;

    vk::Pipeline graphicsPipeline;

    std::vector<vk::Framebuffer> swapChainFrameBuffers;

    vk::CommandPool commandPool;
    std::vector<vk::CommandBuffer> commandBuffers;

    std::vector<vk::Semaphore> imageAvailableSemaphores;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    std::vector<vk::Fence> inFlightFences;

    uint32_t currentFrame = 0;

    bool framebufferResized = false;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    vk::Buffer vertexBuffer;
    vk::DeviceMemory vertexBufferMemory;
    vk::Buffer indexBuffer;
    vk::DeviceMemory indexBufferMemory;

    std::vector<vk::Buffer> uniformBuffers;
    std::vector<vk::DeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    vk::DescriptorPool descriptorPool;
    std::vector<vk::DescriptorSet> descriptorSets;

    uint32_t mipLevels;
    vk::Image textureImage;
    vk::DeviceMemory textureImageMemory;
    vk::ImageView textureImageView;
    vk::Sampler textureSampler;

    vk::Image colorImage;
    vk::DeviceMemory colorImageMemory;
    vk::ImageView colorImageView;

    vk::Image depthImage;
    vk::DeviceMemory depthImageMemory;
    vk::ImageView depthImageView;

    const std::string MODEL_PATH = "resources/models/viking_room/viking_room.obj";
    const std::string TEXTURE_PATH = "resources/models/viking_room/viking_room.png";

    vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;
};