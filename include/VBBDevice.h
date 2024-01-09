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
#include <cstring>
#include "VBBInstance.h"

class VBBDevice {
    // This class is usualy made by the VkPhysicalDevices, so it needs access to it's internal structures
    friend class VBBPhysicalDevices;

  public:
    VBBDevice(VkInstance instance);
    ~VBBDevice();

    VkInstance getInstance(void) { return m_vulkanInstance; }
    VkDevice getDevice(void) { return m_logicalDevice; }
    VkPhysicalDevice getPhysicalDeviceHandle(void) { return m_physicalDevice; }
    VkCommandPool getCommandPool(void) { return m_commandPool; }
    VkQueue getQueue(void) { return m_primaryQueue; }

    VkResult allocateCommandBuffers(VkCommandBuffer* pCommandBuffers, uint32_t nCount);
    void releaseCommandBuffers(VkCommandBuffer* pCommandBuffers, uint32_t nCount);

    inline void addRequiredDeviceExtension(const char* extensionName) { m_requiredDeviceExtensions.push_back(extensionName); }

  protected:
    VkInstance m_vulkanInstance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_logicalDevice = VK_NULL_HANDLE;

    VkQueue m_primaryQueue = VK_NULL_HANDLE;
    uint32_t m_queueFamilyIndex = 0;

    VkCommandPool m_commandPool = VK_NULL_HANDLE;

    std::vector<const char*> m_requiredDeviceExtensions;
};
