//  Copyright © 2020 Subph. All rights reserved.
//

#pragma once

#include "common.h"

std::vector<VkPhysicalDevice>        GetPhysicalDevices(VkInstance instance);
std::vector<VkSurfaceFormatKHR>      GetSurfaceFormatKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
std::vector<VkPresentModeKHR>        GetSurfaceModeKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
std::vector<VkLayerProperties>       GetLayerProperties();
std::vector<VkExtensionProperties>   GetExtensionProperties(VkPhysicalDevice physicalDevice);
std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties(VkPhysicalDevice physicalDevice);
std::vector<VkImage>                 GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapChain);

bool CheckLayerSupport(std::vector<const char*> layers);
bool CheckDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char*> extensions);

int FindGraphicFamilyIndex(VkPhysicalDevice physicalDevice);
int FindPresentFamilyIndex(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                      const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                      const VkAllocationCallbacks* pAllocator,
                                      VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks* pAllocator);

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                             void* pUserData);


bool IsDeviceSuitable(VkPhysicalDevice physicalDevice,
                      VkSurfaceKHR surface,
                      std::vector<const char*> deviceExtensions);

VkFormat            ChooseDepthFormat(VkPhysicalDevice physicalDevice);
VkSurfaceFormatKHR  ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR    ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
VkExtent2D          ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Size<int> size);

std::vector<char> ReadFile (const std::string filename);
unsigned char*    ReadImage(const std::string filename, int* width, int* height, int* channels);

uint32_t MaxMapLevel(int width, int height);
