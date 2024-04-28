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
 * Copyright © 2023-2024 Richard S. Wright Jr. (richard@lunarg.com)
 *
 * This software is part of the Vulkan Building Blocks
 */

#pragma once

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vma/vk_mem_alloc.h"

#ifdef VK_NO_PROTOTYPES
#include <volk/volk.h>
#else
#include <vulkan/vulkan.h>
#endif


#include "VBBDevice.h"
#include "VBBFence.h"

#include <array>

class VBBCanvas {
  public:
    VBBCanvas(VBBDevice* pVulkanDevice, VmaAllocator allocator);
    ~VBBCanvas(void);

    // These functions set the parameters of the swapchain and framebuffer's desired
    // The defaults are also already set, so if the defaults are okay, none of these
    // actually needs to be called.
    void setColorFormat(VkFormat format = VK_FORMAT_B8G8R8A8_UNORM) { m_colorFormat = format; }
    void setColorSpace(VkColorSpaceKHR cspace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) { m_colorSpace = cspace; }

    //  In case this is called after the msaa is set, make sure and reset if the depth/stencil cannot accomodate the desired value
    //  And yes, if you are going to do depth, you might as well have stencil too. That's a design choice... may or may not hold up
    //  with time.
    void setWantDepthStencil(VkBool32 depthFlag = VK_FALSE) {
        m_wantDepthStencil = depthFlag;
        m_msaaSamples = setMSAA(m_msaaSamples);
    }

    // Returns the value actually supported if set or if a lower value was used.
    VkSampleCountFlagBits setMSAA(VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);

    void setFramesInFlight(uint32_t fif = 2) { m_framesInFlight = fif; }
    void setClearColor(VkClearValue value) { m_clearColorValue = value; }
    void setClearDepthStencilValues(VkClearValue value) { m_clearDepthStencilValue = value; }
    void setBlocking(VkBool32 blocking) { m_wantBlocking = blocking; }
    void setViewportFlip(VkBool32 flip) { m_flipViewport = flip; }
    void setPresentMode(VkPresentModeKHR mode) { m_presentMode = mode; }

    inline VkResult getLastResult(void) { return m_lastResult; }

    // Attempt to create a canvas of the specified formats
    VkResult createCanvas(VkSurfaceKHR surface, uint32_t initialWidth, uint32_t initialHeight);
    VkResult resizeCanvas(uint32_t width, uint32_t height);

    VkCommandBuffer startRendering(void);
    VkResult doneRendering(void);

    VmaAllocator getAllocator(void) { return m_vma; }
    VBBDevice* getVBBDevice(void) { return m_pDevice; }
    VkRenderPass getRenderPass(void) { return m_renderPass; }
    VkDevice getLogicalDevice(void) { return m_device; }
    VBBDevice* getDevice(void) { return m_pDevice; }
    VkSampleCountFlagBits getMSAA(void) { return m_msaaSamples; }
    VkBool32 getDepthStencil(void) { return m_wantDepthStencil; }
    VmaAllocator getVMA(void) { return m_vma; }
    VkQueue getQueue(void) { return m_pDevice->getQueue(); }

  protected:
    VkResult createDepthStencil(void);
    VkResult createRenderPass(void);
    VkResult createFramebuffers(void);

    VmaAllocator m_vma = nullptr;
    VBBDevice* m_pDevice = nullptr;
    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

    VkResult m_lastResult = VK_SUCCESS;

    VkExtent2D m_screenExtent2D;

    VkFormat m_colorFormat = VK_FORMAT_B8G8R8A8_UNORM;                   // Widely available, almost deFacto
    VkColorSpaceKHR m_colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;  // Widely available, almost deFacto
    VkFormat m_swapChainImageFormat = VK_FORMAT_UNDEFINED;
    VkSwapchainKHR m_swapChain = nullptr;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkSurfaceFormatKHR m_surfaceFormatToUse = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

    uint32_t m_imageIndex = 0;
    uint32_t m_currentFrame = 0;
    std::vector<VkImage> m_swapchainImages;
    std::vector<VkImageView> m_swapchainImageViews;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VBBFence> m_inFlightFences;

    VkFormat m_depthStencilFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    VkImage m_depthStencilImage = VK_NULL_HANDLE;
    VkImageView m_depthStencilImageView = VK_NULL_HANDLE;
    VmaAllocation m_depthStencilAllocation;

    VkImage m_msaaColorImage = VK_NULL_HANDLE;
    VkImageView m_msaaColorImageView = VK_NULL_HANDLE;
    VmaAllocation m_msaaColorAllocation;

    std::vector<VkCommandBuffer> m_commandBuffers;

    // Sane default values, uses can call setters on before creating the canvas
    VkBool32 m_flipViewport = VK_FALSE;
    VkBool32 m_wantBlocking = VK_TRUE;
    VkBool32 m_wantDepthStencil = VK_FALSE;
    VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    uint32_t m_framesInFlight = 2;
    VkClearValue m_clearColorValue = {{{0.0f, 0.0f, 0.5f, 1.0f}}};
    VkClearValue m_clearDepthStencilValue = {{{1.0f, 0}}};
    VkPresentModeKHR m_presentMode = VK_PRESENT_MODE_FIFO_KHR;

    // We need to be flexible here. The surface can be created elsewhere,
    // or this class can create the surface.
    VkSurfaceKHR m_surfaceHandle = VK_NULL_HANDLE;
};
