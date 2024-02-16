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

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>

#ifdef VK_NO_PROTOTYPES
#include <volk/volk.h>
#else
#include <vulkan/vulkan.h>
#endif

#ifdef VBB_USE_SHADER_TOOLCHAIN
#include <shaderc/shaderc.hpp>
#include <glslang/Include/glslang_c_interface.h>
#endif


class VBBShaderModule {
  public:
    VBBShaderModule();
    ~VBBShaderModule();

    VkResult loadSPIRVFile(const VkDevice device, const char *szFullPath);
    VkResult loadSPIRVSrc(const VkDevice device, void *szShaderSrc, uint32_t sizeBytes);

#ifdef VBB_USE_SHADER_TOOLCHAIN
    VkResult loadGLSLANGFile(const VkDevice device, const char *szFullPath, shaderc_shader_kind kind);
#endif

    VkShaderModule getShaderModule(void) { return m_shaderModule; }

  protected:
    VkDevice m_device;
    VkShaderModule m_shaderModule;
};
