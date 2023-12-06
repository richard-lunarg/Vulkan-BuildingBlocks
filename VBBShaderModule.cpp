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

#include "VBBShaderModule.h"
#include <memory.h>

#define SPIRV_MAGIC 0x07230203


VBBShaderModule::VBBShaderModule() { m_shaderModule = VK_NULL_HANDLE; }

VBBShaderModule::~VBBShaderModule(void) {
    if (m_shaderModule != VK_NULL_HANDLE) vkDestroyShaderModule(m_logicalDevice, m_shaderModule, nullptr);
}

// ******************************************************************************
// Load from a file
VkResult VBBShaderModule::loadSPIRVFile(const VkDevice logical, const char *szFullPath) {
    m_logicalDevice = logical;
    
    std::ifstream file(szFullPath, std::ios::ate | std::ios::binary);
    std::vector<char> buffer;

    if (!file.is_open()) return VK_ERROR_UNKNOWN;   // For lack of a better return code

    size_t fileSize = (size_t)file.tellg();
    buffer.resize(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();


    uint32_t spirvMagic;
    memcpy(&spirvMagic, buffer.data(), 4);
    if (spirvMagic != SPIRV_MAGIC) return VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT;

    return loadSPIRVSrc(logical, buffer.data(), uint32_t(buffer.size()));
}

// *******************************************************************************
// Load from memory
VkResult VBBShaderModule::loadSPIRVSrc(const VkDevice device, void *szShaderSrc, uint32_t sizeBytes) {
    m_logicalDevice = device;

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = sizeBytes;
    createInfo.pCode = reinterpret_cast<const uint32_t *>(szShaderSrc);

    return vkCreateShaderModule(device, &createInfo, nullptr, &m_shaderModule);
}
