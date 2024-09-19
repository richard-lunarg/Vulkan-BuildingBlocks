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
/* Vulkan dynamic buffer is visible and accessable by both the GPU and the CPU.
   It can be used as a staging buffer for static buffer (on the GPU), or as
   data that can be updated on the fly between frames.
   This type of buffer is mapped to a pointer, then you can read/write to it.
*/
/*
 *  Richard S. Wright Jr.
 *  richard@lunarg.com
 *
 */
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vma/vk_mem_alloc.h"

///////////////////////////////////////////////////////////////
// Create a dynamic buffer
class VBBBufferDynamic {
  public:
    VBBBufferDynamic(VmaAllocator allocator) { m_VMA = allocator; }
    ~VBBBufferDynamic();

    VkResult createBuffer(VkDeviceSize size);

    inline VkBuffer getBuffer(void) { return m_buffer; }
    inline size_t getBufferSize(void) { return m_bufferSize; }

    void free(void);

    void updateBuffer(void* pSrcData, VkDeviceSize bytes);
    void* mapMemory(void);
    void unmapMemory(void);

  protected:
    VmaAllocator    m_VMA = VK_NULL_HANDLE;
    VmaAllocation   m_allocation = VK_NULL_HANDLE;
    VkBuffer        m_buffer = VK_NULL_HANDLE;
    size_t          m_bufferSize = 0;
};
