#include "application.hpp"

void Application::Run()
{
    initWindow();
    initVulkan();
    update();
    shutdown();
}

void Application::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(800, 600, "GLFW Vulkan Example", NULL, NULL);
}

void Application::initVulkan()
{
    createVulkanInstance();
    setupDebugMessenger();
}

void Application::update()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void Application::shutdown()
{
    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    instance.destroy();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void Application::createVulkanInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("Requested Validation Layers unavailable!");
    }

    if (!checkInstanceExtensionSupport())
    {
        throw std::runtime_error("Requested Instance Extensions are unavailable");
    }

    appInfo = vk::ApplicationInfo()
                  .setPApplicationName("GLFW Vulcan Example")
                  .setApplicationVersion(1)
                  .setPEngineName("No Engine")
                  .setEngineVersion(1)
                  .setApiVersion(VK_API_VERSION_1_3);

    createInfo = vk::InstanceCreateInfo()
                     .setFlags(flags)
                     .setPApplicationInfo(&appInfo)
                     .setEnabledLayerCount(validationLayers.size())
                     .setPpEnabledLayerNames(validationLayers.data())
                     .setEnabledExtensionCount(requiredExtensions.size())
                     .setPpEnabledExtensionNames(requiredExtensions.data());

    if (enableValidationLayers)
    {
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.setPNext(&debugCreateInfo);
    }

    try
    {
        instance = vk::createInstance(createInfo);
    }
    catch (const vk::SystemError &err)
    {
        throw std::runtime_error("Failed to create Vulkan instance: " + std::string(err.what()));
    }
}

bool Application::checkValidationLayerSupport()
{
    vk::Result result;

    result = vk::enumerateInstanceLayerProperties(&layerCount, nullptr);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to enumerate intance layer count. Error code: " + vk::to_string(result));
    }

    availableLayers.resize(layerCount);

    result = vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to enumerate intance available layers. Error code: " + vk::to_string(result));
    }

    for (const char *layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    // Returning true as all validation layers were found
    return true;
}

std::vector<const char *> Application::getRequiredInstanceExtensions()
{
    // Retrieving all required GLFW extensions for instance creation
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // Adding all required GLFW extensions to the required extension list
    for (uint32_t i = 0; i < glfwExtensionCount; i++)
    {
        requiredExtensions.emplace_back(glfwExtensions[i]);
    }

    // Adding the portability extension (for MoltenVK driver compatibility issue) + Setting flag
    requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    flags = vk::InstanceCreateFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);

    if (enableValidationLayers)
    {
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        std::cout << "Required extensions (" << requiredExtensions.size() << "):\n";

        for (const auto &extension : requiredExtensions)
        {
            std::cout << "\t" << extension << "\n";
        }
    }

    return requiredExtensions;
}

std::vector<vk::ExtensionProperties> Application::getAvailableInstanceExtensions()
{
    // Due to `vk::enumerateInstanceExtensionProperties()` being marked: "nodiscard", the return value must be handled
    vk::Result result;

    result = vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to enumerate instance extension count. Error code: " + vk::to_string(result));
    }

    availableExtensions.resize(extensionCount);

    result = vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to enumerate instance extension properties. Error code: " + vk::to_string(result));
    }

    if (enableValidationLayers)
    {
        // Print all available extensions and their names
        std::cout << "Available extensions (" << extensionCount << "):\n";

        for (const auto &extension : availableExtensions)
        {
            std::cout << "\t" << extension.extensionName << "\n";
        }
    }

    return availableExtensions;
}

bool Application::checkInstanceExtensionSupport()
{
    requiredExtensions = this->getRequiredInstanceExtensions();
    availableExtensions = this->getAvailableInstanceExtensions();

    bool extensionFound;
    for (const auto &requiredExtension : requiredExtensions)
    {
        extensionFound = false;
        for (const auto &availableExtension : availableExtensions)
        {
            if (strcmp(requiredExtension, availableExtension.extensionName) == 0)
            {
                extensionFound = true;
            }
        }

        if (!extensionFound)
        {
            return false;
        }
    }

    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Application::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

VkResult Application::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Application::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, debugMessenger, pAllocator);
    }
}

void Application::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createDebugInfo)
{
    createDebugInfo = {};
    createDebugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createDebugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createDebugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createDebugInfo.pfnUserCallback = debugCallback;
}

void Application::setupDebugMessenger()
{
    if (!enableValidationLayers)
    {
        return;
    }

    populateDebugMessengerCreateInfo(debugCreateInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}