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
    instance.destroy();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void Application::createVulkanInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("Validation layers requested but none are available!");
    }
    
    // Setting basic app info
    appInfo = vk::ApplicationInfo(
        "GLFW Vulcan Example",
        VK_MAKE_API_VERSION(0, 1, 0, 0),
        "Simple Vulcan Renderer",
        VK_MAKE_API_VERSION(0, 1, 0, 0),
        VK_API_VERSION_1_3
    );

    // Retrieving all required GLFW extensions for instance creation
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // Adding all required GLFW extensions to the required extension list 
    for (uint32_t i = 0; i < glfwExtensionCount; i++)
    {
        requiredExtensions.emplace_back(glfwExtensions[i]);
    }

    // Adding the portability extension (for MoltenVK driver compatibility issue) + Setting flag
    requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    createInfo.flags |= vk::InstanceCreateFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);

    // Setting up instance creation info to use our appInfo, not providing layerCount/layerNames and finally our extensions
    createInfo = vk::InstanceCreateInfo(
        {},
        &appInfo,
        static_cast<uint32_t>(validationLayers.size()),
        validationLayers.data(),
        static_cast<uint32_t>(requiredExtensions.size()),
        requiredExtensions.data()
    );

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

    for (const auto& extension : availableExtensions) 
    {
        std::cout << "\t" << extension.extensionName << "\n";
    }

    // Finally, attempt to create a Vulkan instance, passing our creation info
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
    
    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) 
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