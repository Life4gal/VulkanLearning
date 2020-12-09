#ifndef VULKANLEARNING_APPLICATION_H
#define VULKANLEARNING_APPLICATION_H

#define GLFW_INCLUDE_VULKAN
// do nothing, just make this macro had been used already
GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <string>
#include <iostream>
#include <optional>
#include <set>
#include <algorithm>
#include <cstring>

#include "../tools/LoadShader.h"

class VulkanApplication
{
public:
	VulkanApplication(uint32_t width, uint32_t height);

	~VulkanApplication();

	void InitInstance();
	void Run();

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

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	
	void CreateInstance();
	static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void SetupDebugMessenger();
	void CreateSurface();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSwapChain();
	void CreateImageViews();
    void CreateRenderPass();
	void CreateGraphicsPipeline();
    void CreateFramebuffer();
    void CreateCommandPool();
    void CreateCommandBuffer();
    void CreateSyncObjects();

    void DrawFrame();
	
	static std::vector<const char*> GetRequiredExtensions();
	[[nodiscard]] static bool CheckValidationLayerSupport();
	[[nodiscard]] static bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;
	static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR ChooseSwapChainMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	[[nodiscard]] VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
	[[nodiscard]] bool IsDeviceSuitable(VkPhysicalDevice device) const;

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

	VkSwapchainKHR m_swapChain{};
	std::vector<VkImage> m_swapChainImages;
    VkFormat m_swapChainImageFormat{};
    VkExtent2D m_swapChainExtent{};
	std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFramebuffer;

    VkRenderPass m_renderPass{};
    VkPipelineLayout m_pipelineLayout{};
    VkPipeline m_graphicsPipeline{};

    VkCommandPool m_commandPool{};
    std::vector<VkCommandBuffer> m_commandBuffer;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;
    size_t m_currentFrame = 0;
};

#endif
