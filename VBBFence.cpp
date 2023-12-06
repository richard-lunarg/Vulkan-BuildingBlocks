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

#include "VBBFence.h"

// ********************************************************************
// Destroy the fence
VBBFence::~VBBFence(void) {
    if (m_waitFence != VK_NULL_HANDLE) vkDestroyFence(m_device, m_waitFence, nullptr);
}

// ********************************************************************
// Default creation flag is 0, set to VK_FENCE_CREATE_SIGNALED_BIT
// If you want the fence created in a signaled state already
VkResult VBBFence::createFence(VkDevice device, VkFenceCreateFlags flag) {
    m_device = device;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = flag;

    return vkCreateFence(m_device, &fenceInfo, nullptr, &m_waitFence);
}

void VBBFence::destroyFence(void) {
    vkDestroyFence(m_device, m_waitFence, nullptr);
    m_waitFence = VK_NULL_HANDLE;
}

// *********************************************************************
// Reset the fence to unsignaled
VkResult VBBFence::reset(void) { return vkResetFences(m_device, 1, &m_waitFence); }

// *********************************************************************
// Wait or timeout
VkResult VBBFence::wait(uint64_t timeoutValue) { return vkWaitForFences(m_device, 1, &m_waitFence, VK_TRUE, timeoutValue); }
