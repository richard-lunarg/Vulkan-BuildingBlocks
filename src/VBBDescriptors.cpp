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

#include "VBBDescriptors.h"

#include <stdarg.h>

VBBDescriptors::VBBDescriptors(void) {}

VBBDescriptors::~VBBDescriptors(void) {
    if (m_device != VK_NULL_HANDLE && m_descriptorSet != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);

    if (m_device != VK_NULL_HANDLE && m_descriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
}

// ************************************************************************
// I have some bindings... and they are each for "this type" of buffer, there are count many of them, and they are for "this type"
// of stage.
//VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//binding number
// VK_SHADER_STAGE_COMPUTE_BIT
//Rinse, Repeat...
VkResult VBBDescriptors::init(VkDevice device, uint32_t framesInFlight, uint32_t descriptorCount, ...) {
    m_device = device;
    va_list argList;
    va_start(argList, descriptorCount);

    for (uint32_t i = 0; i < descriptorCount; i++) {
        VkDescriptorSetLayoutBinding binding = {};
        binding.descriptorType = va_arg(argList, VkDescriptorType);
        binding.binding = va_arg(argList, uint32_t);
        binding.descriptorCount = 1;
        binding.stageFlags = va_arg(argList, VkShaderStageFlagBits);
        binding.pImmutableSamplers = nullptr;
        m_layoutBindings.push_back(binding);
    }
    va_end(argList);

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = nullptr;
    layoutInfo.flags = 0;
    layoutInfo.bindingCount = descriptorCount;
    layoutInfo.pBindings = m_layoutBindings.data();

    m_lastResult = vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout);
    if (m_lastResult != VK_SUCCESS) return m_lastResult;

    // ******************************************************************************************
    // Descriptor sets must be allocated in a Descriptor pool
    // The pool needs to contain at least one of all the different kinds of
    // descriptors we need from above
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (uint32_t i = 0; i < descriptorCount; i++) {
        // Make a struct for this one
        VkDescriptorPoolSize poolSize;
        poolSize.type = m_layoutBindings[i].descriptorType;
        poolSize.descriptorCount = 1;

        // Before we add it, let's make sure it's not already in the list
        bool bFound = false;
        for (uint32_t p = 0; p < poolSizes.size(); p++) {
            // If we already have one, just increment it's counter
            if (m_layoutBindings[i].descriptorType == poolSizes[p].type) {
                poolSizes[p].descriptorCount++;
                bFound = true;
                break;
            }
        }

        // If we get there, push it on the list
        if (!bFound) poolSizes.push_back(poolSize);
    }

    // Create the descriptor pool
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.pNext = nullptr;
    pool_info.flags = 0;
    pool_info.maxSets = framesInFlight;
    pool_info.poolSizeCount = uint32_t(poolSizes.size());
    pool_info.pPoolSizes = poolSizes.data();
    m_lastResult = vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_descriptorPool);
    if (m_lastResult != VK_SUCCESS) return m_lastResult;

    // ***********************************************************************************
    // Now we can finally allocate the DESCRIPTOR SETS and update them to use our buffers.
    // Allocate the descriptor sets
    VkDescriptorSetAllocateInfo dsAllocInfo = {};
    dsAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;  // using the pool we just set
    dsAllocInfo.pNext = nullptr;
    dsAllocInfo.descriptorPool = m_descriptorPool;  // only 1 descriptor
    dsAllocInfo.descriptorSetCount = 1;             // FOR FRAMES IN FLIGHTt
    dsAllocInfo.pSetLayouts = &m_descriptorSetLayout;

    m_lastResult = vkAllocateDescriptorSets(device, &dsAllocInfo, &m_descriptorSet);
    if (m_lastResult != VK_SUCCESS) return m_lastResult;

    return VK_SUCCESS;
}
