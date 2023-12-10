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

#include "VBBDevice.h"

// ****************************************************************************
// Construct just stores a reference to the instance, and does nothing else
VBBDevice::VBBDevice(VkInstance instance) : m_vulkanInstance(instance) {}

VBBDevice::~VBBDevice() {
    vkDeviceWaitIdle(m_logicalDevice);
    vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);
    vkDestroyDevice(m_logicalDevice, nullptr);
}

// *****************************************************************************
// Allocate one or more command buffersfrom the pool

VkResult VBBDevice::allocateCommandBuffers(VkCommandBuffer* pCommandBuffers, uint32_t nCount) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = nCount;

    return vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, pCommandBuffers);
}

void VBBDevice::releaseCommandBuffers(VkCommandBuffer* pCommandBuffers, uint32_t nCount) {
    vkFreeCommandBuffers(m_logicalDevice, getCommandPool(), nCount, pCommandBuffers);
}
