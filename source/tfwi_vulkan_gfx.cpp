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
#endif // !NDEBUG

class HelloTriangleApplication {
public:
	const uint32_t window_width = 800;
	const uint32_t window_height = 600;
	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "Validation layer: " << pCallbackData->pMessage << '\n';

		return VK_FALSE;
	}
#endif // !NDEBUG

	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	GLFWwindow* window;
	VkInstance instance;
#ifndef NDEBUG
	VkDebugUtilsMessengerEXT debugMessenger;
#endif // !NDEBUG

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
#endif // !NDEBUG
	
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

		if (enableValidationLayers) {
			allRequiredExts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

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

		// We forward declare "debugCreateInfo" here so that it is still in-scope when our instance is actually created (ensuring it is not deleted before it can be used)
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (enableValidationLayers) {

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
		}
		else {
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

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

	void initVulkan() {
		createInstance();
#ifndef NDEBUG
		setupDebugMessenger();
#endif
	}
	
	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void cleanup() {
#ifndef NDEBUG
		//DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
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