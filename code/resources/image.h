//  Copyright © 2021 Subph. All rights reserved.
//

#pragma once

#include "../common.h"

class Renderer;

class Image {
    
public:
    ~Image();
    Image();

    void cleanup();
    void cleanupImageView();
    
    void setupForDepth     (Size<uint32_t> size, uint32_t mipLevels);
    void setupForSwapchain (VkImage image, VkFormat imageFormat);
    void setupForTexture   (const std::string filepath);
    void setupForHDRTexture(const std::string filepath);
    void setupForCubemap   (const std::string *filepaths);
    void setupForCubemap   (Size<uint> size);
    
    void create             ();
    void createForTexture   ();
    void createForCubemap   ();
    void createForSwapchain ();
    
    void createImage        ();
    void createImageView    ();
    void allocateImageMemory();
    void createSampler      ();
    
    void copyRawHDRToImage  ();
    void copyRawDataToImage ();
    void copyCubemapToImage ();
    
    void cmdTransitionToTransferDest();
    void cmdCopyBufferToImage      (VkBuffer buffer);
    void cmdGenerateMipmaps        ();
    
    VkImage          getImage      ();
    VkImageView      getImageView  ();
    VkDeviceMemory   getImageMemory();
    VkDeviceSize     getImageSize  ();
    VkSampler        getSampler    ();
    unsigned int     getChannelSize();
    VkDescriptorImageInfo getImageInfo();
    
    VkImageCreateInfo     m_imageInfo{};
    VkImageViewCreateInfo m_imageViewInfo{};
    
    
private:
    
    unsigned char* m_desc;
    unsigned char* m_rawData;
    float        * m_rawHDR;
    std::vector<unsigned char*> m_rawCubemap;
    
    VkDevice         m_device         = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    
    VkImage          m_image          = VK_NULL_HANDLE;
    VkImageView      m_imageView      = VK_NULL_HANDLE;
    VkDeviceMemory   m_imageMemory    = VK_NULL_HANDLE;
    
    // For Texture
    VkSampler m_sampler = VK_NULL_HANDLE;
    
    static unsigned int GetChannelSize(VkFormat format);
    static VkFormat ChooseDepthFormat(VkPhysicalDevice physicalDevice);
    static VkImageCreateInfo     GetDefaultImageCreateInfo();
    static VkImageViewCreateInfo GetDefaultImageViewCreateInfo();
    static VkImageMemoryBarrier  GetDefaultImageMemoryBarrier();
    
};
