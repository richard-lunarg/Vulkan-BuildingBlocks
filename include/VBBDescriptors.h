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

    The idea here is to just have one class that manages descriptors... non-attribute data passed to a shader.
    You create this and pass it the list of descriptors.
    It creates a descriptor pool from which descriptors can be allocated

*/

// TBD: Future idea... can we just parse a shader and build from that?
#pragma once

#include <stdio.h>
#include <vector>

#ifdef VK_NO_PROTOTYPES
#include <volk/volk.h>
#else
#include <vulkan/vulkan.h>
#endif

class VBBDescriptors {
  public:
    VBBDescriptors(void);
    virtual ~VBBDescriptors(void);
    VkResult init(VkDevice device, uint32_t framesInFlight, uint32_t descriptorCount, ...);

    VkResult getLastResult(void) { return m_lastResult; }
    VkDescriptorSetLayout getLayout(void) { return m_descriptorSetLayout; }
    VkDescriptorPool getPool(void) { return m_descriptorPool; }
    VkDescriptorSet getDescriptorSet(void) { return m_descriptorSet; }

  private:
    VkResult m_lastResult = VK_SUCCESS;
    VkDevice m_device = VK_NULL_HANDLE;

    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;

    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

    VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

    // An array of these is assembled from the init function call
    std::vector<VkDescriptorSetLayoutBinding> m_layoutBindings;
};
