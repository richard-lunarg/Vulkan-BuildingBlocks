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

#ifndef VBBCANVAS_H
#define VBBCANVAS_H
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vma/vk_mem_alloc.h"
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
    void setColorFormat(VkFormat format = VK_FORMAT_B8G8R8A8_UNORM) { colorFormat = format; }
    void setColorSpace(VkColorSpaceKHR cspace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) { colorSpace = cspace; }

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
    void setClearDepthStencil(VkClearValue value) { m_clearDepthStencilValue = value; }
    void setBlocking(VkBool32 blocking) { m_wantBlocking = blocking; }
    void setViewportFlip(VkBool32 flip) { m_flipViewport = flip; }
    void setPresentMode(VkPresentModeKHR mode) { m_presentMode = mode; }

    inline VkResult getLastResult(void) { return m_lastResult; }

    // Attempt to create a canvas of the specified formats
    VkResult createCanvas(VkSurfaceKHR surface, uint32_t initialWidth, uint32_t initialHeight);
    VkResult resizeCanvas(uint32_t width, uint32_t height);

    VkCommandBuffer startRendering(void);
    VkResult doneRendering(void);

    VkRenderPass getRenderPass(void) { return m_renderPass; }
    VkDevice getLogicalDevice(void) { return m_device; }
    VkSampleCountFlagBits getMSAA(void) { return m_msaaSamples; }
    VkBool32 getDepthStencil(void) { return m_wantDepthStencil; }

  protected:
    VkResult createDepthStencil(void);
    VkResult createRenderPass(void);
    VkResult createFramebuffers(void);

    VmaAllocator m_vma = nullptr;
    VBBDevice* pDevice = nullptr;
    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

    VkResult m_lastResult = VK_SUCCESS;

    VkExtent2D screenExtent2D;

    VkFormat colorFormat = VK_FORMAT_B8G8R8A8_UNORM;                 // Widely available, almost deFacto
    VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;  // Widely available, almost deFacto
    VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
    VkSwapchainKHR swapChain = nullptr;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkSurfaceFormatKHR surfaceFormatToUse = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

    uint32_t imageIndex = 0;
    uint32_t currentFrame = 0;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VBBFence> inFlightFences;

    VkFormat depthStencilFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
    VkImage depthStencilImage = VK_NULL_HANDLE;
    VkImageView depthStencilImageView = VK_NULL_HANDLE;
    VmaAllocation depthStencilAllocation;

    VkImage msaaColorImage = VK_NULL_HANDLE;
    VkImageView msaaColorImageView = VK_NULL_HANDLE;
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

#endif  // VBBCANVAS_H
