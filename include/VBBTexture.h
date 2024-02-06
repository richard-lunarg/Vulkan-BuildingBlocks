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
#pragma once

#ifdef VK_NO_PROTOTYPES
#include <volk/volk.h>
#else
#include <vulkan/vulkan.h>
#endif

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vma/vk_mem_alloc.h"

#include "VBBDevice.h"
#include "VBBBufferDynamic.h"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>

class VBBTexture {
  public:
    VBBTexture(VmaAllocator Allocator, VBBDevice* pLogicalDevice);
    ~VBBTexture();

    VkSampler getSampler(void) { return textureSampler; }
    VkImageView getImageView(void) { return textureImageView; }
    VkFormat getFormat(void) { return imageFormat; }
    VkImageLayout getLayout(void) { return imageLayout; }
    void setFinalLayout(VkImageLayout layout) { imageLayout = layout; }

    uint32_t getWidth() { return textureWidth; }
    uint32_t getHeight() { return textureHeight; }

    // Create a texture from raw data
    bool loadRawTexture(const void* pImageData, VkFormat format, uint32_t channels, uint32_t width, uint32_t height, uint32_t totalBytes,
                        int mipLevels = 1);
    bool loadRawTexture(VBBBufferDynamic& imageBuffer, VkFormat format, uint32_t channels, uint32_t width, uint32_t height,
                        uint32_t totalBytes, int mipLevels = 1);

    // ******************************************************************
    // Defaults, override before loading texture
    VkFilter minFilter = VK_FILTER_LINEAR;
    VkFilter magFilter = VK_FILTER_LINEAR;
    VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL;

  protected:
    void createTextureImageView(void);
    void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkDeviceSize offset = 0,
                           int nMipLevel = 0);

    void createSampler(void);

    uint32_t textureWidth;
    uint32_t textureHeight;
    uint32_t textureChannels;
    uint32_t mipMapLevels;

    VkDeviceSize imageSize;

    VmaAllocator  m_VMA = VK_NULL_HANDLE;
    VmaAllocation m_allocation;

    VkImage textureImage = VK_NULL_HANDLE;
    VkImageView textureImageView = VK_NULL_HANDLE;
    VkFormat imageFormat;

    VkSampler textureSampler = VK_NULL_HANDLE;

    VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkPhysicalDevice physicalDevice;
    VkDevice m_Device;
    VkQueue graphicsQueue;
    VkCommandPool commandPool;
};
