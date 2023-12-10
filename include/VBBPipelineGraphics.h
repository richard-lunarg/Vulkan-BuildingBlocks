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
#include "VBBCanvas.h"

class VBBPipelineGraphics {
  public:
    VBBPipelineGraphics();
    virtual ~VBBPipelineGraphics();

    // ******************************************************************************
    // ** Configurable options common to all pipelines
    /*
     *  Front/Back face orientation
     *  Face Culling
     *  Polygon mode
     *  Stencil operations
     *  etc...
     *
     */

    void setPrimitiveTopology(VkPrimitiveTopology topo) { m_primitiveTopology = topo; }
    void setFrontFace(VkFrontFace face) { m_frontFace = face; }
    void setCullMode(VkCullModeFlags mode) { m_cullMode = mode; }
    void setPolygonMode(VkPolygonMode mode) { m_polygonMode = mode; }

    void setEnableBlend(VkBool32 flag) { m_blendingFlag = flag; }
    void setColorBlendFactors(VkBlendFactor src, VkBlendFactor dst) {
        m_srcColorBlendFactor = src;
        m_dstColorBlendFactor = dst;
    }

    void setEnableDepthTest(VkBool32 flag) { m_depthTestFlag = flag; }
    void setEnableDepthWrite(VkBool32 flag) { m_depthWriteFlag = flag; }
    void setDepthCompareOp(VkCompareOp op) { m_depthCompareOp = op; }

    void setEnableStencilTest(VkBool32 flag) { m_stencilTestFlag = flag; }
    void setStencilOpStateFront(VkStencilOpState front) { m_stencilOpStateFront = front; }
    void setStencilOpStateBack(VkStencilOpState back) { m_stencilOpStateBack = back; }

    // *******************************************************************************
    // To be done... we can archive pipelines to disk
    // Set and Get pipeline from raw binary data (allow multiple pipelines in a single file or archive)
    // Load and save pipeline from/to a file
    // OR NOT... why wrap the Vulkan functions?

    virtual VkResult createPipeline(VBBCanvas* pCanvas, VBBShaderModule* pVertShader, VBBShaderModule* pFragShader);
    VkPipeline getPipeline(void) { return graphicsPipeline; }
    VkPipelineLayout getPipelineLayout(void) { return pipelineLayout; }
    VkResult getLastResult(void) { return m_lastResult; }

    void setPushConstants(uint32_t count, VkPushConstantRange* pRange) {
        m_pushConstantCount = count;
        m_pPushConstants = pRange;
    }
    void setDescriptorSetLayouts(uint32_t count, VkDescriptorSetLayout* pLayouts) {
        m_descriptorLayoutCount = count;
        m_pDescriptorLayouts = pLayouts;
    }

    void addVertexAttributeBinding(uint32_t stride, VkVertexInputRate inputRate, uint32_t location, VkFormat format);

  protected:
    VkResult m_lastResult;

    VBBCanvas* pVulkanCanvas = nullptr;
    VkDevice device = VK_NULL_HANDLE;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;

    VkPushConstantRange* m_pPushConstants = nullptr;
    uint32_t m_pushConstantCount = 0;

    VkDescriptorSetLayout* m_pDescriptorLayouts = nullptr;
    uint32_t m_descriptorLayoutCount = 0;

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    VkPipelineMultisampleStateCreateInfo multisampling{};
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    VkVertexInputBindingDescription bindingDescription{};
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;

  protected:
    // ********************************************************************************
    // Pipeline options, all of these must be set before pipeline creation
    VkPrimitiveTopology m_primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkFrontFace m_frontFace = VK_FRONT_FACE_CLOCKWISE;
    VkCullModeFlags m_cullMode = VK_CULL_MODE_BACK_BIT;
    VkPolygonMode m_polygonMode = VK_POLYGON_MODE_FILL;

    VkBool32 m_blendingFlag = VK_FALSE;
    VkBlendFactor m_srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    VkBlendFactor m_dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

    VkBool32 m_depthTestFlag = VK_FALSE;
    VkBool32 m_depthWriteFlag = VK_FALSE;
    VkCompareOp m_depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    VkBool32 m_stencilTestFlag = VK_FALSE;
    VkStencilOpState m_stencilOpStateFront = {};
    VkStencilOpState m_stencilOpStateBack = {};
};
