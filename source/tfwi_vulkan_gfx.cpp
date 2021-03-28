#include "tfwi_vulkan_gfx.hpp"

#ifndef NDEBUG
VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}
#endif

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() &&
			presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class HelloTriangleApplication {
public:
	const uint32_t window_width = 800;
	const uint32_t window_height = 600;
	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

#ifndef NDEBUG
	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "Validation layer: " << pCallbackData->pMessage << '\n';

		return VK_FALSE;
	}
#endif

	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	GLFWwindow* window;
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
#ifndef NDEBUG
	VkDebugUtilsMessengerEXT debugMessenger;
#endif

	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(window_width, window_height, "Learn Vulkan", nullptr, nullptr);
	}

#ifndef NDEBUG
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;

	}
#endif

	void createInstance() {
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Learn Vulkan";
		appInfo.applicationVersion = VK_MAKE_VERSION(
			LEARN_VULKAN_VERSION_MAJOR,
			LEARN_VULKAN_VERSION_MINOR,
			LEARN_VULKAN_VERSION_PATCH);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// Retrieve required extensions from GLFW
		uint32_t glfwRequiredExtCount = 0;
		const char** rawRequiredExts = glfwGetRequiredInstanceExtensions(&glfwRequiredExtCount);
		std::vector<const char*> allRequiredExts(rawRequiredExts, rawRequiredExts + glfwRequiredExtCount);

#ifndef NDEBUG
		allRequiredExts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		std::cout << "Extensions required:\n";
		for (const char* extension : allRequiredExts) {
			std::cout << '\t' << std::string(extension) << '\n';
		}

		// Retrieve supported extensions from Vulkan
		uint32_t vkSupportedExtCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &vkSupportedExtCount, nullptr);
		std::vector<VkExtensionProperties> vkSupportedExts(vkSupportedExtCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &vkSupportedExtCount, vkSupportedExts.data());

		std::cout << "Extensions supported by Vulkan:\n";
		for (const auto& vkSupportedExt : vkSupportedExts) {
			std::cout << '\t' << vkSupportedExt.extensionName << '\n';
		}

		// Make sure the drivers on the system support the required extensions.
		for (const char* requiredExt : allRequiredExts) {
			if (!std::any_of(
				vkSupportedExts.begin(),
				vkSupportedExts.end(),
				[requiredExt](VkExtensionProperties ext) {
					return !strcmp(ext.extensionName, requiredExt);
				})) {
				std::string required = "Vulkan driver missing required extension " + std::string(requiredExt);
				throw std::runtime_error(required);
			}
		}

		createInfo.enabledExtensionCount = allRequiredExts.size();
		createInfo.ppEnabledExtensionNames = allRequiredExts.data();


#ifndef NDEBUG
		// We forward declare "debugCreateInfo" here so that it is still in-scope when our instance is actually created (ensuring it is not deleted before it can be used)
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		uint32_t vkLayerCount = 0;
		vkEnumerateInstanceLayerProperties(&vkLayerCount, nullptr);
		std::vector<VkLayerProperties> vkAvailableLayers(vkLayerCount);
		vkEnumerateInstanceLayerProperties(&vkLayerCount, vkAvailableLayers.data());

		std::cout << "Validation layers available:\n";
		for (auto& layerProperties : vkAvailableLayers) {
			std::cout << '\t' << layerProperties.layerName << '\n';
		}

		bool allLayersAvailable = true;
		std::cout << "Validation layers requested:\n";
		for (const char* layerName : validationLayers) {
			std::cout << '\t' << layerName << '\n';

			allLayersAvailable &= std::any_of(
				vkAvailableLayers.begin(),
				vkAvailableLayers.end(),
				[layerName](VkLayerProperties availableLayer) {
					return !strcmp(availableLayer.layerName, layerName);
				});
		}

		if (!allLayersAvailable) {
			throw std::runtime_error("Validation layers requested, but not available!");
		}

		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#else
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
#endif

		if (vkCreateInstance(&createInfo, nullptr, &instance)) {
			throw std::runtime_error("Failed to create instance!");
		}

	}

#ifndef NDEBUG
	void setupDebugMessenger() {

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("Failed to set up debug mesenger!");
		}
	}
#endif

	void createSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface!");
		}
	}

	QueueFamilyIndices findQueueFamilyIndices(VkPhysicalDevice device) {
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
		
		std::cout << "Required device extensions:\n";
		for (const std::string& requiredExtension : requiredExtensions) {
			std::cout << '\t' << requiredExtension << '\n';
		}
		
		std::cout << "Available device extensions:\n";
		for (const auto& extension : availableExtensions) {
			std::cout << '\t' << extension.extensionName << '\n';
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount > 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount > 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	bool isDeviceSuitable(VkPhysicalDevice device) {
		QueueFamilyIndices indices = findQueueFamilyIndices(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty();
			swapChainAdequate &= !swapChainSupport.presentModes.empty();
		}

		return indices.graphicsFamily.has_value() &&
			extensionsSupported &&
			swapChainAdequate;
	}
	
	void pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		
		std::vector<VkPhysicalDeviceProperties> allDeviceProperties(deviceCount);
		std::cout << "Vulkan is supported on the following system devices:\n";
		for (int i = 0; i < deviceCount; i++) {
			VkPhysicalDevice device = devices[i];
			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			allDeviceProperties.push_back(deviceProperties);

			std::cout << '\t' << i << ": " << deviceProperties.deviceName << '\n';
			std::cout << "\t\t" << "Device ID: "		<< '\t' << deviceProperties.deviceID		<< '\n';
			std::cout << "\t\t" << "Device Type: "		<< '\t' << deviceProperties.deviceType		<< '\n';
			std::cout << "\t\t" << "Driver Version: "			<< deviceProperties.driverVersion	<< '\n';
			std::cout << "\t\t" << "Vendor ID: "		<< '\t' << deviceProperties.vendorID		<< '\n';
			std::cout << "\t\t" << "API Version: "		<< '\t' << deviceProperties.apiVersion		<< '\n';
		}

		// For now, we just pick the first device (Vulkan Tutorial pg. 62 for suitability checks)
		VkPhysicalDevice device = devices[0];
		if (!isDeviceSuitable(device)) {
			throw std::runtime_error("Selected device does not support required extensions!");
		}

		physicalDevice = device;
	}

	void createLogicalDevice() {
		QueueFamilyIndices indices = findQueueFamilyIndices(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { 
			indices.graphicsFamily.value(), 
			indices.presentFamily.value() 
		};

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// We'll be doing more interesting things with this later...
		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

#ifndef NDEBUG
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
#else
		createInfo.enabledLayerCount = 0;
#endif
		
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create logical device!");
		}

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}
	
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		// Pick the first format we find which matches our specifications
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}
		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		// Use triple-buffering if available
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		// Otherwise use "v-sync"
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		
		VkExtent2D actualExtent = { window_width, window_height };

		actualExtent.width =
			std::max(capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height =
			std::max(capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}

	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		// Simply sticking to the minimum number of images in the swap chain
		// may result in the application waiting on the driver to complete
		// internal operations before another image can be acquired for
		// rendering. So, request 1 more image than the minimum.
		// Clamp this value to the swap chain's max image count, if necessary.
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		uint32_t maxImageCount = swapChainSupport.capabilities.maxImageCount;
		if (maxImageCount > 0 &&
			imageCount > maxImageCount) {
			imageCount = maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		// This field indicates how the swap chain images will be used.
		// VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT - render color information directly to the image
		// VK_IMAGE_USAGE_TRANSFER_DST_BIT - render from another image (post-processing)
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = findQueueFamilyIndices(physicalDevice);
		uint32_t queueFamilyIndices[] = { 
			indices.graphicsFamily.value(),
			indices.presentFamily.value()
		};

		if (indices.graphicsFamily != indices.presentFamily) {
			// Avoids dealing with ownership for now (not the most performant code)
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		
		// Blending with the window system
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		
		createInfo.presentMode = presentMode;
		// Don't care about pixels which are blocked by other windows
		// If our rendering algorithms ever need those pixels, then disable this...
		createInfo.clipped = VK_TRUE;

		// For now, assume only 1 swapchain is ever created
		// Because of this, the window cannot be resized (for now)
		createInfo.oldSwapchain = VK_NULL_HANDLE;
		
		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
		
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	void createImageViews() {
		swapChainImageViews.resize(swapChainImages.size());

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create image views!");
			}
		}
	}

	void initVulkan() {
		createInstance();
#ifndef NDEBUG
		setupDebugMessenger();
#endif
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
	}
	
	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void cleanup() {
		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(device, swapChain, nullptr);
		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
#ifndef NDEBUG
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();
	}
};

int main(int argc, char** argv) {
	HelloTriangleApplication app;
	
	std::cout 
		<< "LearnVulkan Version "
		<< LEARN_VULKAN_VERSION_MAJOR
		<< "."
		<< LEARN_VULKAN_VERSION_MINOR
		<< "."
		<< LEARN_VULKAN_VERSION_PATCH
		<< "\n";

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}