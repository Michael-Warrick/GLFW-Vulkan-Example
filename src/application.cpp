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

    if (enableValidationLayers)
    {
#ifdef __APPLE__
        // Enable MoltenVK's validation via environment variables
        setenv("VK_LAYER_PATH", "@ VULKAN_LAYER_PATH @", 1);
        setenv("VK_INSTANCE_LAYERS", "VK_LAYER_MESA_overlay", 1); // This enables MoltenVK's overlay for validation

        // MoltenVK doesn't use the standard Vulkan debug callback.
        return;
#endif
        setupDebugMessenger();
    }
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
#ifdef __APPLE__
        return;
#endif
        destroyDebugMessenger();
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

    // Print all available extensions and their names
    std::cout << "Available extensions (" << extensionCount << "):\n";

    for (const auto &extension : availableExtensions)
    {
        std::cout << "\t" << extension.extensionName << "\n";
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
    vk::DebugReportFlagsEXT messageSeverity,
    vk::DebugReportObjectTypeEXT messageType,
    const vk::DebugUtilsMessengerCallbackDataEXT *callbackData,
    void *userData)
{
    std::cerr << "Validation layer: " << callbackData->pMessage << std::endl;

    return VK_FALSE;
}

void Application::setupDebugMessenger()
{
    if (!enableValidationLayers)
    {
        return;
    }

    debugCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT()
                          .setMessageSeverity(
                              vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                              vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                              vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
                          .setMessageType(
                              vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                              vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                              vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
                          .setPfnUserCallback((PFN_vkDebugUtilsMessengerCallbackEXT)debugCallback)
                          .setPUserData(nullptr);

    auto CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)this->instance.getProcAddr("vkCreateDebugUtilsMessengerEXT");
    if (CreateDebugUtilsMessengerEXT != nullptr)
    {
        VkDebugUtilsMessengerEXT tmp;
        const VkDebugUtilsMessengerCreateInfoEXT tmpCreateInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
        if (CreateDebugUtilsMessengerEXT(this->instance, &tmpCreateInfo, nullptr, &tmp) == VK_SUCCESS)
        {
            debugMessanger = tmp;
        }
        else
        {
            throw std::runtime_error("Failed to create Vulkan debug messenger.");
        }
    }
    else
    {
        throw std::runtime_error("Cannot find required vkCreateDebugUtilsMessengerEXT function");
    }
}

void Application::destroyDebugMessenger()
{
    auto DestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)this->instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT");
    if (DestroyDebugUtilsMessengerEXT != nullptr)
    {
        DestroyDebugUtilsMessengerEXT(this->instance, this->debugMessanger, nullptr);
    }
    else
    {
        std::cerr << "Failed to destroy debug callback" << std::endl;
    }
}