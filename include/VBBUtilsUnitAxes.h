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

#ifdef VK_NO_PROTOTYPES
#include <volk/volk.h>
#else
#include <vulkan/vulkan.h>
#endif

#include "VBBBufferStatic.h"
#include "VBBPipelineGraphics.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>


// Uniform (push constant actually)
struct pushConstantDef {
    float mvp[16];
    float packMatrix[16]; // 3x3 normal matrix, with color in last column
};


class VBBUtilsUnitAxes {
  public:
    VBBUtilsUnitAxes(void) { }

    ~VBBUtilsUnitAxes(void);   // Destructor destroys buffers

    VkResult createAxes(VBBCanvas* pCanvas);
    VkResult renderAxes(glm::mat4& modelView, glm::mat4& proj, VkCommandBuffer cmdBuffer);


    VBBPipelineGraphics    *pPipeline = nullptr;

  protected:
    void packagePushConstants(pushConstantDef& pc, glm::mat4& modelView,  glm::mat4& mvp);
    void drawCylinder(VkCommandBuffer cmdBuffer);
    void drawSphere(VkCommandBuffer cmdBuffer);
    void drawDisk(VkCommandBuffer cmdBuffer);
    void drawCone(VkCommandBuffer cmdBuffer);


    VkResult            lastResult = VK_SUCCESS;
    VBBCanvas*          m_pCanvas = nullptr;

    float               sphereSize = 0.07f;
    uint32_t            indexCountSphere = 0;
    uint32_t            attribCountSphere = 0;
    VBBBufferStatic     *pVertexBufferSphere = nullptr;
    VBBBufferStatic     *pNormalBufferSphere = nullptr;
    VBBBufferStatic     *pIndexBufferSphere = nullptr;

    float               cylinderRadius = 0.04f;
    float               cylinderLength = 0.9f;
    uint32_t            indexCountCylinder = 0;
    uint32_t            attribCountCylinder = 0;
    VBBBufferStatic     *pVertexBufferCylinder = nullptr;
    VBBBufferStatic     *pNormalBufferCylinder = nullptr;
    VBBBufferStatic     *pIndexBufferCylinder = nullptr;

    float               diskRadius = 0.06f;
    uint32_t            indexCountDisk = 0;
    uint32_t            attribCountDisk = 0;
    VBBBufferStatic     *pVertexBufferDisk = nullptr;
    VBBBufferStatic     *pNormalBufferDisk = nullptr;
    VBBBufferStatic     *pIndexBufferDisk = nullptr;

    float               coneBottomRadius = 0.07f;
    float               coneHeight = 0.1f;
    uint32_t            indexCountCone = 0;
    uint32_t            attribCountCone = 0;
    VBBBufferStatic     *pVertexBufferCone = nullptr;
    VBBBufferStatic     *pNormalBufferCone = nullptr;
    VBBBufferStatic     *pIndexBufferCone = nullptr;



};
