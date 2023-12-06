//
//
//  Created by LunarG on 7/20/23.
//
#include <iostream>
#define VMA_IMPLEMENTATION          // This only goes in one source file
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vma/vk_mem_alloc.h"

#define VOLK_IMPLEMENTATION
#include <volk/volk.h>

#include <SDL2/SDL.h>
#include "SDL2/SDL_vulkan.h"
#include "VBBInstance.h"
#include "VBBPhysicalDevices.h"
#include "VBBCanvas.h"
#include "VBBPipelineGraphics.h"
#include "VBBBufferDynamic.h"
#include "VBBBufferStatic.h"
#include "VBBUtils.h"
#include "VBBDescriptors.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

VBBPipelineGraphics*    pPipeline = nullptr;
VBBBufferDynamic*       pVertexBuffer = nullptr;
VBBBufferDynamic*       pNormalBuffer = nullptr;
VBBBufferDynamic*       pTexCoordBuffer = nullptr;
VBBBufferStatic*        pIndexBuffer = nullptr;
VBBDescriptors*         pDescriptors = nullptr;


//*************************************************************************************
void printInstanceExtensions(VBBInstance& vulkanInstance)
{
    const std::vector<VkLayerProperties>& layers = vulkanInstance.getLayerProperties();
    
    // List layers found
    for(uint32_t i = 0; i < layers.size(); i++)
        std::cout << "Vulkan layer found: " << layers[i].layerName << std::endl;
}

// *************************************************************************************
void printFoundLayers(VBBInstance& vulkanInstance)
{
    const std::vector<VkExtensionProperties>& extensions = vulkanInstance.getExtensionProperties();
    
    // List instance extensions found
    for(uint32_t i = 0; i < extensions.size(); i++)
        std::cout << "Vulkan instance extension found: " << extensions[i].extensionName << std::endl;
}

// *************************************************************************************
void printInstanceVersion(VBBInstance& vulkanInstance)
{
    uint32_t major, minor, patch;
    vulkanInstance.getInstanceVersion(major, minor, patch);
    std::cout << "Instance version: " << major << "." << minor << "." << patch << std::endl;
    
    if(vulkanInstance.createInstance(VK_TRUE))
        std::cout << "Created Vulkan instance." << std::endl;
    else
        std::cout << "Error code " << vulkanInstance.getLastResult() << " trying to create Vulkan instance." << std::endl;
}

// *************************************************************************************
void listDevices(VBBPhysicalDevices& vulkanDevices)
{
    const std::vector<VkPhysicalDeviceProperties2> &deviceProperties = vulkanDevices.getDeviceProperties();
    for(int32_t i = 0; i < deviceProperties.size(); i++) {
        std::cout << "Device " << i << ": " << deviceProperties[i].properties.deviceName << std::endl;
        std::cout << std::endl;

        uint32_t major, minor, patch;
        vulkanDevices.getAPIVersion(i, major, minor, patch);
        std::cout << "API Version: " << major << "." << minor << "." << patch << std::endl;
        std::cout << std::endl;

        vulkanDevices.getDriverVersion(i, major, minor, patch);
        std::cout << "Driver Version: " << major << "." << minor << "." << patch << std::endl;
        std::cout << std::endl;

        std::cout << "Device Extensions:" << std::endl;
        
        for(int j = 0; j < vulkanDevices.getDeviceExtensionProperties(i).size(); j++)
            std::cout << vulkanDevices.getDeviceExtensionProperties(i)[j].extensionName << std::endl;

        std::cout << std::endl;
    }
}



int main(int argc, char *argv[]) {
    
    VkResult result = volkInitialize();

    VBBInstance          vulkanInstance;

    // Create the vulkan instance and display what's going on
    if(vulkanInstance.getLastResult() != VK_SUCCESS) {
        std::cout << "Error querying Vulkan instance properties." << std::endl;
        return -1;
    }
    

    printInstanceExtensions(vulkanInstance);
    std::cout << std::endl;
    
    printFoundLayers(vulkanInstance);
    std::cout << std::endl;

    // We need this extension whenever we are planning to do any rendering
    vulkanInstance.addRequiredExtension(VK_KHR_SURFACE_EXTENSION_NAME);
        
    // We need this for Apple desktop and mobile devices
    vulkanInstance.addRequiredExtension("VK_EXT_metal_surface");
        
    // We need this for our layer demo
    //vulkanInstance.addRequiredLayer("VK_LAYER_LUNARG_api_dump");
    //vulkanInstance.addRequiredLayer("VK_LAYER_KHRONOS_validation");
    
    // Create the actual instance
    if(VK_SUCCESS != vulkanInstance.createInstance(VK_TRUE)) {
        std::cout << "Error creating Vulkan Instance. Error " << vulkanInstance.getLastResult() << std::endl;
        return -1;
    }
    
    volkLoadInstance(vulkanInstance.getInstance());

    // Create the Vulkan devices object. This is a list of all physical devices and their properties
    VBBPhysicalDevices vulkanDevices(vulkanInstance.getInstance());
    
    if(vulkanDevices.getDeviceCount() == 0) {
        std::cout << "No devices found that match instance criteria." << std::endl;
        return -1;
    }
    
    // List all devices found
    std::cout << std::endl;
    listDevices(vulkanDevices);
    
    return 0;
}
