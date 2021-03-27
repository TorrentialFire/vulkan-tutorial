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

	bool isComplete() {
		return graphicsFamily.has_value();
	}
};

QueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice device) {
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

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

class HelloTriangleApplication {
public:
	const uint32_t window_width = 800;
	const uint32_t window_height = 600;

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
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
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

	bool isDeviceSuitable(VkPhysicalDevice device) {
		QueueFamilyIndices indices = FindQueueFamilyIndices(device);

		return indices.graphicsFamily.has_value();
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
		physicalDevice = devices[0];
	}

	void createLogicalDevice() {
		QueueFamilyIndices indices = FindQueueFamilyIndices(physicalDevice);

		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;

		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		// We'll be doing more interesting things with this later...
		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = 0;

#ifndef NDEBUG
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
#else
		createInfo.enabledLayerCount = 0;
#endif
		
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create logical device!");
		}
	}

	void initVulkan() {
		createInstance();
#ifndef NDEBUG
		setupDebugMessenger();
#endif
		pickPhysicalDevice();
		createLogicalDevice();
	}
	
	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void cleanup() {
		vkDestroyDevice(device, nullptr);
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