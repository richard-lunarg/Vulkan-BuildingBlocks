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
    This is the base pipeline class. It contains common handles, data structures, and methods that will be common
    to all rendering pipelines. This is meant to be a  virutal base class, and child pipelines must
    implement or override all other functionality. It is functional by itself, but will just display
    a blank screen.

    The idea here is to make it easy to derive a custom pipeline, but also, and perhaps more importatnly, to
    have an inventory of STOCK PIPELINES for common rendering tasks.

    Pipelines need to export what vertex data and uniforms are supposed to look like.

*/

#pragma once

#include "VBBShaderModule.h"

class VBBPipelineCompute {
  public:
    VBBPipelineCompute();
    virtual ~VBBPipelineCompute();

    // *******************************************************************************
    // To be done... we can archive pipelines to disk
    // Set and Get pipeline from raw binary data (allow multiple pipelines in a single file or archive)
    // Load and save pipeline from/to a file
    // OR NOT... why wrap the Vulkan functions?

    virtual VkResult createPipeline(VkDevice logicalDevice, VkShaderModule hShaderModule);
    VkPipeline getPipeline(void) { return m_computePipeline; }
    VkPipelineLayout getPipelineLayout(void) { return m_pipelineLayout; }
    VkResult getLastResult(void) { return m_lastResult; }

    void setDescriptorSetLayouts(uint32_t count, VkDescriptorSetLayout* pLayouts) {
        m_descriptorLayoutCount = count;
        m_pDescriptorLayouts = pLayouts;
    }

    void setPushConstants(uint32_t count, VkPushConstantRange* pRange) {
        m_pushConstantCount = count;
        m_pPushConstants = pRange;
    }

  protected:
    VkResult m_lastResult;
    VkDevice m_device = VK_NULL_HANDLE;

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_computePipeline = VK_NULL_HANDLE;

    VkPushConstantRange* m_pPushConstants = nullptr;
    uint32_t m_pushConstantCount = 0;

    VkDescriptorSetLayout* m_pDescriptorLayouts = nullptr;
    uint32_t m_descriptorLayoutCount = 0;


  protected:
    // ********************************************************************************
    // Pipeline options, all of these must be set before pipeline creation
};
