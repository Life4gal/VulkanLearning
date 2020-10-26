#include "VulkanApplication.h"

namespace
{
#ifdef NDEBUG
	constexpr bool EnableValidationLayers = false;
#else
	constexpr bool EnableValidationLayers = true;
#endif

const std::vector<const char*> ValidationLayers = {
	"VK_LAYER_KHRONOS_validation"
};
const std::vector<const char*> DeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VkResult CreateDebugUtilsMessengerExt(
	// ReSharper disable once CppParameterMayBeConst
	VkInstance instance, 
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfoExt, 
	const VkAllocationCallbacks* pAllocationCallbacks, 
	VkDebugUtilsMessengerEXT* pDebugUtilsMessengerExt
)
{
	const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
		instance, "vkCreateDebugUtilsMessengerEXT"));
	if(func != nullptr)
	{
		return func(instance, pCreateInfoExt, pAllocationCallbacks, pDebugUtilsMessengerExt);
	}
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerExt(
	// ReSharper disable once CppParameterMayBeConst
	VkInstance instance,
	const VkDebugUtilsMessengerEXT debugUtilsMessenger,
	const VkAllocationCallbacks* pAllocationCallbacks
)
{
	const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
		instance, "vkDestroyDebugUtilsMessengerEXT"));
	if(func != nullptr)
	{
		func(instance, debugUtilsMessenger, pAllocationCallbacks);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackDataExt,
	void* pUserData)
{
	std::cerr << "Validation layer info -> " << pCallbackDataExt->pMessage << std::endl;

	return VK_FALSE;
}
	
}

VulkanApplication::VulkanApplication(const uint32_t width, const uint32_t height)
	:
	m_width(width), m_height(height)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_pWindow = glfwCreateWindow(m_width, m_height, "Vulkan", nullptr, nullptr);
}

VulkanApplication::~VulkanApplication()
{
	vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
	vkDestroyDevice(m_device, nullptr);
	
	if(EnableValidationLayers)
	{
		DestroyDebugUtilsMessengerExt(m_instance, m_debugUtilsMessenger, nullptr);
	}

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
	
	glfwDestroyWindow(m_pWindow);

	glfwTerminate();
}


void VulkanApplication::InitInstance()
{
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapChain();
	CreateImageViews();
}

void VulkanApplication::Run() const
{
	while (!glfwWindowShouldClose(m_pWindow))
	{
		glfwPollEvents();
	}
}

void VulkanApplication::CreateInstance()
{
	// ReSharper disable once CppRedundantBooleanExpressionArgument
	if(EnableValidationLayers && !CheckValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}
	
	VkApplicationInfo appInfo = 
	{
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		"Triangle",
		VK_MAKE_VERSION(1, 0, 0),
		"No Engine",
		VK_MAKE_VERSION(1, 0, 0),
		VK_API_VERSION_1_0
	};

	auto extensions = GetRequiredExtensions();
	VkInstanceCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		nullptr,
		0,
		&appInfo,
		0,
		nullptr,
		static_cast<uint32_t>(extensions.size()),
		extensions.data()
	};
	
	if(EnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		createInfo.ppEnabledLayerNames = ValidationLayers.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugCreateInfo);
	}

	if(vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create instance!");
	}
}

void VulkanApplication::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo =
	{
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		nullptr,
		0,
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		DebugCallback,
		nullptr
	};
}

void VulkanApplication::SetupDebugMessenger()
{
	if constexpr (!EnableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);

	if(CreateDebugUtilsMessengerExt(m_instance, &createInfo, nullptr, &m_debugUtilsMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to setup debug messenger!");
	}
}

void VulkanApplication::CreateSurface()
{
	if(glfwCreateWindowSurface(m_instance, m_pWindow, nullptr, &m_surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface!");
	}
}

void VulkanApplication::PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

	if(deviceCount == 0)
	{
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

	for(const auto& device : devices)
	{
		if(FindQueueFamilies(device).IsComplete())
		{
			m_physicalDevice = device;
			break;
		}
	}

	if(m_physicalDevice == nullptr)
	{
		throw std::runtime_error("Failed to find a suitable GPU!");
	}
}

void VulkanApplication::CreateLogicalDevice()
{
	auto indices = FindQueueFamilies(m_physicalDevice);
	
	std::set<uint32_t> uniqueQueueFamilies =
	{
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};
	
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	auto queuePriority = 1.0f;
	for(auto queueFamily : uniqueQueueFamilies)
	{
		queueCreateInfos.emplace_back(VkDeviceQueueCreateInfo{
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			queueFamily,
			1,
			&queuePriority
		}
		);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	VkDeviceCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(queueCreateInfos.size()),
		queueCreateInfos.data(),
		0,
		nullptr,
		static_cast<uint32_t>(DeviceExtensions.size()),
		DeviceExtensions.data(),
		&deviceFeatures
	};

	if(EnableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		createInfo.ppEnabledLayerNames = ValidationLayers.data();
	}

	if(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device!");
	}

	vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
}


void VulkanApplication::CreateSwapChain()
{
	const auto swapChainSupport = QuerySwapChainSupport(m_physicalDevice);

	const auto surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	const auto presentMode = ChooseSwapChainMode(swapChainSupport.presentModes);
	const auto extent = ChooseSwapExtent(swapChainSupport.capabilities);

	auto imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo =
	{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		nullptr,
		0,
		m_surface,
		imageCount,
		surfaceFormat.format,
		surfaceFormat.colorSpace,
		extent,
		1,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		nullptr,
		swapChainSupport.capabilities.currentTransform,
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		presentMode,
		VK_TRUE,
		nullptr
	};

	auto indices = FindQueueFamilies(m_physicalDevice);
	
	if(indices.graphicsFamily != indices.presentFamily)
	{
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
		
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}

	if(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
	m_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());

	m_swapChainImageFormat = surfaceFormat.format;
	m_swapChainExtent = extent;
}

void VulkanApplication::CreateImageViews()
{
	swapChainImageViews.resize(m_swapChainImages.size());

	for(size_t i = 0; i < m_swapChainImages.size(); ++i)
	{
		VkImageViewCreateInfo createInfo =
		{
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			nullptr,
			0,
			m_swapChainImages[i],
			VK_IMAGE_VIEW_TYPE_2D,
			m_swapChainImageFormat,
			{
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY
			},
			{
				VK_IMAGE_ASPECT_COLOR_BIT,
				0,
				1,
				0,
				1
			}
		};

		if(vkCreateImageView(m_device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create image views!");
		}
	}
}

std::vector<const char*> VulkanApplication::GetRequiredExtensions()
{
	uint32_t extensionsCount = 0;
	const auto extensions = glfwGetRequiredInstanceExtensions(&extensionsCount);

	std::vector<const char*> extensionsName(extensions, extensions + extensionsCount);

	if(EnableValidationLayers)
	{
		extensionsName.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensionsName;
}

bool VulkanApplication::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const auto layerName : ValidationLayers)
	{
		for(const auto& layerProperties : availableLayers)
		{
			if(strcmp(layerName, layerProperties.layerName) == 0)
			{
				return true;
			}
		}
	}
	
	return false;
}

// ReSharper disable once CppParameterMayBeConst
bool VulkanApplication::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);

	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

	for(const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

// ReSharper disable once CppParameterMayBeConst
VulkanApplication::QueueFamilyIndices VulkanApplication::FindQueueFamilies(VkPhysicalDevice device) const
{
	QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	auto i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

		if(presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.IsComplete())
		{
			break;
		}

		++i;
	}

	return indices;
}

VulkanApplication::SwapChainSupportDetails VulkanApplication::QuerySwapChainSupport(VkPhysicalDevice device) const
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

	uint32_t formatCount;

	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;

	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

	if(presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR VulkanApplication::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VulkanApplication::ChooseSwapChainMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanApplication::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
{
	if(capabilities.currentExtent.width != UINT32_MAX)
	{
		return capabilities.currentExtent;
	}

	return {
		std::clamp(m_width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
		std::clamp(m_height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
	};
}

// ReSharper disable once CppParameterMayBeConst
bool VulkanApplication::IsDeviceSuitable(VkPhysicalDevice device) const
{
	const auto indices = FindQueueFamilies(device);


	if (indices.IsComplete() && CheckDeviceExtensionSupport(device))
	{
		const auto swapChainSupport = QuerySwapChainSupport(device);
		return !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return false;
}
