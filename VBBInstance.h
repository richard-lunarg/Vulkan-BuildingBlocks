/*
 * Permission is hereby granted, free of charge, to any person obtaining a copy
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
 *
 * This class encapsulates the Vulkan Instance. It checks for loader extensions, and
 * layers, and creates a Vulkan instance using the desired extensions and enabling
 * any requrested explicit layers.
 */

#pragma once

#define VK_ENABLE_BETA_EXTENSIONS
#ifdef VK_NO_PROTOTYPES
#include <volk/volk.h>
#else
#include <vulkan/vulkan.h>
#endif

#include <vector>

class VBBInstance {
  public:
    VBBInstance();
    ~VBBInstance();

    VkResult createInstance(VkBool32 wantPortability = VK_TRUE, VkApplicationInfo* pAppInfo = nullptr);

    inline VkResult getLastResult(void) { return m_lastResult; }
    inline VkInstance getInstance(void) { return m_instanceHandle; }
    inline VkBool32 getIsPortabilityAvailable(void) { return m_isPortability; }

    inline void getInstanceVersion(uint32_t& major, uint32_t& minor, uint32_t& patch) {
        major = VK_API_VERSION_MAJOR(m_instanceVersion);
        minor = VK_API_VERSION_MINOR(m_instanceVersion);
        patch = VK_API_VERSION_PATCH(m_instanceVersion);
    }

    // Get the list of instance extensions
    const std::vector<VkExtensionProperties>& getExtensionProperties(void) const { return m_availableExtensions; }

    // Get the list of available layers
    const std::vector<VkLayerProperties>& getLayerProperties(void) const { return m_availableLayers; }

    // Helper functions - only valid before creating the instance.
    inline VkBool32 isExtensionAvailable(const char* extensionName);
    inline void addRequiredExtension(const char* extensionName) { m_requiredExtensions.push_back(extensionName); }

    inline VkBool32 isLayerAvailable(const char* layerName);
    inline void addRequiredLayer(const char* layerName) { m_requiredLayers.push_back(layerName); }

  protected:
    VkResult m_lastResult = VK_SUCCESS;            // Most recent Vulkan return code
    VkInstance m_instanceHandle = VK_NULL_HANDLE;  // Vulkan Instane Handle
    VkBool32 m_isPortability = VK_FALSE;           // This loader supports the portability enumeration extension
    uint32_t m_instanceVersion = 0;                // Get the Vulkan instance version

    std::vector<VkExtensionProperties> m_availableExtensions;  // List of supported instance extension
    std::vector<const char*> m_requiredExtensions;             // List of extensions we must have
    std::vector<VkLayerProperties> m_availableLayers;          // List of available layers
    std::vector<const char*> m_requiredLayers;                 // List of layers we must have
};
