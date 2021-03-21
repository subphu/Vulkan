//  Copyright © 2020 Subph. All rights reserved.
//

#pragma once

#include "common.h"

std::vector<VkImage>                 GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain);

uint32_t FindMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);


VkFormat            ChooseDepthFormat(VkPhysicalDevice physicalDevice);
VkSurfaceFormatKHR  ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR    ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
VkExtent2D          ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Size<int> size);

std::vector<char> ReadBinaryFile (const std::string filename);
unsigned char*    ReadImage(const std::string filename, int* width, int* height, int* channels);

uint32_t MaxMipLevel(int width, int height);
