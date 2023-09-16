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
};