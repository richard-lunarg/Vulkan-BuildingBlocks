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

#include "VBBBufferDynamic.h"
#include <memory.h>

VBBBufferDynamic::~VBBBufferDynamic() { free(); }

void VBBBufferDynamic::free(void) {
    if (m_buffer != VK_NULL_HANDLE) vmaDestroyBuffer(m_VMA, m_buffer, m_allocation);

    m_buffer = VK_NULL_HANDLE;
}

VkResult VBBBufferDynamic::createBuffer(VkDeviceSize size) {
    m_bufferSize = size;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;

    // Really flexible...
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                       VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
    return vmaCreateBuffer(m_VMA, &bufferInfo, &allocInfo, &m_buffer, &m_allocation, nullptr);
}

void VBBBufferDynamic::updateBuffer(void* pSrcData, VkDeviceSize bytes) {
    void* pDst = mapMemory();
    memcpy(pDst, pSrcData, bytes);
    unmapMemory();
}

void* VBBBufferDynamic::mapMemory(void) {
    void* pData;
    vmaMapMemory(m_VMA, m_allocation, &pData);
    return pData;
}

void VBBBufferDynamic::unmapMemory(void) { vmaUnmapMemory(m_VMA, m_allocation); }
