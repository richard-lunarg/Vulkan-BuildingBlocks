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

#include "VBBTextureStreaming.h"
#include "VBBSingleShotCommand.h"

// *****************************************************************************************************************
// Constructor just stores data, does no real work that can fail
VBBTextureStreaming::VBBTextureStreaming(VmaAllocator Allocator, VBBDevice& device) {
    m_VMA = Allocator;
    m_physicalDevice = device.getPhysicalDeviceHandle();
    m_device = device.getDevice();
    m_graphicsQueue = device.getQueue();
    m_commandPool = device.getCommandPool();
}

// *****************************************************************************************************************
// Cleanup any residual objects and memory
VBBTextureStreaming::~VBBTextureStreaming() {
    if (m_textureSampler != VK_NULL_HANDLE) vkDestroySampler(m_device, m_textureSampler, nullptr);

    if (m_textureImageView != VK_NULL_HANDLE) vkDestroyImageView(m_device, m_textureImageView, nullptr);

    vmaDestroyImage(m_VMA, m_textureImage, m_allocation);
}

// ************************************************************************************************************************
// Initalize to maximum storage. Format is baked in.
// TBD: bytesPerPixel can be derived from format... write a helper function somewhere to do this
bool VBBTextureStreaming::createTexture(VBBBufferDynamic& textureData, uint32_t maxWidth, uint32_t maxHeight, uint32_t bytesPerPixel,
                                       VkFormat format) {
    maxImageSize = maxWidth * maxHeight * bytesPerPixel;
    currImageSize = maxImageSize;
    currImageFormat = format;
    currTextureWidth = maxWidth;
    currTextureHeight = maxHeight;

    this->bytesPerPixel = bytesPerPixel;

    // Fully create the texture
    // Create the image
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = currTextureWidth;
    imageInfo.extent.height = currTextureHeight;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = currImageFormat;
    imageInfo.tiling = imageTiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    VmaAllocationCreateInfo texAllocInfo = {};
    texAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    texAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
    vmaCreateImage(m_VMA, &imageInfo, &texAllocInfo, &m_textureImage, &m_allocation, nullptr);
    transitionImageLayout(m_textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Copy from staging to the actual GPU texture
    copyBufferToImage(textureData.getBuffer(), m_textureImage, currTextureWidth, currTextureHeight);

    // Do the transfer along with any layout changes
    transitionImageLayout(m_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    createImageView();
    createSampler();

    return true;
}

// *************************************************************************************************************************************
// Update a texture.
bool VBBTextureStreaming::updateTexture(VBBBufferDynamic& textureData) {
    // Okay, easy, just replace the texture bits...
    transitionImageLayout(m_textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // Update it for reals
    copyBufferToImage(textureData.getBuffer(), m_textureImage, currTextureWidth, currTextureHeight);

    transitionImageLayout(m_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    return true;
}

void VBBTextureStreaming::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkDeviceSize offset,
                                           int nMipLevel) {
    VBBSingleShotCommand singleShot(m_device, m_commandPool, m_graphicsQueue);
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

// ******************************************************************************************************************
// Sets up for the transitiion operatioin
void VBBTextureStreaming::transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VBBSingleShotCommand singleShot(m_device, m_commandPool, m_graphicsQueue);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
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

void VBBTextureStreaming::createImageView(void) {
    // If already set, just return
    if (m_textureImageView != VK_NULL_HANDLE) return;

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_textureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = currImageFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(m_device, &viewInfo, nullptr, &m_textureImageView);
}

void VBBTextureStreaming::createSampler() {
    // If already setup, just return
    if (m_textureSampler != VK_NULL_HANDLE) return;

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = this->magFilter;
    samplerInfo.minFilter = this->minFilter;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;

    vkCreateSampler(m_device, &samplerInfo, nullptr, &m_textureSampler);
}
