#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

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

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
        glm::vec2 position;
        glm::vec3 color;

        static vk::VertexInputBindingDescription getBindingDescription()
        {
            vk::VertexInputBindingDescription bindingDescription = vk::VertexInputBindingDescription()
                                                                       .setBinding(0)
                                                                       .setStride(sizeof(Vertex))
                                                                       .setInputRate(vk::VertexInputRate::eVertex);

            return bindingDescription;
        }

        static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions()
        {
            std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions{};

            attributeDescriptions[0]
                .setBinding(0)
                .setLocation(0)
                .setFormat(vk::Format::eR32G32Sfloat)
                .setOffset(offsetof(Vertex, position));

            attributeDescriptions[1]
                .setBinding(0)
                .setLocation(1)
                .setFormat(vk::Format::eR32G32B32Sfloat)
                .setOffset(offsetof(Vertex, color));

            return attributeDescriptions;
        }
    };

    struct UniformBufferObject 
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
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

    void createUniformBuffers();
    void updateUniformBuffer(uint32_t currentImages);

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

    const std::vector<Vertex> vertices = {
        // Position (x, y) and Color (RGB)
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };
    
    vk::Buffer vertexBuffer;
    vk::DeviceMemory vertexBufferMemory;
    vk::Buffer indexBuffer;
    vk::DeviceMemory indexBufferMemory;

    std::vector<vk::Buffer> uniformBuffers;
    std::vector<vk::DeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
};