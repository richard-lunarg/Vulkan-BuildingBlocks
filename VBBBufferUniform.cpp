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
 *
 * This is really just a more specialized version of the VBBDynamicBuffer. It's specifically for uniforms,
 * and has a convienience function for copying the uniform data to the buffer.
 *
 */
#include "VBBBufferUniform.h"
#include <memory.h>
#include <stdio.h>

VBBBufferUniform::VBBBufferUniform(VmaAllocator allocator) {
    m_VMA = allocator;
    m_buffer = VK_NULL_HANDLE;
    m_bufferSize = 0;
}

void VBBBufferUniform::free(void) {
    if (m_buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(m_VMA, m_buffer, m_allocation);
        m_buffer = VK_NULL_HANDLE;
    }
}

VkResult VBBBufferUniform::createBuffer(VkDeviceSize size) {
    m_bufferSize = size;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
    return vmaCreateBuffer(m_VMA, &bufferInfo, &allocInfo, &m_buffer, &m_allocation, nullptr);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Copy in the data.
// I specifically don't think checking for nllptr is warranted here....
void VBBBufferUniform::updateUnform(void *pSrcData) {
    void *pBuffer = nullptr;
    vmaMapMemory(m_VMA, m_allocation, &pBuffer);
    memcpy(pBuffer, pSrcData, m_bufferSize);
    vmaUnmapMemory(m_VMA, m_allocation);
}
