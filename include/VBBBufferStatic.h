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
/* Vulkan static buffer is stored on the GPU, and is typically
   not visible by the CPU. It is loaded by copying contents from a VulkanDynamicBuffer
*/

#include "VBBBufferDynamic.h"
#include "VBBDevice.h"
///////////////////////////////////////////////////////////////
// Create a dynamic buffer
class VBBBufferStatic {
  public:
    VBBBufferStatic(VmaAllocator allocator);
    ~VBBBufferStatic(void);

    VkResult createBuffer(VkDeviceSize size);
    VkResult createBuffer(void* pData, VkDeviceSize size, VBBDevice& logicalDevice);

    inline VkBuffer getBuffer(void) { return m_buffer; }
    inline size_t getBufferSize(void) { return m_bufferSize; }
    void free(void);

    // Many ways to update this buffer
    bool updateBuffer(VBBBufferDynamic& dynamicBuffer, VBBDevice& logicalDevice, VkDeviceSize srcOffset = 0,
                      VkDeviceSize dstOffset = 0, VkDeviceSize size = 0);

    bool updateBuffer(void* pData, VkDeviceSize size, VBBDevice& logicalDevice);

  protected:
    VmaAllocator m_VMA = VK_NULL_HANDLE;
    VmaAllocation m_allocation;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    size_t m_bufferSize;
};
