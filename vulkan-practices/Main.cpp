
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <optional>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    bool isComplete() const {
        return graphicsFamily.has_value();
    }
};


class HelloTriangleApplication {
public:
    void run() {
        init();
        mainLoop();
        cleanup(); 
    }

private:
    void init() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(Width, Height, "Vulkan Window", nullptr, nullptr);

        std::uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::cout << extensionCount << " extensions supported" << std::endl;

        createInstance();
        pickPhysicalDevice();
    }


    void pickPhysicalDevice() {
        std::vector<VkPhysicalDevice> devices = this->enumeratePhysicalDevices();

        if (devices.size() == 0) {
            throw std::runtime_error("there is no GPUs with vulkan support on your system");
        }

        for (const VkPhysicalDevice &device : devices) {
            if (this->isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU");
        }
    }


    bool isDeviceSuitable(VkPhysicalDevice device) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(device, &features);

        QueueFamilyIndices familyIndices = findQueueFamilies(device);

        return  /*properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU 
                && */features.geometryShader
                && familyIndices.isComplete();
    }
    

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        std::vector<VkQueueFamilyProperties> families = getFamilyProperties(device);

        for (int i = 0; i<families.size(); i++) {
            if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            if (indices.isComplete()) {
                break;
            }
        }
        
        return indices;
    }


    std::vector<VkQueueFamilyProperties> getFamilyProperties(VkPhysicalDevice device) {
        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

        std::vector<VkQueueFamilyProperties> properties(count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &count, properties.data());

        return properties;
    }


    std::vector<VkPhysicalDevice> enumeratePhysicalDevices() const {
        uint32_t count = 0;
        vkEnumeratePhysicalDevices(instance, &count, nullptr);

        std::vector<VkPhysicalDevice> devices(count);
        vkEnumeratePhysicalDevices(instance, &count, devices.data());

        return devices;
    }


    void createInstance() {
        if (enabledValidationLayers() && !checkValidationLayers()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "XE";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions = nullptr;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;

        // validation layers
        if (enabledValidationLayers()) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (VkResult result = vkCreateInstance(&createInfo, nullptr, &instance); result != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance. error code: " + std::to_string(result));
        }
    }


    void mainLoop() {
        while (!::glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }


    void cleanup() {
        if (instance) {
            vkDestroyInstance(instance, nullptr);
            instance = nullptr;
        }

        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }

        glfwTerminate();
    }


    bool enabledValidationLayers() const {
#if NDEBUG
        return false;
#else
        return true;
#endif
    }


    std::vector<VkLayerProperties> getAvailableLayers() const {
        uint32_t count;
        vkEnumerateInstanceLayerProperties(&count, nullptr);

        std::vector<VkLayerProperties> layers(count);
        vkEnumerateInstanceLayerProperties(&count, layers.data());

        return layers;
    }


    bool checkValidationLayers() const {
        auto availableLayers = this->getAvailableLayers();

        for (const char *layerName : validationLayers) {
            bool layerFound = false;

            for (const VkLayerProperties &layerProperties : availableLayers) {
                if (std::strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;

                    break;
                }
            }

            if (! layerFound) {
                return false;
            }
        }

        return true;
    }

private:
    const int Width = 800, Height = 600;
    GLFWwindow *window = nullptr;
    VkInstance instance = nullptr;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    std::vector<const char*> validationLayers {
        "VK_LAYER_KHRONOS_validation"
    };
};


int main(int argc, char **argv) {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception &exp) {
        std::cerr << exp.what() << std::endl;

        return EXIT_FAILURE;       
    }

    return EXIT_SUCCESS;
}
