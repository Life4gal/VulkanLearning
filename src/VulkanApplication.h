#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <string>
#include <iostream>
#include <optional>

class VulkanApplication
{
public:
	VulkanApplication(uint32_t width, uint32_t height);

	~VulkanApplication();

	void InitInstance();
	void Run() const;

private:
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		[[nodiscard]] constexpr bool IsComplete() const
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};
	
	void CreateInstance();
	static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void SetupDebugMessenger();
	void CreateSurface();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	
	static std::vector<const char*> GetRequiredExtensions();
	[[nodiscard]] static bool CheckValidationLayerSupport();
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
	
	uint32_t m_width;
	uint32_t m_height;
	GLFWwindow* m_pWindow;
	
	VkInstance m_instance{};
	VkDebugUtilsMessengerEXT m_debugUtilsMessenger{};
	VkSurfaceKHR m_surface{};
	
	VkPhysicalDevice m_physicalDevice{};
	VkDevice m_device{};
	
	VkQueue m_graphicsQueue{};
	VkQueue m_presentQueue{};
};

