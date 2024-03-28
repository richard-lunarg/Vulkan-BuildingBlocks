/* Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Copyright © 2023 Richard S. Wright Jr. (richard@lunarg.com)
 *
 * This software is part of the Vulkan Building Blocks
 */

/* This is intended for dynamic, frequently updated 2D textures, just as a video or camera stream
   No mipmaps needed for this either
 */

#pragma once

#ifdef VK_NO_PROTOTYPES
#include <volk/volk.h>
#else
#include <vulkan/vulkan.h>
#endif

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vma/vk_mem_alloc.h"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>

#include "VBBBufferDynamic.h"
#include "VBBDevice.h"

class VBBTextureStreaming {
  public:
    VBBTextureStreaming(VmaAllocator allocator, VBBDevice& device);
    ~VBBTextureStreaming();

    // Initalize to maximum storage. Format is baked in.
    bool createTexture(VBBBufferDynamic& textureData, uint32_t maxWidth, uint32_t maxHeight, uint32_t bytesPerPixel,
                       VkFormat format);

    // Update a texture.
    bool updateTexture(VBBBufferDynamic& textureData);

    VkSampler getSampler(void) { return m_textureSampler; }
    VkImageView getImageView(void) { return m_textureImageView; }
    VkFormat getFormat(void) { return currImageFormat; }
    VkImageLayout getLayout(void) { return imageLayout; }
    uint32_t getWidth() { return currTextureWidth; }
    uint32_t getHeight() { return currTextureHeight; }

    // ******************************************************************
    // Defaults, overwrite before loading texture
    VkFilter minFilter = VK_FILTER_NEAREST;
    VkFilter magFilter = VK_FILTER_NEAREST;
    VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL;


  protected:
    void createImageView(void);
    void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkDeviceSize offset = 0,
                           int nMipLevel = 0);
    void createSampler(void);

    VkImageCreateInfo imageInfo = {};
    VmaAllocator  m_VMA = VK_NULL_HANDLE;
    VmaAllocation m_allocation;
    uint32_t currTextureWidth = 0;
    uint32_t currTextureHeight = 0;
    VkFormat currImageFormat;
    uint32_t bytesPerPixel = 0;

    VkDeviceSize currImageSize = 0;
    VkDeviceSize maxImageSize = 0;

    VkImage m_textureImage = VK_NULL_HANDLE;
    VkImageView m_textureImageView = VK_NULL_HANDLE;

    VkSampler m_textureSampler = VK_NULL_HANDLE;

    VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


    // Passed in at creation time
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    VkQueue m_graphicsQueue;
    VkCommandPool m_commandPool;
};
