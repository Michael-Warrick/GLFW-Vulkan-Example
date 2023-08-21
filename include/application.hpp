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
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();

    GLFWwindow* window = nullptr;   

    // INSTANCE
    vk::Instance instance;
    vk::InstanceCreateFlags flags;
    vk::ApplicationInfo appInfo{};
    vk::InstanceCreateInfo createInfo{};

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = nullptr;

    std::vector<const char*> requiredExtensions;

    uint32_t extensionCount = 0;
    std::vector<vk::ExtensionProperties> availableExtensions;

    // VALIDATION
    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    uint32_t layerCount = 0;
    std::vector<vk::LayerProperties> availableLayers;
};