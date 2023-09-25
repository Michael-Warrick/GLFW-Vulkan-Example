#include "application.hpp"

void Application::Run()
{
    init();
    update();
    shutdown();
}

void Application::init()
{
    initWindow();
    initVulkan();
}

void Application::initWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(800, 600, "GLFW Vulkan Example", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Application::initVulkan()
{
    createVulkanInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createImageViews();
    createRenderPass();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPool();
    createVertexBuffer();
    createIndexBuffer();
    createCommandBuffers();
    createSyncObjects();
}

void Application::update()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        drawFrame();
    }

    logicalDevice.waitIdle();
}

void Application::shutdown()
{
    cleanupSwapChain();

    logicalDevice.destroyBuffer(indexBuffer);
    logicalDevice.freeMemory(indexBufferMemory);

    logicalDevice.destroyBuffer(vertexBuffer);
    logicalDevice.freeMemory(vertexBufferMemory);

    logicalDevice.destroyPipeline(graphicsPipeline);
    logicalDevice.destroyPipelineLayout(pipelineLayout);

    logicalDevice.destroyRenderPass(renderPass);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        logicalDevice.destroySemaphore(imageAvailableSemaphores[i]);
        logicalDevice.destroySemaphore(renderFinishedSemaphores[i]);
        logicalDevice.destroyFence(inFlightFences[i]);
    }

    logicalDevice.destroyCommandPool(commandPool);

    logicalDevice.destroy();

    if (enableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    instance.destroySurfaceKHR(surface);
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
    vk::Result result = vk::enumerateInstanceLayerProperties(&layerCount, nullptr);
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

#ifdef __APPLE__
    // Adding the portability extension (for MoltenVK driver compatibility issue) + Setting flag
    requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    flags = vk::InstanceCreateFlags(vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR);
#endif

    if (enableValidationLayers)
    {
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        std::cout << "Required instance extensions (" << requiredExtensions.size() << "):\n";

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
    vk::Result result = vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
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
        std::cout << "Available instance extensions (" << extensionCount << "):\n";

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
    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

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
    createDebugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
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

void Application::pickPhysicalDevice()
{
    vk::Result result = instance.enumeratePhysicalDevices(&physicalDeviceCount, nullptr);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to enumerate physical devices. Error code: " + vk::to_string(result));
    }

    if (physicalDeviceCount == 0)
    {
        throw std::runtime_error("Failed to find a GPU with Vulkan support!");
    }

    physicalDevices = std::vector<vk::PhysicalDevice>(physicalDeviceCount);
    result = instance.enumeratePhysicalDevices(&physicalDeviceCount, physicalDevices.data());
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to enumerate physical devices with selected data. Error code: " + vk::to_string(result));
    }

    for (const auto &device : physicalDevices)
    {
        if (enableValidationLayers)
        {
            std::cout << "Targeted GPU: \n\t" << device.getProperties().deviceName << std::endl;
        }

        if (isDeviceSuitable(device))
        {
            physicalDevice = device;
            break;
        }
    }

    if (!physicalDevice)
    {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

bool Application::checkDeviceExtensionSupport(vk::PhysicalDevice device)
{
    uint32_t deviceExtensionCount;
    vk::Result result = device.enumerateDeviceExtensionProperties(nullptr, &deviceExtensionCount, nullptr);

    std::vector<vk::ExtensionProperties> availableDeviceExtensions(deviceExtensionCount);
    result = device.enumerateDeviceExtensionProperties(nullptr, &deviceExtensionCount, availableDeviceExtensions.data());

    std::set<std::string> requiredDeviceExtensions(logicalDeviceExtensions.begin(), logicalDeviceExtensions.end());

    for (const auto &deviceExtension : availableDeviceExtensions)
    {
        requiredDeviceExtensions.erase(deviceExtension.extensionName);
    }

    return requiredDeviceExtensions.empty();
}

Application::SwapChainSupportDetails Application::querySwapChainSupport(vk::PhysicalDevice device)
{
    SwapChainSupportDetails details;
    vk::Result result = device.getSurfaceCapabilitiesKHR(surface, &details.capabilities);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to get swapchain surface capabilities! Error Code: " + vk::to_string(result));
    }

    uint32_t formatCount;
    result = device.getSurfaceFormatsKHR(surface, &formatCount, nullptr);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to get swapchain surface formats! Error Code: " + vk::to_string(result));
    }

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);

        result = device.getSurfaceFormatsKHR(surface, &formatCount, details.formats.data());
        if (result != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to add swapchain surface formats to swapchain support details object! Error Code: " + vk::to_string(result));
        }
    }

    uint32_t presentModeCount;
    result = device.getSurfacePresentModesKHR(surface, &presentModeCount, nullptr);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to get swapchain surface present modes! Error Code: " + vk::to_string(result));
    }

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);

        result = device.getSurfacePresentModesKHR(surface, &presentModeCount, details.presentModes.data());
        if (result != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to add swapchain surface present modes to swapchain support details object! Error Code: " + vk::to_string(result));
        }
    }

    return details;
}

bool Application::isDeviceSuitable(vk::PhysicalDevice device)
{
    Application::QueueFamilyIndices indices = findQueueFamilies(device);
    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

Application::QueueFamilyIndices Application::findQueueFamilies(vk::PhysicalDevice device)
{
    Application::QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;

    device.getQueueFamilyProperties(&queueFamilyCount, nullptr);
    std::vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
    device.getQueueFamilyProperties(&queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        vk::Bool32 presentSupport = false;

        vk::Result result = device.getSurfaceSupportKHR(i, surface, &presentSupport);
        if (result != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to physical device surface support! Error Code: " + vk::to_string(result));
        }

        if (queueFamily.queueCount > 0)
        {
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                indices.graphicsFamily = i;
            }

            if (presentSupport)
            {
                indices.presentFamily = i;
            }
        }

        if (indices.isComplete())
        {
            break;
        }

        i++;
    }

    return indices;
}

void Application::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    float queuePriority = 1.0f;

    std::vector<vk::DeviceQueueCreateInfo> queueFamilyCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(),
        indices.presentFamily.value()};

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        queueFamilyCreateInfos.push_back(vk::DeviceQueueCreateInfo()
                                             .setQueueFamilyIndex(queueFamily)
                                             .setQueueCount(1)
                                             .setPQueuePriorities(&queuePriority));
    }

    logicalDeviceCreateInfo = vk::DeviceCreateInfo()
                                  .setPQueueCreateInfos(queueFamilyCreateInfos.data())
                                  .setQueueCreateInfoCount(queueFamilyCreateInfos.size())
                                  .setPEnabledFeatures(&physicalDeviceFeatures)
                                  .setEnabledExtensionCount(logicalDeviceExtensions.size())
                                  .setPpEnabledExtensionNames(logicalDeviceExtensions.data());

    if (enableValidationLayers)
    {
        logicalDeviceCreateInfo.setEnabledLayerCount(validationLayers.size());
        logicalDeviceCreateInfo.setPpEnabledLayerNames(validationLayers.data());
    }
    else
    {
        logicalDeviceCreateInfo.setEnabledLayerCount(0);
    }

    vk::Result result = physicalDevice.createDevice(&logicalDeviceCreateInfo, nullptr, &logicalDevice);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to create logical device! Error Code: " + vk::to_string(result));
    }

    logicalDevice.getQueue(indices.graphicsFamily.value(), 0, &graphicsQueue);
    logicalDevice.getQueue(indices.presentFamily.value(), 0, &presentQueue);
}

void Application::createSurface()
{
    VkResult result = glfwCreateWindowSurface(instance, window, nullptr, reinterpret_cast<VkSurfaceKHR *>(&surface));
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create surface!");
    }
}

vk::SurfaceFormatKHR Application::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR Application::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes)
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox)
        {
            return availablePresentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Application::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width;
        int height;

        glfwGetFramebufferSize(window, &width, &height);
        vk::Extent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void Application::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swapChainCreateInfo = vk::SwapchainCreateInfoKHR()
                                                         .setSurface(surface)
                                                         .setMinImageCount(imageCount)
                                                         .setImageFormat(surfaceFormat.format)
                                                         .setImageColorSpace(surfaceFormat.colorSpace)
                                                         .setImageExtent(extent)
                                                         .setImageArrayLayers(1)
                                                         .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
                                                         .setPreTransform(swapChainSupport.capabilities.currentTransform)
                                                         .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                                                         .setPresentMode(presentMode)
                                                         .setClipped(VK_TRUE)
                                                         .setOldSwapchain(VK_NULL_HANDLE);

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
        swapChainCreateInfo.setQueueFamilyIndexCount(2);
        swapChainCreateInfo.setPQueueFamilyIndices(queueFamilyIndices);
    }
    else
    {
        swapChainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
        swapChainCreateInfo.setQueueFamilyIndexCount(0);
        swapChainCreateInfo.setPQueueFamilyIndices(nullptr);
    }

    vk::Result result = logicalDevice.createSwapchainKHR(&swapChainCreateInfo, nullptr, &swapChain);

    result = logicalDevice.getSwapchainImagesKHR(swapChain, &imageCount, nullptr);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to get swap chain images! Error Code: " + vk::to_string(result));
    }

    swapChainImages.resize(imageCount);

    result = logicalDevice.getSwapchainImagesKHR(swapChain, &imageCount, swapChainImages.data());
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to set swap chain images! Error Code: " + vk::to_string(result));
    }

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void Application::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        vk::ImageViewCreateInfo imageViewCreateInfo = vk::ImageViewCreateInfo()
                                                          .setImage(swapChainImages[i])
                                                          .setViewType(vk::ImageViewType::e2D)
                                                          .setFormat(swapChainImageFormat)
                                                          .setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity))
                                                          .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

        vk::Result result = logicalDevice.createImageView(&imageViewCreateInfo, nullptr, &swapChainImageViews[i]);
        if (result != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to create image views! Error Code: " + vk::to_string(result));
        }
    }
}

void Application::createGraphicsPipeline()
{
    auto vertexShaderCode = readFile("resources/shaders/compiled/vert.spv");
    auto fragmentShaderCode = readFile("resources/shaders/compiled/frag.spv");

    vk::ShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
    vk::ShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

    vk::PipelineShaderStageCreateInfo vertexShaderStageCreateInfo = vk::PipelineShaderStageCreateInfo()
                                                                        .setStage(vk::ShaderStageFlagBits::eVertex)
                                                                        .setModule(vertexShaderModule)
                                                                        .setPName("main");
    vk::PipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = vk::PipelineShaderStageCreateInfo()
                                                                          .setStage(vk::ShaderStageFlagBits::eFragment)
                                                                          .setModule(fragmentShaderModule)
                                                                          .setPName("main");
    vk::PipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo = vk::PipelineVertexInputStateCreateInfo()
                                                                       .setVertexBindingDescriptionCount(1)
                                                                       .setPVertexBindingDescriptions(&bindingDescription)
                                                                       .setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()))
                                                                       .setPVertexAttributeDescriptions(attributeDescriptions.data());

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = vk::PipelineInputAssemblyStateCreateInfo()
                                                                           .setTopology(vk::PrimitiveTopology::eTriangleList)
                                                                           .setPrimitiveRestartEnable(VK_FALSE);

    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo()
                                                                      .setViewportCount(1)
                                                                      .setScissorCount(1);

    vk::PipelineRasterizationStateCreateInfo rasterizerCreateInfo = vk::PipelineRasterizationStateCreateInfo()
                                                                        .setDepthClampEnable(VK_FALSE)
                                                                        .setRasterizerDiscardEnable(VK_FALSE)
                                                                        .setPolygonMode(vk::PolygonMode::eFill)
                                                                        .setLineWidth(1.0f)
                                                                        .setCullMode(vk::CullModeFlagBits::eBack)
                                                                        .setFrontFace(vk::FrontFace::eClockwise)
                                                                        .setDepthBiasEnable(VK_FALSE)
                                                                        .setDepthBiasConstantFactor(0.0f)
                                                                        .setDepthBiasClamp(0.0f)
                                                                        .setDepthBiasSlopeFactor(0.0f);

    vk::PipelineMultisampleStateCreateInfo multisamplingCreateInfo = vk::PipelineMultisampleStateCreateInfo()
                                                                         .setSampleShadingEnable(VK_FALSE)
                                                                         .setRasterizationSamples(vk::SampleCountFlagBits::e1)
                                                                         .setMinSampleShading(1.0f)
                                                                         .setPSampleMask(nullptr)
                                                                         .setAlphaToCoverageEnable(VK_FALSE)
                                                                         .setAlphaToOneEnable(VK_FALSE);

    vk::PipelineColorBlendAttachmentState colorBlendAttachmentState = vk::PipelineColorBlendAttachmentState()
                                                                          .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
                                                                          .setBlendEnable(VK_FALSE)
                                                                          .setSrcColorBlendFactor(vk::BlendFactor::eOne)
                                                                          .setDstColorBlendFactor(vk::BlendFactor::eZero)
                                                                          .setColorBlendOp(vk::BlendOp::eAdd)
                                                                          .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                                                                          .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
                                                                          .setAlphaBlendOp(vk::BlendOp::eAdd);

    vk::PipelineColorBlendStateCreateInfo colorBlendingCreateInfo = vk::PipelineColorBlendStateCreateInfo()
                                                                        .setLogicOpEnable(VK_FALSE)
                                                                        .setLogicOp(vk::LogicOp::eCopy)
                                                                        .setAttachmentCount(1)
                                                                        .setPAttachments(&colorBlendAttachmentState)
                                                                        .setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});

    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor};

    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo()
                                                                    .setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()))
                                                                    .setPDynamicStates(dynamicStates.data());

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
                                                                .setSetLayoutCount(0)
                                                                .setPSetLayouts(nullptr)
                                                                .setPushConstantRangeCount(0)
                                                                .setPPushConstantRanges(nullptr);

    vk::Result result = logicalDevice.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to create pipeline layout! Error Code: " + vk::to_string(result));
    }

    vk::GraphicsPipelineCreateInfo pipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
                                                            .setStageCount(2)
                                                            .setPStages(shaderStages)
                                                            .setPVertexInputState(&vertexInputCreateInfo)
                                                            .setPInputAssemblyState(&inputAssemblyCreateInfo)
                                                            .setPViewportState(&viewportStateCreateInfo)
                                                            .setPRasterizationState(&rasterizerCreateInfo)
                                                            .setPMultisampleState(&multisamplingCreateInfo)
                                                            .setPDepthStencilState(nullptr)
                                                            .setPColorBlendState(&colorBlendingCreateInfo)
                                                            .setPDynamicState(&dynamicStateCreateInfo)
                                                            .setLayout(pipelineLayout)
                                                            .setRenderPass(renderPass)
                                                            .setSubpass(0)
                                                            .setBasePipelineHandle(VK_NULL_HANDLE)
                                                            .setBasePipelineIndex(-1);

    result = logicalDevice.createGraphicsPipelines(VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to create graphics pipeline! Error Code: " + vk::to_string(result));
    }

    logicalDevice.destroyShaderModule(fragmentShaderModule);
    logicalDevice.destroyShaderModule(vertexShaderModule);
}

std::vector<char> Application::readFile(const std::string &fileName)
{
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

vk::ShaderModule Application::createShaderModule(const std::vector<char> &code)
{
    vk::ShaderModuleCreateInfo shaderModuleCreateInfo = vk::ShaderModuleCreateInfo()
                                                            .setCodeSize(code.size())
                                                            .setPCode(reinterpret_cast<const uint32_t *>(code.data()));

    vk::ShaderModule shaderModule;
    vk::Result result = logicalDevice.createShaderModule(&shaderModuleCreateInfo, nullptr, &shaderModule);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to create shader module! Error Code: " + vk::to_string(result));
    }

    return shaderModule;
}

void Application::createRenderPass()
{
    vk::AttachmentDescription colorAttachment = vk::AttachmentDescription()
                                                    .setFormat(swapChainImageFormat)
                                                    .setSamples(vk::SampleCountFlagBits::e1)
                                                    .setLoadOp(vk::AttachmentLoadOp::eClear)
                                                    .setStoreOp(vk::AttachmentStoreOp::eStore)
                                                    .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                                                    .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                                                    .setInitialLayout(vk::ImageLayout::eUndefined)
                                                    .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference colorAttachmentReference = vk::AttachmentReference()
                                                           .setAttachment(0)
                                                           .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::SubpassDescription subpass = vk::SubpassDescription()
                                         .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                                         .setColorAttachmentCount(1)
                                         .setPColorAttachments(&colorAttachmentReference);

    vk::SubpassDependency subpassDependency = vk::SubpassDependency()
                                                  .setSrcSubpass(vk::SubpassExternal)
                                                  .setDstSubpass(0)
                                                  .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                                                  .setSrcAccessMask(vk::AccessFlagBits::eNone)
                                                  .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                                                  .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    vk::RenderPassCreateInfo renderPassCreateInfo = vk::RenderPassCreateInfo()
                                                        .setAttachmentCount(1)
                                                        .setPAttachments(&colorAttachment)
                                                        .setSubpassCount(1)
                                                        .setPSubpasses(&subpass)
                                                        .setDependencyCount(1)
                                                        .setPDependencies(&subpassDependency);

    vk::Result result = logicalDevice.createRenderPass(&renderPassCreateInfo, nullptr, &renderPass);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to create render pass! Error Code: " + vk::to_string(result));
    }
}

void Application::createFramebuffers()
{
    swapChainFrameBuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        vk::ImageView attachments[] = {swapChainImageViews[i]};

        vk::FramebufferCreateInfo framebufferCreateInfo = vk::FramebufferCreateInfo()
                                                              .setRenderPass(renderPass)
                                                              .setAttachmentCount(1)
                                                              .setPAttachments(attachments)
                                                              .setWidth(swapChainExtent.width)
                                                              .setHeight(swapChainExtent.height)
                                                              .setLayers(1);

        vk::Result result = logicalDevice.createFramebuffer(&framebufferCreateInfo, nullptr, &swapChainFrameBuffers[i]);
        if (result != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to create framebuffer! Error Code: " + vk::to_string(result));
        }
    }
}

void Application::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    vk::CommandPoolCreateInfo commandPoolCreateInfo = vk::CommandPoolCreateInfo()
                                                          .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
                                                          .setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value());

    vk::Result result = logicalDevice.createCommandPool(&commandPoolCreateInfo, nullptr, &commandPool);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to create command pool! Error Code: " + vk::to_string(result));
    }
}

void Application::createCommandBuffers()
{
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    vk::CommandBufferAllocateInfo allocateCreateInfo = vk::CommandBufferAllocateInfo()
                                                           .setCommandPool(commandPool)
                                                           .setLevel(vk::CommandBufferLevel::ePrimary)
                                                           .setCommandBufferCount((uint32_t)commandBuffers.size());

    vk::Result result = logicalDevice.allocateCommandBuffers(&allocateCreateInfo, commandBuffers.data());
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to allocate command buffers! Error Code: " + vk::to_string(result));
    }
}

void Application::recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
{
    vk::CommandBufferBeginInfo beginCreateInfo = vk::CommandBufferBeginInfo();

    vk::Result result = commandBuffer.begin(&beginCreateInfo);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to allocate command buffers! Error Code: " + vk::to_string(result));
    }

    vk::ClearValue clearColor = {{0.0f, 0.0f, 0.0f, 1.0f}};
    vk::RenderPassBeginInfo renderPassBeginCreateInfo = vk::RenderPassBeginInfo()
                                                            .setRenderPass(renderPass)
                                                            .setFramebuffer(swapChainFrameBuffers[imageIndex])
                                                            .setRenderArea(vk::Rect2D({0, 0}, swapChainExtent))
                                                            .setClearValueCount(1)
                                                            .setPClearValues(&clearColor);

    commandBuffer.beginRenderPass(&renderPassBeginCreateInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    vk::Viewport viewport = vk::Viewport()
                                .setX(0.0f)
                                .setY(0.0f)
                                .setWidth((float)swapChainExtent.width)
                                .setHeight((float)swapChainExtent.height)
                                .setMinDepth(0.0f)
                                .setMaxDepth(1.0f);
    commandBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor = vk::Rect2D()
                             .setOffset({0, 0})
                             .setExtent(swapChainExtent);
    commandBuffer.setScissor(0, 1, &scissor);

    vk::Buffer vertexBuffers[] = {vertexBuffer};
    vk::DeviceSize offsets[] = {0};
    commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
    commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint16);

    commandBuffer.drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    commandBuffer.endRenderPass();
    commandBuffer.end();
}

void Application::drawFrame()
{
    vk::Result result = logicalDevice.waitForFences(1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to wait for in-flight fence! Error Code: " + vk::to_string(result));
    }

    uint32_t imageIndex;
    result = logicalDevice.acquireNextImageKHR(swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == vk::Result::eErrorOutOfDateKHR)
    {
        recreateSwapChain();
        return;
    }
    else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    result = logicalDevice.resetFences(1, &inFlightFences[currentFrame]);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to reset in-flight fence! Error Code: " + vk::to_string(result));
    }

    commandBuffers[currentFrame].reset();
    recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};

    vk::SubmitInfo submitInfo = vk::SubmitInfo()
                                    .setWaitSemaphoreCount(1)
                                    .setPWaitSemaphores(waitSemaphores)
                                    .setPWaitDstStageMask(waitStages)
                                    .setCommandBufferCount(1)
                                    .setPCommandBuffers(&commandBuffers[currentFrame])
                                    .setSignalSemaphoreCount(1)
                                    .setPSignalSemaphores(signalSemaphores);

    result = graphicsQueue.submit(1, &submitInfo, inFlightFences[currentFrame]);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to submit draw command buffer! Error Code: " + vk::to_string(result));
    }

    vk::SwapchainKHR swapChains[] = {swapChain};

    vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR()
                                         .setWaitSemaphoreCount(1)
                                         .setPWaitSemaphores(signalSemaphores)
                                         .setSwapchainCount(1)
                                         .setPSwapchains(swapChains)
                                         .setPImageIndices(&imageIndex)
                                         .setPResults(nullptr);

    result = presentQueue.presentKHR(&presentInfo);
    if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || framebufferResized)
    {
        framebufferResized = false;
        recreateSwapChain();
    }
    else if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to present: present queue! Error Code: " + vk::to_string(result));
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Application::createSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    vk::SemaphoreCreateInfo semaphoreCreateInfo = vk::SemaphoreCreateInfo();
    vk::FenceCreateInfo fenceCreateInfo = vk::FenceCreateInfo()
                                              .setFlags(vk::FenceCreateFlagBits::eSignaled);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vk::Result result = logicalDevice.createSemaphore(&semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]);
        if (result != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to create image available semaphore! Error Code: " + vk::to_string(result));
        }

        result = logicalDevice.createSemaphore(&semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]);
        if (result != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to create render finished semaphore! Error Code: " + vk::to_string(result));
        }

        result = logicalDevice.createFence(&fenceCreateInfo, nullptr, &inFlightFences[i]);
        if (result != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to create in-flight fence! Error Code: " + vk::to_string(result));
        }
    }
}

void Application::recreateSwapChain()
{
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    logicalDevice.waitIdle();

    cleanupSwapChain();

    createSwapChain();
    createImageViews();
    createFramebuffers();
}

void Application::cleanupSwapChain()
{
    for (size_t i = 0; i < swapChainFrameBuffers.size(); i++)
    {
        logicalDevice.destroyFramebuffer(swapChainFrameBuffers[i]);
    }

    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        logicalDevice.destroyImageView(swapChainImageViews[i]);
    }

    logicalDevice.destroySwapchainKHR(swapChain);
}

void Application::framebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    auto app = reinterpret_cast<Application *>(glfwGetWindowUserPointer(window));
    app->framebufferResized = true;
}

void Application::createVertexBuffer()
{
    vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

    void *data;
    vk::Result result = logicalDevice.mapMemory(stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlags(), &data);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to map vertex buffer memory! Error Code: " + vk::to_string(result));
    }

    memcpy(data, vertices.data(), (size_t)bufferSize);
    logicalDevice.unmapMemory(stagingBufferMemory);

    createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBuffer, vertexBufferMemory);
    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    logicalDevice.destroyBuffer(stagingBuffer);
    logicalDevice.freeMemory(stagingBufferMemory);
}

void Application::createIndexBuffer()
{
    vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    vk::Buffer stagingBuffer;
    vk::DeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

    void* data;
    vk::Result result = logicalDevice.mapMemory(stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlags(), &data);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to map vertex buffer memory! Error Code: " + vk::to_string(result));
    }

    memcpy(data, indices.data(), (size_t)bufferSize);
    logicalDevice.unmapMemory(stagingBufferMemory);

    createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer, indexBufferMemory);
    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    logicalDevice.destroyBuffer(stagingBuffer);
    logicalDevice.freeMemory(stagingBufferMemory);
}

uint32_t Application::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
    vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    physicalDevice.getMemoryProperties(&physicalDeviceMemoryProperties);

    for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

void Application::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory)
{
    vk::BufferCreateInfo bufferCreateInfo = vk::BufferCreateInfo()
                                                .setSize(size)
                                                .setUsage(usage)
                                                .setSharingMode(vk::SharingMode::eExclusive);

    vk::Result result = logicalDevice.createBuffer(&bufferCreateInfo, nullptr, &buffer);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to create buffer! Error Code: " + vk::to_string(result));
    }

    vk::MemoryRequirements memoryRequirements;
    logicalDevice.getBufferMemoryRequirements(buffer, &memoryRequirements);

    vk::MemoryAllocateInfo memoryAllocateInfo = vk::MemoryAllocateInfo()
                                                    .setAllocationSize(memoryRequirements.size)
                                                    .setMemoryTypeIndex(findMemoryType(memoryRequirements.memoryTypeBits, properties));

    result = logicalDevice.allocateMemory(&memoryAllocateInfo, nullptr, &bufferMemory);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to allocate buffer memory! Error Code: " + vk::to_string(result));
    }

    logicalDevice.bindBufferMemory(buffer, bufferMemory, 0);
}

void Application::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
    vk::CommandBufferAllocateInfo allocateInfo = vk::CommandBufferAllocateInfo()
                                                     .setLevel(vk::CommandBufferLevel::ePrimary)
                                                     .setCommandPool(commandPool)
                                                     .setCommandBufferCount(1);

    vk::CommandBuffer commandBuffer;
    vk::Result result = logicalDevice.allocateCommandBuffers(&allocateInfo, &commandBuffer);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to allocate command buffer! Error Code: " + vk::to_string(result));
    }

    vk::CommandBufferBeginInfo commandBufferBeginInfo = vk::CommandBufferBeginInfo()
                                                            .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    result = commandBuffer.begin(&commandBufferBeginInfo);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to begin command buffer! Error Code: " + vk::to_string(result));
    }

    vk::BufferCopy copyRegion = vk::BufferCopy()
                                    .setSrcOffset(0)
                                    .setDstOffset(0)
                                    .setSize(size);
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);
    commandBuffer.end();

    vk::SubmitInfo submitInfo = vk::SubmitInfo()
                                    .setCommandBufferCount(1)
                                    .setPCommandBuffers(&commandBuffer);

    result = graphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);
    if (result != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to submit command buffer! Error Code: " + vk::to_string(result));
    }

    graphicsQueue.waitIdle();

    logicalDevice.freeCommandBuffers(commandPool, 1, &commandBuffer);
}