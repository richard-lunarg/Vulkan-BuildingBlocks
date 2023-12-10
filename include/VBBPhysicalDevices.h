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
/*
    This class is disposable. Create it temporarily to enumerate physical devices on the system,
    and create a logical device with desired queue properties. Then it can be disposed of.
 */
#pragma once

#include "VBBInstance.h"
#include "VBBDevice.h"
#include <cstring>

class VBBPhysicalDevices {
  public:
    VBBPhysicalDevices(VkInstance instance);

    inline uint32_t getDeviceCount(void) { return uint32_t(m_physicalDevices.size()); }
    virtual VkResult createLogicalDevice(VBBDevice* pLogicalDevice, VkQueueFlags queueType = VK_QUEUE_GRAPHICS_BIT,
                                         int nDeviceOverride = -1);

    inline VkResult getLastResult(void) { return m_lastResult; }
    inline void getAPIVersion(uint32_t deviceIndex, uint32_t& major, uint32_t& minor, uint32_t& patch) {
        uint32_t version = m_deviceProperties[deviceIndex].properties.apiVersion;
        major = VK_API_VERSION_MAJOR(version);
        minor = VK_API_VERSION_MINOR(version);
        patch = VK_API_VERSION_PATCH(version);
    }

    inline void getDriverVersion(uint32_t deviceIndex, uint32_t& major, uint32_t& minor, uint32_t& patch) {
        uint32_t version = m_deviceProperties[deviceIndex].properties.driverVersion;
        major = VK_API_VERSION_MAJOR(version);
        minor = VK_API_VERSION_MINOR(version);
        patch = VK_API_VERSION_PATCH(version);
    }

    // Check for a device extension
    VkBool32 isExtensionSupported(uint32_t deviceIndex, const char* szExtension) {
        for (size_t i = 0; i < m_deviceExtensions[deviceIndex].size(); i++)
            if (strcmp(szExtension, m_deviceExtensions[deviceIndex][i].extensionName) == 0) return VK_TRUE;

        return VK_FALSE;
    }

    // Get arrays of device properties, features, queues
    const std::vector<VkPhysicalDevice>& getPysicalDevices(void) const { return m_physicalDevices; }
    const std::vector<VkPhysicalDeviceProperties2>& getDeviceProperties(void) const { return m_deviceProperties; }
    const std::vector<VkPhysicalDeviceFeatures>& getDeviceFeatures(void) const { return m_deviceFeatures; }
    const std::vector<VkPhysicalDeviceMemoryProperties2>& getDeviceMemoryProperties(void) const { return m_deviceMemoryProperties; }
    const std::vector<VkQueueFamilyProperties>& getQueueFamilyProperties(void) const { return m_deviceQueueFamilyProperties; }

    // Get a list of device extensions for a particular physical device
    const std::vector<VkExtensionProperties>& getDeviceExtensionProperties(int nDeviceIndex) {
        return m_deviceExtensions[nDeviceIndex];
    }

  protected:
    VkInstance m_vulkanInstance = VK_NULL_HANDLE;
    VkResult m_lastResult = VK_SUCCESS;

    std::vector<VkPhysicalDevice> m_physicalDevices;
    std::vector<VkPhysicalDeviceProperties2> m_deviceProperties;
    std::vector<VkPhysicalDeviceFeatures> m_deviceFeatures;
    std::vector<VkPhysicalDeviceMemoryProperties2> m_deviceMemoryProperties;
    std::vector<VkQueueFamilyProperties> m_deviceQueueFamilyProperties;

    std::vector<std::vector<VkExtensionProperties>> m_deviceExtensions;  // There's a list for each device
};
