//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"

class Renderer;

class ResourceImage {
    
public:
    ~ResourceImage();
    ResourceImage();

    void cleanup();
    void cleanupImageView();
    
    unsigned char* m_desc;
    unsigned char* m_rawData;
    
    VkDevice         m_device         = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    
    VkImage          m_image          = VK_NULL_HANDLE;
    VkImageView      m_imageView      = VK_NULL_HANDLE;
    VkDeviceMemory   m_imageMemory    = VK_NULL_HANDLE;
    
    VkImageCreateInfo     m_imageInfo{};
    VkImageViewCreateInfo m_imageViewInfo{};
    
    // For Texture
    VkSampler m_sampler = VK_NULL_HANDLE;
    
    void setupForDepth     (Size<uint32_t> size, uint32_t mipLevels);
    void setupForSwapchain (VkImage image, VkFormat imageFormat);
    void setupForTexture   (const std::string filepath);
    
    void create             ();
    void createForTexture   ();
    void createForSwapchain ();
    
    void createImage        ();
    void createImageView    ();
    void allocateImageMemory();
    void createSampler      ();
    
    void copyRawDataToImage ();
    
    void cmdTransitionToTransferDst();
    void cmdCopyBufferToImage      (VkBuffer buffer);
    void cmdGenerateMipmaps        ();
    
    VkImage          getImage      ();
    VkImageView      getImageView  ();
    VkDeviceMemory   getImageMemory();
    VkDeviceSize     getImageSize  ();
    VkSampler        getSampler    ();
    unsigned int     getChannelSize();
    
    
private:
    
    static unsigned int GetChannelSize(VkFormat format);
    static VkFormat ChooseDepthFormat(VkPhysicalDevice physicalDevice);
    static VkImageCreateInfo     GetDefaultImageCreateInfo();
    static VkImageViewCreateInfo GetDefaultImageViewCreateInfo();
    static VkImageMemoryBarrier  GetDefaultImageMemoryBarrier();
    
};
