//
//  Created by Richard S. Wright Jr.
//  richard@lunarg.com
//
#ifdef VK_NO_PROTOTYPES
#define VOLK_IMPLEMENTATION
#include <volk/volk.h>
#endif

#define VMA_IMPLEMENTATION  // This only goes in one source file
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vma/vk_mem_alloc.h"

#include <iostream>
#include <filesystem>

#ifdef __APPLE__
#include <unistd.h>
#endif

#include <SDL2/SDL.h>
#include "SDL2/SDL_vulkan.h"
#include "VBBInstance.h"
#include "VBBPhysicalDevices.h"
#include "VBBCanvas.h"

#include "Orrery.h"


static int screen_w, screen_h;

Orrery* pOrrery = nullptr;

/////////////////////////////////////////////////////////////////////////////////
// No-op on anything other than the Mac, sets the working directory to
// the /Resources folder
void gltSetWorkingDirectory(const char *szArgv)
{
    (void)szArgv;
#ifdef __APPLE__
    static char szParentDirectory[255];
    
    ///////////////////////////////////////////////////////////////////////////
    // Get the directory where the .exe resides
    char *c;
    strncpy( szParentDirectory, szArgv, sizeof(szParentDirectory) );
    szParentDirectory[254] = '\0'; // Make sure we are NULL terminated
    
    c = (char*) szParentDirectory;
    
    while (*c != '\0')     // go to end
        c++;
    
    while (*c != '/')      // back up to parent
        c--;
    
    *c++ = '\0';           // cut off last part (binary name)
    
    ///////////////////////////////////////////////////////////////////////////
    // Change to Resources directory. Any data files need to be placed there
    chdir(szParentDirectory);
    chdir("../Resources");
#endif
}

// *************************************************************************************
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
        
        uint32_t major, minor, patch;
        vulkanDevices.getAPIVersion(i, major, minor, patch);
        std::cout << "API Version: " << major << "." << minor << "." << patch << std::endl;
        
        vulkanDevices.getDriverVersion(i, major, minor, patch);
        std::cout << "Driver Version: " << major << "." << minor << "." << patch << std::endl;
        
        std::cout << "Device Extensions:" << std::endl;
        
        for(int j = 0; j < vulkanDevices.getDeviceExtensionProperties(i).size(); j++)
            std::cout << vulkanDevices.getDeviceExtensionProperties(i)[j].extensionName << std::endl;
    }
}



int main(int argc, char *argv[]) {
    
    gltSetWorkingDirectory(argv[0]);
    
    #ifdef VK_NO_PROTOTYPES
    VkResult result = volkInitialize();
    printf("Result = %d\n", result);
    #endif

    VBBInstance          vulkanInstance;

    // Create the vulkan instance and display what's going on
    if(vulkanInstance.getLastResult() != VK_SUCCESS) {
        std::cout << "Error querying Vulkan instance properties." << std::endl;
        return -1;
    }
    

    printInstanceExtensions(vulkanInstance);
    printFoundLayers(vulkanInstance);
        
    // We need this extension whenever we are planning to do any rendering
    vulkanInstance.addRequiredExtension(VK_KHR_SURFACE_EXTENSION_NAME);
    
#ifdef __APPLE__
    // We need this for Apple desktop and mobile devices
    vulkanInstance.addRequiredExtension("VK_EXT_metal_surface");
#endif
    
#ifdef WIN32
    vulkanInstance.addRequiredExtension("VK_KHR_win32_surface");
#endif
        
    // We need this for our layer demo
    //vulkanInstance.addRequiredLayer("VK_LAYER_LUNARG_api_dump");
    //vulkanInstance.addRequiredLayer("VK_LAYER_KHRONOS_validation");
    
    // Create the actual instance
    if(VK_SUCCESS != vulkanInstance.createInstance(VK_TRUE)) {
        std::cout << "Error creating Vulkan Instance. Error " << vulkanInstance.getLastResult() << std::endl;
        return -1;
    }
    
    #ifdef VK_NO_PROTOTYPES
    volkLoadInstance(vulkanInstance.getInstance());
    #endif

    // Create the Vulkan devices object. This is a list of all physical devices and their properties
    VBBPhysicalDevices vulkanDevices(vulkanInstance.getInstance());
    
    if(vulkanDevices.getDeviceCount() == 0) {
        std::cout << "No devices found that match instance criteria." << std::endl;
        return -1;
    }
    
    // List all devices found
    listDevices(vulkanDevices);
    
    // Create empty instance of a logical device and configure it
    VBBDevice logicalDevice(vulkanInstance.getInstance());
    #ifdef __APPLE__
    logicalDevice.addRequiredDeviceExtension("VK_KHR_portability_subset");  // Must have on macOS/iOS
    #endif
    logicalDevice.addRequiredDeviceExtension("VK_KHR_swapchain");           // Must always have for drawing
  

    // Try to create the logical device
    // Last parameter forces hardware choice
    // 0 - Qualcom native Vulkan
    // 1 - Microsoft Vulkan on DX
    // 2 - Microsoft Vulkan on DX/Software Reference
    if(VK_SUCCESS != vulkanDevices.createLogicalDevice(&logicalDevice, 1, 0)) {
        std::cout << "Could not create logical device." << std::endl;
        return -1;
        }
    
    
    VkSurfaceKHR        surface = 0;
    
    SDL_Window *window;         /* main window */

    /* initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return -1;
    
        
    //SDL_SetEventFilter(HandleAppEvents, NULL);

//    window = SDL_CreateWindow(NULL, 100, 100, 1024, 768,
//                              SDL_WINDOW_VULKAN | SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALLOW_HIGHDPI);
    
    window = SDL_CreateWindow(NULL, 100, 100, 1200, 800, SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_SetWindowTitle(window, "The Orrery");

    //SDL_SetWindowFullscreen(window ,SDL_WINDOW_FULLSCREEN_DESKTOP);// SDL_WINDOW_FULLSCREEN); // SDK_WINDOW_FULLSCREEN_DESKTOP
    
    // This comes back zero for the surface, and API dump does not show a singled Vulkan call made by SDL framework,
    // and probably because SDL want's to pick a device
    SDL_Vulkan_CreateSurface(window, vulkanInstance.getInstance(), &surface);
    if (surface == nullptr) {
        printf("SDL Could not create a Vulkan compatible Surface\n");
        return -1;
    }
    
    SDL_GetWindowSize(window, &screen_w, &screen_h);
    printf("Screen size is %d x %d\n", screen_w, screen_h);
    
    int drawableW, drawableH;
    SDL_Vulkan_GetDrawableSize(window, &drawableW, &drawableH);
    printf("Drawable size: %d x %d\n", drawableW, drawableH);
    
    
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_1;
    allocatorCreateInfo.physicalDevice = logicalDevice.getPhysicalDeviceHandle();
    allocatorCreateInfo.device = logicalDevice.getDevice();
    allocatorCreateInfo.instance = vulkanInstance.getInstance();
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
    
    VmaAllocator Allocator;
    vmaCreateAllocator(&allocatorCreateInfo, &Allocator);
    
    VBBCanvas *pVulkanCanvas = new VBBCanvas(&logicalDevice, Allocator);
    pVulkanCanvas->setViewportFlip(VK_TRUE);
    pVulkanCanvas->setWantDepthStencil(VK_TRUE);
    pVulkanCanvas->setMSAA(VK_SAMPLE_COUNT_4_BIT);
    VkClearValue clear = {0.0f, 0.0f, 0.0f, 0.0f};
    pVulkanCanvas->setClearColor(clear);


    pVulkanCanvas->setPresentMode(VK_PRESENT_MODE_MAILBOX_KHR);  
    pVulkanCanvas->setBlocking(VK_TRUE);
    pVulkanCanvas->setFramesInFlight(2); // Set this to 3 on Windows 11 ARM, get validation warning
    
    pVulkanCanvas->createCanvas(surface, drawableW, drawableH);

    pOrrery = new Orrery();
    pOrrery->initOrrery(Allocator, &logicalDevice, pVulkanCanvas);

    SDL_Event event;
    bool bDone = false;
    while (!bDone) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            bDone = true;
            }
        
        // We can't render in the background, so just go ahead and terminate
        if(event.type == SDL_APP_WILLENTERBACKGROUND || event.type == SDL_APP_TERMINATING)
            bDone = true;
        
        VkCommandBuffer cmdBuffer = pVulkanCanvas->startRendering();

        pOrrery->renderOrrery(cmdBuffer, 1.0f / 100.0);
        
        pVulkanCanvas->doneRendering();

        //SDL_Delay(1);
        }

    vkQueueWaitIdle(logicalDevice.getQueue());
    delete pOrrery;
    delete pVulkanCanvas;

    vkDestroySurfaceKHR(vulkanInstance.getInstance(), surface, nullptr);

    SDL_Quit();
    
    // Why do I have to do this? On iOS, the process does not actually terminate, it just "hides",
    // but the next time you try and "run" it, it comes up empty because it's still "running", but past
    // the above loop.
    exit(0);
    return 0;
}
