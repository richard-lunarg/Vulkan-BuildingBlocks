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

#include "VBBBufferStatic.h"
#include "VBBSingleShotCommand.h"
#include <memory.h>

VBBBufferStatic::VBBBufferStatic(VmaAllocator allocator) { m_VMA = allocator; }

VBBBufferStatic::~VBBBufferStatic(void) { free(); }

void VBBBufferStatic::free(void) {
    if (m_buffer != VK_NULL_HANDLE) vmaDestroyBuffer(m_VMA, m_buffer, m_allocation);

    m_buffer = VK_NULL_HANDLE;
}

VkResult VBBBufferStatic::createBuffer(VkDeviceSize size) {
    m_bufferSize = size;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;

    // I can be used for lots of things
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                       VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
    vmaCreateBuffer(m_VMA, &bufferInfo, &allocInfo, &m_buffer, &m_allocation, nullptr);

    return VK_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copy a dynamic buffer (visible by both CPU and GPU) to a static buffer (visible/local only to GPU).
// Source and destination offsets may be set, and the size, if left -1 will be the entire buffer.
bool VBBBufferStatic::updateBuffer(VBBBufferDynamic& dynamicBuffer, VBBDevice* pLogicalDevice, VkCommandBuffer cmdBuffer,
                                   VkDeviceSize srcOffset, VkDeviceSize dstOffset, VkDeviceSize size) {
    VkDevice device = pLogicalDevice->getDevice();
    VkCommandPool commandPool = pLogicalDevice->getCommandPool();
    VkQueue queue = pLogicalDevice->getQueue();

    // Copy everything?
    if (size == 0) size = dynamicBuffer.getBufferSize();

    // Make sure the destination is big enough
    if (size > (m_bufferSize - dstOffset)) return false;

    // Make sure the source has the space too
    if ((dynamicBuffer.getBufferSize() - srcOffset) < m_bufferSize) return false;

    VBBSingleShotCommand singleShot(device, commandPool, queue);

    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size = size;
    if(cmdBuffer == VK_NULL_HANDLE) {
        singleShot.start();
        vkCmdCopyBuffer(singleShot.getCommandBuffer(), dynamicBuffer.getBuffer(), m_buffer, 1, &copyRegion);
        singleShot.end();
    } else
        vkCmdCopyBuffer(cmdBuffer, dynamicBuffer.getBuffer(), m_buffer, 1, &copyRegion);

    return VK_SUCCESS;
}

bool VBBBufferStatic::updateBuffer(void* pData, VkDeviceSize size, VBBDevice* pLogicalDevice) {
    VBBBufferDynamic temp(m_VMA);
    temp.createBuffer(size);
    void* p = temp.mapMemory();
    memcpy(p, pData, size);
    temp.unmapMemory();

    updateBuffer(temp, pLogicalDevice);
    return true;
}

VkResult VBBBufferStatic::createBuffer(void* pData, VkDeviceSize size, VBBDevice* pLogicalDevice) {
    VkResult result = createBuffer(size);
    if (result == VK_SUCCESS) updateBuffer(pData, size, pLogicalDevice);

    return result;
}
