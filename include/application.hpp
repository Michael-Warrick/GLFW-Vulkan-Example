#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <iostream>
#include <exception>
#include <stdexcept>

class Application
{
public:
    void Run();
private:
    void initWindow();
    void initVulkan();
    void update();
    void shutdown();

    void createVulkanInstance();

    std::vector<const char*> getRequiredInstanceExtensions();
    std::vector<vk::ExtensionProperties> getAvailableInstanceExtensions();
    bool checkInstanceExtensionSupport();
    bool checkValidationLayerSupport();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        vk::DebugReportFlagsEXT messageSeverity, 
        vk::DebugReportObjectTypeEXT messageType,
        const vk::DebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData
    );

    void setupDebugMessenger();
    void destroyDebugMessenger();
    
    GLFWwindow* window = nullptr;   

    vk::Instance instance;
    vk::InstanceCreateFlags flags;
    vk::ApplicationInfo appInfo{};
    vk::InstanceCreateInfo createInfo{};

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = nullptr;
    std::vector<const char*> requiredExtensions;
    uint32_t extensionCount = 0;
    std::vector<vk::ExtensionProperties> availableExtensions;

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    uint32_t layerCount = 0;
    std::vector<vk::LayerProperties> availableLayers;

    vk::DebugUtilsMessengerEXT debugMessenger;
    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
};