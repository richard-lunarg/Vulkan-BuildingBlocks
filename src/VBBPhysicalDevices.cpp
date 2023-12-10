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

#include "VBBPhysicalDevices.h"


// *******************************************************
// Constructor... all we do here is query for the number of devices,
// and get their properties. How we select which device we want can get
// complicated, and we want some easy to use ways to pick one, but
// we want to leave the flexibility to be very involved in our choice.
VBBPhysicalDevices::VBBPhysicalDevices(VkInstance instance) : m_vulkanInstance(instance) {
    uint32_t deviceCount = 0;

    // Find out how many physical devices are present
    m_lastResult = vkEnumeratePhysicalDevices(m_vulkanInstance, &deviceCount, nullptr);
    if (m_lastResult != VK_SUCCESS) return;

    // There must be at least one
    if (deviceCount == 0) return;

    // Get the actual list of devices
    m_physicalDevices.resize(deviceCount);
    m_lastResult = vkEnumeratePhysicalDevices(m_vulkanInstance, &deviceCount, m_physicalDevices.data());
    if (m_lastResult != VK_SUCCESS) return;

    // Get the properties of all devices found
    m_deviceProperties.resize(deviceCount);
    m_deviceFeatures.resize(deviceCount);
    m_deviceExtensions.resize(deviceCount);
    m_deviceMemoryProperties.resize(deviceCount);
    for (uint32_t i = 0; i < deviceCount; i++) {
        m_deviceProperties[i].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        vkGetPhysicalDeviceProperties2(m_physicalDevices[i], &m_deviceProperties[i]);

        vkGetPhysicalDeviceFeatures(m_physicalDevices[i], &m_deviceFeatures[i]);

        m_deviceMemoryProperties[i].sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
        vkGetPhysicalDeviceMemoryProperties2(m_physicalDevices[i], &m_deviceMemoryProperties[i]);

        uint32_t extCount = 0;
        vkEnumerateDeviceExtensionProperties(m_physicalDevices[i], nullptr, &extCount, nullptr);
        m_deviceExtensions[i].resize(extCount);
        vkEnumerateDeviceExtensionProperties(m_physicalDevices[i], nullptr, &extCount, m_deviceExtensions[i].data());
    }
}

// ***********************************************************************************
/*  This class instance has all the information it needs about the connected physical
    devices. THIS function is provided as a very simple logical device creation method
    that will suit most basic Vulkan applications. It is delcared virtual on purpose
    to facilitate replacing it with an applicaiton specific. A more elaborate selection
    function can also easily be added to child class.

    nDeviceOverride can be used to force a particular GPU be used.
    queueType can be any combination of queue flags. By default it is just a graphics queue
 */
VkResult VBBPhysicalDevices::createLogicalDevice(VBBDevice* pLogicalDevice, VkQueueFlags queueType, int nDeviceOverride) {
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    // If we are going to do graphics, you have to have a KHR_swapchain
    if (queueType & VK_QUEUE_GRAPHICS_BIT) {
        pLogicalDevice->addRequiredDeviceExtension("VK_KHR_swapchain");  // Must always have for drawing
    }

    // If we did an overrride, we are done looking
    if (nDeviceOverride >= 0 && nDeviceOverride >= int(m_physicalDevices.size()))
        physicalDevice = m_physicalDevices[nDeviceOverride];
    else {
        // Look for a discrete GPU rather than integrated (VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU).
        // Discrete are usually faster/more memory if they are present,
        // but if there's only one device, no matter what it is, we have to take it
        for (uint32_t i = 0; i < m_physicalDevices.size(); i++) {
            physicalDevice = m_physicalDevices[i];
            nDeviceOverride = i;
            if (m_deviceProperties[i].properties.deviceType ==
                VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)  // VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
                break;
        }
    }

    // If we could not find a physical device, we need to bail
    if (physicalDevice == VK_NULL_HANDLE) return VK_ERROR_INCOMPATIBLE_DRIVER;

    // Find a queue family that works.
    uint32_t familyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
    m_deviceQueueFamilyProperties.resize(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, m_deviceQueueFamilyProperties.data());

    const float defaultQueuePriority(1.0f);
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    VkBool32 found = VK_FALSE;
    for (uint32_t i = 0; i < static_cast<uint32_t>(m_deviceQueueFamilyProperties.size()); i++) {
        if ((m_deviceQueueFamilyProperties[i].queueFlags & queueType)) {
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = i;
            queueCreateInfo.queueCount = 1;
            pLogicalDevice->m_queueFamilyIndex = i;
            queueCreateInfo.pQueuePriorities = &defaultQueuePriority;
            found = VK_TRUE;
            break;
        }
    }

    // What if we didn't find a queue?
    if (found == VK_FALSE) return VK_ERROR_UNKNOWN;  // We don't really have a good "Vulkan" error message for this

    // Enable optional features? Etc. Etc. Etc.
    VkPhysicalDeviceFeatures2KHR physicalDeviceFeatures2 = {};
    physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;

    VkPhysicalDevicePortabilitySubsetFeaturesKHR portabilityFeatures = {};
    portabilityFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PORTABILITY_SUBSET_FEATURES_KHR;

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    // If the device supports the portability subset extnesion, then we
    // must enable it.
    if (isExtensionSupported(nDeviceOverride, "VK_KHR_portability_subset")) {
        pLogicalDevice->addRequiredDeviceExtension("VK_KHR_portability_subset");

        // Get and enable all portability features that are available
        physicalDeviceFeatures2.pNext = &portabilityFeatures;
    }

    vkGetPhysicalDeviceFeatures2(physicalDevice, &physicalDeviceFeatures2);

    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.enabledExtensionCount = (uint32_t)pLogicalDevice->m_requiredDeviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = pLogicalDevice->m_requiredDeviceExtensions.data();
    deviceCreateInfo.pEnabledFeatures = nullptr;
    deviceCreateInfo.pNext = &physicalDeviceFeatures2;

    VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &pLogicalDevice->m_logicalDevice);
    if (result != VK_SUCCESS) return result;

    // Logical device might want to know who it's daddy is.
    pLogicalDevice->m_physicalDevice = physicalDevice;

    // Get the queue
    vkGetDeviceQueue(pLogicalDevice->m_logicalDevice, queueCreateInfo.queueFamilyIndex, 0, &pLogicalDevice->m_primaryQueue);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueCreateInfo.queueFamilyIndex;

    return vkCreateCommandPool(pLogicalDevice->m_logicalDevice, &poolInfo, nullptr, &pLogicalDevice->m_commandPool);
}
