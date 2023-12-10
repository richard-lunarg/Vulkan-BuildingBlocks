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

#include <memory.h>

#include "VBBTexture.h"
#include "VBBSingleShotCommand.h"

VBBTexture::VBBTexture(VmaAllocator allocator, VBBDevice &logicalDevice) {
    m_VMA = allocator;
    physicalDevice = logicalDevice.getPhysicalDeviceHandle();
    m_Device = logicalDevice.getDevice();
    graphicsQueue = logicalDevice.getQueue();
    commandPool = logicalDevice.getCommandPool();
    mipMapLevels = 1;
}

VBBTexture::~VBBTexture() {
    vkDestroySampler(m_Device, textureSampler, nullptr);
    vkDestroyImageView(m_Device, textureImageView, nullptr);
    vmaDestroyImage(m_VMA, textureImage, m_allocation);
}

void VBBTexture::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkDeviceSize offset,
                                   int nMipLevel) {
    VBBSingleShotCommand singleShot(m_Device, commandPool, graphicsQueue);
    singleShot.start();

    VkBufferImageCopy region = {};
    region.bufferOffset = offset;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = nMipLevel;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(singleShot.getCommandBuffer(), buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    singleShot.end();
}

void VBBTexture::transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VBBSingleShotCommand singleShot(m_Device, commandPool, graphicsQueue);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipMapLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    barrier.srcAccessMask = 0;  // TBD
    barrier.dstAccessMask = 0;  // TBD

    singleShot.start();

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        sourceStage = 0;
        destinationStage = 0;
        // FAILED REALLY
    }

    vkCmdPipelineBarrier(singleShot.getCommandBuffer(), sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    singleShot.end();
}

void VBBTexture::createTextureImageView(void) {
    if (textureImageView != VK_NULL_HANDLE) return;

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = textureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = imageFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipMapLevels;  // YES, CONFIRMED
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(m_Device, &viewInfo, nullptr, &textureImageView);
}

void VBBTexture::createSampler() {
    if (textureSampler != VK_NULL_HANDLE) return;

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = this->magFilter;
    samplerInfo.minFilter = this->minFilter;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(mipMapLevels);

    vkCreateSampler(m_Device, &samplerInfo, nullptr, &textureSampler);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Load a raw blob of data into a texture
/// TBD: Should be able to get channels from format... just say'n.
bool VBBTexture::loadRawTexture(void *pImageData, VkFormat format, uint32_t channels, uint32_t width, uint32_t height,
                                uint32_t totalBytes, int mipLevels) {
    imageSize = totalBytes;

    // Make this static (MEMBER, THAT IS, NOT A STATIC BUFFER), reserve the largest texture buffer possible, and reuse. TBD:
    VBBBufferDynamic tempBuffer(m_VMA);
    tempBuffer.createBuffer(imageSize);

    void *pData = tempBuffer.mapMemory();
    memcpy(pData, pImageData, imageSize);
    tempBuffer.unmapMemory();

    return loadRawTexture(tempBuffer, format, channels, width, height, totalBytes, mipLevels);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Load a raw blob into a texture from a buffer
/// TBD: Should be able to get channels from format... just say'n.
bool VBBTexture::loadRawTexture(VBBBufferDynamic &imageBuffer, VkFormat format, uint32_t channels, uint32_t width, uint32_t height,
                                uint32_t totalBytes, int mipLevels) {
    textureWidth = width;
    textureHeight = height;
    textureChannels = channels;
    imageFormat = format;
    mipMapLevels = mipLevels;
    imageSize = totalBytes;

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = textureWidth;
    imageInfo.extent.height = textureHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipMapLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.format = imageFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;// -> unless you want to also use it as image storage... VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    VmaAllocationCreateInfo texAllocInfo = {};
    texAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    texAllocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    vmaCreateImage(m_VMA, &imageInfo, &texAllocInfo, &textureImage, &m_allocation, nullptr);

    // Wait... wait... Look down below
    transitionImageLayout(textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    if (mipLevels == 1)
        copyBufferToImage(imageBuffer.getBuffer(), textureImage, textureWidth, textureHeight);
    else {
        VkDeviceSize offset = 0;
        uint32_t uiWidth = width;
        uint32_t uiHeight = height;

        for (int m = 0; m < mipLevels; m++) {
            copyBufferToImage(imageBuffer.getBuffer(), textureImage, uiWidth, uiHeight, offset, m);
            offset += uiWidth * uiHeight * channels;
            uiWidth /= 2;
            uiHeight /= 2;
        }
    }

    // Does this really need to be done.... look above?!?
    transitionImageLayout(textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    createTextureImageView();
    createSampler();
    return true;
}
