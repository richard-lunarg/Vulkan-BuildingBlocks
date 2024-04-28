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

#include "VBBCanvas.h"

VBBCanvas::VBBCanvas(VBBDevice* pVulkanDevice, VmaAllocator allocator) : m_vma(allocator) {
    m_pDevice = pVulkanDevice;
    m_device = pVulkanDevice->getDevice();
    m_physicalDevice = pVulkanDevice->getPhysicalDeviceHandle();
}

VBBCanvas::~VBBCanvas(void) {
    // We need to wait for the queue to be idle before we can do this stuff
    if (m_pDevice) vkQueueWaitIdle(m_pDevice->getQueue());

    if (m_commandBuffers.size() != 0 && m_pDevice != nullptr)
        m_pDevice->releaseCommandBuffers(m_commandBuffers.data(), static_cast<uint32_t>(m_commandBuffers.size()));

    if (m_swapchainImageViews.size() != 0) {
        for (auto imageView : m_swapchainImageViews) vkDestroyImageView(m_device, imageView, nullptr);

        vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
    }

    if (m_swapChainFramebuffers.size() != 0) {
        for (auto framebuffer : m_swapChainFramebuffers) vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }

    for (size_t i = 0; i < m_imageAvailableSemaphores.size(); i++) vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);

    for (size_t i = 0; i < m_renderFinishedSemaphores.size(); i++) vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);

    for (size_t i = 0; i < m_inFlightFences.size(); i++) m_inFlightFences[i].destroyFence();

    if (m_renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);
        m_renderPass = VK_NULL_HANDLE;
    }

    if (m_depthStencilImageView != VK_NULL_HANDLE) vkDestroyImageView(m_device, m_depthStencilImageView, nullptr);

    if (m_depthStencilAllocation != VK_NULL_HANDLE) vmaDestroyImage(m_vma, m_depthStencilImage, m_depthStencilAllocation);

    if (m_msaaColorImageView != VK_NULL_HANDLE) vkDestroyImageView(m_device, m_msaaColorImageView, nullptr);

    if (m_msaaColorImage != VK_NULL_HANDLE) vmaDestroyImage(m_vma, m_msaaColorImage, m_msaaColorAllocation);
}

// **************************************************************************
// Set desired number of samples. This validates against the physical device
// and returns either the requested amount of multisampling, or the next largest
// available value.
VkSampleCountFlagBits VBBCanvas::setMSAA(VkSampleCountFlagBits samples) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(m_physicalDevice, &deviceProperties);
    VkSampleCountFlags counts = deviceProperties.limits.framebufferColorSampleCounts;
    ;

    // If a depth/stencil is also requested make sure it's limits are taken into account as well (they COULD be
    // different from the color buffer.
    if (m_wantDepthStencil)
        counts &= (deviceProperties.limits.framebufferDepthSampleCounts & deviceProperties.limits.framebufferStencilSampleCounts);

    switch (samples) {
        // If they asked for one (or an invalid value), give them 1 (no multisampling)
        case VK_SAMPLE_COUNT_1_BIT:
        default:
            m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
            break;

        // Try and give them what they asked for, if not avaialbe, give them the next highest one
        case VK_SAMPLE_COUNT_64_BIT:
            if (counts & VK_SAMPLE_COUNT_64_BIT) {
                m_msaaSamples = VK_SAMPLE_COUNT_64_BIT;
                return m_msaaSamples;
            }

        case VK_SAMPLE_COUNT_32_BIT:
            if (counts & VK_SAMPLE_COUNT_32_BIT) {
                m_msaaSamples = VK_SAMPLE_COUNT_32_BIT;
                return m_msaaSamples;
            }

        case VK_SAMPLE_COUNT_16_BIT:
            if (counts & VK_SAMPLE_COUNT_16_BIT) {
                m_msaaSamples = VK_SAMPLE_COUNT_16_BIT;
                return m_msaaSamples;
            }

        case VK_SAMPLE_COUNT_8_BIT:
            if (counts & VK_SAMPLE_COUNT_8_BIT) {
                m_msaaSamples = VK_SAMPLE_COUNT_8_BIT;
                return m_msaaSamples;
            }

        case VK_SAMPLE_COUNT_4_BIT:
            if (counts & VK_SAMPLE_COUNT_4_BIT) {
                m_msaaSamples = VK_SAMPLE_COUNT_4_BIT;
                return m_msaaSamples;
            }

        case VK_SAMPLE_COUNT_2_BIT:
            if (counts & VK_SAMPLE_COUNT_2_BIT) {
                m_msaaSamples = VK_SAMPLE_COUNT_2_BIT;
                return m_msaaSamples;
            }

        // Just no msaaa
            m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
            return m_msaaSamples;
    }

    return m_msaaSamples;
}

// **********************************************************************
// A surface must be passed in, created by this class or another framework.
// This needs to return a useful error code if the desired surface
// characteristics aren't available.
VkResult VBBCanvas::createCanvas(VkSurfaceKHR surface, uint32_t initialWidth, uint32_t initialHeight) {
    m_surfaceHandle = surface;

    // Is this test really necessary - UPDATE WITH QUEUE FAMILY INDEX IF IT IS, assuming it's always zero
    VkBool32 bSupported = VK_FALSE;
    m_lastResult = vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, 0, m_surfaceHandle, &bSupported);
    if (m_lastResult != VK_SUCCESS) return m_lastResult;
    if (bSupported == VK_FALSE) return VK_ERROR_UNKNOWN;

    uint32_t formatCount;
    m_lastResult = vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
    m_lastResult = vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, surface, &formatCount, surfaceFormats.data());

    // Find the surface format we want. If it's not availabe, return an error. The defaults are widely widely available,
    // but the developer could alwasy query ahead of time to pick a different one based on what is supported.
    // We are really just making sure the selected formats are in the list
    for (uint32_t i = 0; i < formatCount; i++) {
        if (surfaceFormats[i].format == m_colorFormat && surfaceFormats[i].colorSpace == m_colorSpace) {
            m_surfaceFormatToUse = surfaceFormats[i];
            m_swapChainImageFormat = surfaceFormats[i].format;
            break;
        }
    }

    if (m_swapChainImageFormat == VK_FORMAT_UNDEFINED)  // Thanks for trying, please play again
        return VK_ERROR_FORMAT_NOT_SUPPORTED;

    // Presentation modes
    // VK_PRESENT_MODE_FIFO_KHR is required by the spec to be supported, we only need this code if we want
    // to try for something else.
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, surface, &presentModeCount, presentModes.data());

    // Pick the presentation mode we want
    // The default is FIFO (and it is required to be supported by Vulkan
    // If we can't find the one asked for, fall back to FIFO
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < presentModeCount; i++) {
        if (presentModes[i] == m_presentMode) {
            presentMode = m_presentMode;
            break;
        }
    }
    m_presentMode = presentMode;

    // Figure out what depth/stencil format we want (or can use)
    // TBD: THIS IS SUPPORTED WIDELY, IT WILL DO FOR NOW
    m_depthStencilFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    m_imageAvailableSemaphores.resize(m_framesInFlight);
    m_renderFinishedSemaphores.resize(m_framesInFlight);
    m_inFlightFences.resize(m_framesInFlight);

    for (size_t i = 0; i < m_framesInFlight; i++) {
        m_lastResult = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]);
        if (m_lastResult != VK_SUCCESS) return m_lastResult;

        m_lastResult = vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]);
        if (m_lastResult != VK_SUCCESS) return m_lastResult;

        m_lastResult = m_inFlightFences[i].createFence(m_device, VK_FENCE_CREATE_SIGNALED_BIT);
        if (m_lastResult != VK_SUCCESS) return m_lastResult;
    }

    // Get a command buffer
    m_commandBuffers.resize(m_framesInFlight);
    m_lastResult = m_pDevice->allocateCommandBuffers(m_commandBuffers.data(), m_framesInFlight);
    if (m_lastResult != VK_SUCCESS) return m_lastResult;

    createRenderPass();

    return resizeCanvas(initialWidth, initialHeight);
}

// ***************************************************************************
// Create image and view for the depth/stencil buffer
VkResult VBBCanvas::createDepthStencil(void) {
    if (m_depthStencilImageView != VK_NULL_HANDLE) vkDestroyImageView(m_device, m_depthStencilImageView, nullptr);
    if (m_depthStencilImage != VK_NULL_HANDLE) vmaDestroyImage(m_vma, m_depthStencilImage, m_depthStencilAllocation);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = m_screenExtent2D.width;
    imageInfo.extent.height = m_screenExtent2D.height;
    imageInfo.samples = m_msaaSamples;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = m_depthStencilFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;//VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    vmaCreateImage(m_vma, &imageInfo, &allocInfo, &m_depthStencilImage, &m_depthStencilAllocation, nullptr);

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_depthStencilImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_depthStencilFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    m_lastResult = vkCreateImageView(m_device, &viewInfo, nullptr, &m_depthStencilImageView);

    return m_lastResult;
}

// ***************************************************************************
// Update when the canvas changes size
VkResult VBBCanvas::resizeCanvas(uint32_t width, uint32_t height) {
    (void)width;
    (void)height;

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    m_lastResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surfaceHandle, &surfaceCapabilities);

    // We are probably closing the window and the windowing system is still calling this
    if (m_lastResult == VK_ERROR_SURFACE_LOST_KHR) return m_lastResult;

    m_screenExtent2D.width = surfaceCapabilities.currentExtent.width;
    m_screenExtent2D.height = surfaceCapabilities.currentExtent.height;

    if (m_wantDepthStencil) {
        m_lastResult = createDepthStencil();
        if (m_lastResult != VK_SUCCESS) return m_lastResult;
    }

    // If the image views already exist, free them up along with render passes and framebuffers
    if (m_swapchainImageViews.size() != 0) {
        for (auto imageView : m_swapchainImageViews) vkDestroyImageView(m_device, imageView, nullptr);

        vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
    }

    if (m_swapChainFramebuffers.size() != 0) {
        for (auto framebuffer : m_swapChainFramebuffers) vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }

    m_swapchainImageViews.resize(0);
    m_swapchainImages.resize(0);
    m_swapChainFramebuffers.resize(0);

    VkSwapchainCreateInfoKHR swapChainInfo = {};
    swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainInfo.pNext = nullptr;
    swapChainInfo.surface = m_surfaceHandle;
    swapChainInfo.minImageCount = m_framesInFlight;
    swapChainInfo.imageFormat = m_surfaceFormatToUse.format;
    swapChainInfo.imageColorSpace = m_surfaceFormatToUse.colorSpace;
    swapChainInfo.imageExtent = m_screenExtent2D;
    swapChainInfo.imageArrayLayers = 1;
    swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapChainInfo.queueFamilyIndexCount = 0;
    swapChainInfo.pQueueFamilyIndices = nullptr;
    swapChainInfo.preTransform = surfaceCapabilities.currentTransform;  // VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainInfo.presentMode = m_presentMode;
    swapChainInfo.clipped = VK_TRUE;
    swapChainInfo.oldSwapchain = nullptr;  // I believe this is a bug in MoltenVK, I should be able to put the old swapchain here.

    m_lastResult = vkCreateSwapchainKHR(m_device, &swapChainInfo, nullptr, &m_swapChain);
    if (m_lastResult != VK_SUCCESS) return m_lastResult;

    uint32_t swapChainImageCount = 0;
    m_lastResult = vkGetSwapchainImagesKHR(m_device, m_swapChain, &swapChainImageCount, nullptr);
    m_framesInFlight = swapChainImageCount;  // Seems the sanest approach in case they are different
    m_swapchainImages.resize(swapChainImageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapChain, &swapChainImageCount, m_swapchainImages.data());

    m_swapchainImageViews.resize(swapChainImageCount);
    for (uint32_t i = 0; i < swapChainImageCount; i++) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        m_lastResult = vkCreateImageView(m_device, &createInfo, nullptr, &m_swapchainImageViews[i]);
        if (m_lastResult != VK_SUCCESS) return m_lastResult;
    }

    // Do we need an MSAA color image?
    if (m_msaaSamples != VK_SAMPLE_COUNT_1_BIT) {
        if (m_msaaColorImageView != VK_NULL_HANDLE) vkDestroyImageView(m_device, m_msaaColorImageView, nullptr);

        if (m_msaaColorImage != VK_NULL_HANDLE) vmaDestroyImage(m_vma, m_msaaColorImage, m_msaaColorAllocation);

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = m_screenExtent2D.width;
        imageInfo.extent.height = m_screenExtent2D.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = m_swapChainImageFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = m_msaaSamples;
        imageInfo.flags = 0;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;//VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        vmaCreateImage(m_vma, &imageInfo, &allocInfo, &m_msaaColorImage, &m_msaaColorAllocation, nullptr);

        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_msaaColorImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_swapChainImageFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        vkCreateImageView(m_device, &viewInfo, nullptr, &m_msaaColorImageView);
    }

    createFramebuffers();

    return m_lastResult;
}

// *****************************************************************************
VkResult VBBCanvas::createRenderPass(void) {
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    if (m_wantDepthStencil == VK_TRUE) {
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapChainImageFormat;
    colorAttachment.samples = m_msaaSamples;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout =
        (m_msaaSamples == VK_SAMPLE_COUNT_1_BIT) ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = m_depthStencilFormat;
    depthAttachment.samples = m_msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthStencilAttachmentRef{};
    depthStencilAttachmentRef.attachment = 1;
    depthStencilAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = m_swapChainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;  // Yes, this has to be one
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = (m_wantDepthStencil) ? 2 : 1;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    if (m_msaaSamples != VK_SAMPLE_COUNT_1_BIT) subpass.pResolveAttachments = &colorAttachmentResolveRef;

    if (m_wantDepthStencil == VK_TRUE) subpass.pDepthStencilAttachment = &depthStencilAttachmentRef;

    std::vector<VkAttachmentDescription> attachments;
    if (m_msaaSamples == VK_SAMPLE_COUNT_1_BIT) {
        attachments.push_back(colorAttachment);

        if (m_wantDepthStencil) attachments.push_back(depthAttachment);
    } else {  // They go in a different order when msaa is used
        attachments.push_back(colorAttachment);

        if (m_wantDepthStencil) attachments.push_back(depthAttachment);

        attachments.push_back(colorAttachmentResolve);
    }

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    return vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass);
}

// ******************************************************************88
VkResult VBBCanvas::createFramebuffers(void) {
    m_swapChainFramebuffers.resize(m_swapchainImageViews.size());

    for (size_t i = 0; i < m_swapchainImageViews.size(); i++) {
        std::vector<VkImageView> attachments;

        if (m_msaaSamples == VK_SAMPLE_COUNT_1_BIT) {
            attachments.push_back(m_swapchainImageViews[i]);

            if (m_wantDepthStencil) attachments.push_back(m_depthStencilImageView);
        } else {  // Different order
            attachments.push_back(m_msaaColorImageView);

            if (m_wantDepthStencil) attachments.push_back(m_depthStencilImageView);

            attachments.push_back(m_swapchainImageViews[i]);
        }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;

        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_screenExtent2D.width;
        framebufferInfo.height = m_screenExtent2D.height;
        framebufferInfo.layers = 1;

        m_lastResult = vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]);
        if (m_lastResult != VK_SUCCESS) return m_lastResult;
    }

    return m_lastResult;
}

VkCommandBuffer VBBCanvas::startRendering(void) {
    m_inFlightFences[m_currentFrame].wait();   // Until any previous rendering is done
    m_inFlightFences[m_currentFrame].reset();  // Clear it for the next use

    // TBD: Block at the beginniing. For GUI apps, this means the finish is asynchronous an other
    // messages in the message queue can be processed between paint calls.
    // The pure move along, is causing problems with my descriptor sets. I'm trying to update one
    // while it's already being used by the previous command buffer. So do we need arrays of
    // descriptor sets? Or the indexed descriptor set extension...
    if (m_wantBlocking) vkQueueWaitIdle(m_pDevice->getQueue());

    m_lastResult =
        vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &m_imageIndex);
    if (m_lastResult == VK_ERROR_OUT_OF_DATE_KHR) {
        resizeCanvas(m_screenExtent2D.width, m_screenExtent2D.height);
        return VK_NULL_HANDLE;
    }

    // ************************************************************************
    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];
    vkResetCommandBuffer(commandBuffer, 0);

    // ************************************************************************
    // Start recording a command buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    m_lastResult = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (m_lastResult != VK_SUCCESS) return VK_NULL_HANDLE;

    // ***********************************************************************
    // Start the render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapChainFramebuffers[m_imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_screenExtent2D;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0] = m_clearColorValue;
    clearValues[1] = m_clearDepthStencilValue;

    renderPassInfo.clearValueCount = (m_wantDepthStencil) ? 2 : 1;
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // *********************************************************
    // NOTE: We are taking advantage of a Vulkan 1.1 feature
    // which allows the vieweport to have a negative hight
    // (the y origin also needs to be changed from zero for this to work)
    // This gives us a right handed coordinate system like OpenGL
    // Z comes out of screen, Y goes up
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = (m_flipViewport) ? static_cast<float>(m_screenExtent2D.height) : 0;
    viewport.width = static_cast<float>(m_screenExtent2D.width);
    viewport.height = (m_flipViewport) ? -static_cast<float>(m_screenExtent2D.height) : static_cast<float>(m_screenExtent2D.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_screenExtent2D;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    return commandBuffer;
}

// ********************************************************************************
// DONE drawing, wrap up the command buffer, wait for the queue to complete, and
// present the results.
VkResult VBBCanvas::doneRendering(void) {
    VkCommandBuffer commandBuffer = m_commandBuffers[m_currentFrame];
    vkCmdEndRenderPass(commandBuffer);

    m_lastResult = vkEndCommandBuffer(commandBuffer);
    if (m_lastResult != VK_SUCCESS) return m_lastResult;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[m_currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[m_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    m_lastResult = vkQueueSubmit(m_pDevice->getQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame].getFence());

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &m_imageIndex;
    presentInfo.pResults = nullptr;

    // Note, this is actually asynchronous...
    m_lastResult = vkQueuePresentKHR(m_pDevice->getQueue(), &presentInfo);

    m_currentFrame = (m_currentFrame + 1) % m_framesInFlight;

    if (m_lastResult == VK_ERROR_OUT_OF_DATE_KHR || m_lastResult == VK_SUBOPTIMAL_KHR) {
        vkQueueWaitIdle(pDevice->getQueue());   // Important! we can't do this with frames in flight
        resizeCanvas(m_screenExtent2D.width, m_screenExtent2D.height);
    }

    return VK_SUCCESS;
}
