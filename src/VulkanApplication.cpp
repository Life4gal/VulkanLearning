#include "VulkanApplication.h"

namespace
{
#ifdef NDEBUG
	constexpr bool EnableValidationLayers = false;
#else
	constexpr bool EnableValidationLayers = true;
#endif

const char* g_validationLayers = "VK_LAYER_KHRONOS_validation";

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
		createInfo.enabledLayerCount = 1;
		createInfo.ppEnabledLayerNames = &g_validationLayers;

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

	auto queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo =
	{
		VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		nullptr,
		0,
		indices.graphicsFamily.value(),
		1,
		&queuePriority
	};

	VkPhysicalDeviceFeatures deviceFeatures{};
	VkDeviceCreateInfo createInfo =
	{
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,
		0,
		1,
		&queueCreateInfo,
		0,
		nullptr,
		0,
		nullptr,
		&deviceFeatures
	};

	if(EnableValidationLayers)
	{
		createInfo.enabledLayerCount = 1;
		createInfo.ppEnabledLayerNames = &g_validationLayers;
	}

	if(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device!");
	}

	vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
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

	for(const auto& properties : availableLayers)
	{
		if(strcmp(properties.layerName, g_validationLayers) == 0)
		{
			return true;
		}
	}
	
	return false;
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
