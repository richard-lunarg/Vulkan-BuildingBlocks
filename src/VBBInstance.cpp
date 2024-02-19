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

#include "VBBInstance.h"
#include <string.h>
#include <iostream>

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    switch(messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            std::cout << "Message severity: Verbose mode." << std::endl;
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            std::cout << "Message severity: Informational." << std::endl;
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            std::cout << "Message severity: Warning." << std::endl;
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            std::cout << "Message severity: Error." << std::endl;
            break;
            
        default:
            std::cout << "Message severity: Unknown." << std::endl;
    }
    std::cout << "validation layer: " << pCallbackData->pMessage << std::endl << std::endl;

    return VK_FALSE;
}


// ********************************************************************************************
/*  The constructor does not actually make an instance.
    Here we get a list of instance extensions, and a list of available layers, and see if the
    portability enumeration extension is present.
    To make sure the constructor succeeds, call getLastError() and check for VK_SUCCESS, or
    a vulkan error code that likely will tell you that there is no ICD or loader found.
*/
VBBInstance::VBBInstance() {
    uint32_t count = 0;

    // *******************************************************
    // Get the instance version
    m_lastResult = vkEnumerateInstanceVersion(&m_instanceVersion);
    if (m_lastResult != VK_SUCCESS) return;

    // *******************************************************
    // Get a list of Vulkan Instance Extensions supported
    m_lastResult = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    if (m_lastResult != VK_SUCCESS) return;

    // *******************************************************
    // Get the actual list of instance extension
    m_availableExtensions.resize(count);
    m_lastResult = vkEnumerateInstanceExtensionProperties(nullptr, &count, m_availableExtensions.data());
    if (m_lastResult != VK_SUCCESS) return;

    // *******************************************************
    // Get a list of Vulkan Layers available
    m_lastResult = vkEnumerateInstanceLayerProperties(&count, nullptr);
    if (m_lastResult != VK_SUCCESS) return;

    // *******************************************************
    m_availableLayers.resize(count);
    m_lastResult = vkEnumerateInstanceLayerProperties(&count, m_availableLayers.data());
    if (m_lastResult != VK_SUCCESS) return;

    // ********************************************************
    // Set flag in advance if the portability enumeration extension is present
    m_isPortability = isExtensionAvailable(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
}

// *********************************************************************
// Simply clean up the instance handle
VBBInstance::~VBBInstance() {
    
    // Cleanup debugger?
    if(m_debugMessenger != VK_NULL_HANDLE) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_instanceHandle, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
            func(m_instanceHandle, m_debugMessenger, nullptr);
    }

    if (m_instanceHandle != VK_NULL_HANDLE) vkDestroyInstance(m_instanceHandle, nullptr);
}

// *********************************************************************
// Convenience function to see if the named extension is supported.
// It would be faster to loop through the entire list at once, and check
// for each extension, and that can still be done by the application by
// calling getExtensionProperties() and searching all at once.
// Using this might be more readable and easier to maintain at the
// expense of negligable few extra CPU cycles on startup.
VkBool32 VBBInstance::isExtensionAvailable(const char* extensionName) {
    for (uint32_t i = 0; i < m_availableExtensions.size(); i++)
        if (strcmp(extensionName, m_availableExtensions[i].extensionName) == 0) return VK_TRUE;

    return VK_FALSE;
}

// *********************************************************************
// Convenience function to see if the named explicit layer is present.
VkBool32 VBBInstance::isLayerAvailable(const char* layerName) {
    for (uint32_t i = 0; i < m_availableLayers.size(); i++)
        if (strcmp(layerName, m_availableLayers[i].layerName) == 0) return VK_TRUE;

    return VK_FALSE;
}

// **************************************************************************
// Create a Vulkan Instance.
// This will include all required layers and all required instance extensions
// Include portability is true by default, set to VK_FALSE if you specifically do
// not want portability implmementions listed.
// There is an Option to pass in your own VkApplicaionInfo
VkResult VBBInstance::createInstance(VkBool32 wantPortability, VkApplicationInfo* pAppInfo) {
    m_instanceHandle = VK_NULL_HANDLE;

    // Application info
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan Application";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "Vulkan Building Blocks";
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_1;  // Vulkan 1.1 is the minimum version for this library

    // Instance Info
    VkInstanceCreateInfo inst_info = {};
    inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pNext = NULL;

    if (pAppInfo != nullptr)
        inst_info.pApplicationInfo = pAppInfo;
    else
        inst_info.pApplicationInfo = &appInfo;

    // Turn on any required layers
    inst_info.enabledLayerCount = (uint32_t)m_requiredLayers.size();
    inst_info.ppEnabledLayerNames = m_requiredLayers.data();

    // If we want portability devices, and the loader supports it, we need to specify this.
    if (m_isPortability && wantPortability) {
        m_requiredExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        inst_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    } else
        m_isPortability = VK_FALSE;  // Set to false, even if portability is detected, because we specified we didn't want to use it

    // Turn on any required instance extensions
    inst_info.enabledExtensionCount = (int)m_requiredExtensions.size();
    inst_info.ppEnabledExtensionNames = m_requiredExtensions.data();
    
    // Turn on the debug callbacks
    VkDebugUtilsMessengerCreateInfoEXT  debugCreateInfo = {};
    if(isExtensionRequested(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;
        inst_info.pNext = &debugCreateInfo;
        m_debuggerOn = VK_TRUE;
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create the Instance
    m_lastResult = vkCreateInstance(&inst_info, NULL, &m_instanceHandle);
    if (m_lastResult != VK_SUCCESS) return m_lastResult;

    // Setup debugger?
    if(m_debuggerOn) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_instanceHandle, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
            func(m_instanceHandle, &debugCreateInfo, nullptr, &m_debugMessenger);
    }
    
    // Once the instance is created, this little bit of memory is
    // acutally not needed, so get rid of it.
    m_availableExtensions.clear();
    m_requiredExtensions.clear();
    m_availableLayers.clear();
    m_requiredLayers.clear();

    return VK_SUCCESS;
}
