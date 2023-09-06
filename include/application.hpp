#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <iostream>
#include <exception>
#include <stdexcept>
#include <optional>
#include <set>

class Application
{
public:
    void Run();

private:
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
        void *pUserData
    );

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createDebugInfo);
    void setupDebugMessenger();

    void pickPhysicalDevice();
    bool isDeviceSuitable(vk::PhysicalDevice device);

    struct QueueFamilyIndices 
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() 
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);

    void createLogicalDevice();

    void createSurface();

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
    const std::vector<const char*> logicalDeviceExtensions = {"VK_KHR_portability_subset"};

    vk::SurfaceKHR surface;
    vk::Queue presentQueue;
};