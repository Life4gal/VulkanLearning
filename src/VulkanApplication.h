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
	void CreateInstance();
	static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void SetupDebugMessenger();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	
	static std::vector<const char*> GetRequiredExtensions();
	[[nodiscard]] static bool CheckValidationLayerSupport();
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackDataExt,
		void* pUserData);

	
	uint32_t m_width;
	uint32_t m_height;
	
	GLFWwindow* m_pWindow;
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugUtilsMessenger;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;
	VkQueue m_graphicsQueue;
};

