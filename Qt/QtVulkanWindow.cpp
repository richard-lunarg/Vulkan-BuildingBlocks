/*
 *  Richard S. Wright Jr.
 *  richard@lunarg.com
 *
 */

#include "QtVulkanWindow.h"



#ifdef WIN32
#include <windows.h>
#include "vulkan/vulkan_win32.h"
#endif

#ifdef __APPLE__
extern "C" {
void *makeViewMetalCompatible(void* handle);
}
#endif


QtVulkanWindow::QtVulkanWindow(VkInstance vulkanInstance, VmaAllocator allocator, QWindow *parent)
    : QWindow(parent), m_vma(allocator)
{

    this->vulkanInstance = vulkanInstance;

#ifdef __APPLE__
    setSurfaceType(QSurface::VulkanSurface);
#endif

}

QtVulkanWindow::~QtVulkanWindow()
{
    delete pVulkanCanvas;

    if(pDevice != nullptr) {
        vkDestroySurfaceKHR(pDevice->getInstance(), vulkanSurface, nullptr);
    }
}

VkResult QtVulkanWindow::createCanvas(VBBDevice* pLogicalDevice)
{
    pDevice = pLogicalDevice;

#ifdef WIN32
    VkWin32SurfaceCreateInfoKHR createInfoWin = {};
    createInfoWin.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfoWin.hwnd = (HWND)this->winId();
    createInfoWin.hinstance = GetModuleHandle(nullptr);

    lastResult = vkCreateWin32Surface(vulkanInstance, &createInfoWin, nullptr, &vulkanSurface);
    if(lastResult != VK_SUCCESS)
        return lastResult;

#endif

#ifdef __APPLE__
    setSurfaceType(QSurface::MetalSurface);

    CAMetalLayer *layer = (void*)makeViewMetalCompatible((void*)this->winId());

    VkMetalSurfaceCreateInfoEXT metalInfo = {};
    metalInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    metalInfo.flags = 0;
    metalInfo.pNext = nullptr;
    metalInfo.pLayer = layer;

    lastResult = vkCreateMetalSurfaceEXT(vulkanInstance, &metalInfo, nullptr, &vulkanSurface);
    if(lastResult != VK_SUCCESS)
        return lastResult;

#endif

    pVulkanCanvas = new VBBCanvas(pDevice, m_vma);
    if(pVulkanCanvas == nullptr)
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;   // TBD: Is this the right one really?

    if(pVulkanCanvas->getLastResult() != VK_SUCCESS)
        return pVulkanCanvas->getLastResult();

    // How many frames in flight do you want?
    pVulkanCanvas->setFramesInFlight(framesInFlight);

    // Depth and Stencil are intertwined
    pVulkanCanvas->setWantDepthStencil(wantDepthStencil);

    // If we want msaa, set it, and return the actual used value (may not be the same)
    msaaSamples = pVulkanCanvas->setMSAA(msaaSamples);

    // Y is up like OpenGL
    pVulkanCanvas->setViewportFlip(flipViewport);

    lastResult = pVulkanCanvas->createCanvas(vulkanSurface, geometry().size().width(), geometry().size().height());

    return VK_SUCCESS;
}


void QtVulkanWindow::resizeEvent(QResizeEvent *ev)
{
    if(pVulkanCanvas == nullptr)
        return;

    uint32_t width = ev->size().width();
    uint32_t height = ev->size().height();

    pVulkanCanvas->resizeCanvas(width, height);
    requestUpdate();
}



void QtVulkanWindow::closeEvent(QCloseEvent *event)
{
    delete pVulkanCanvas;
    pVulkanCanvas = nullptr;

    if(pDevice != nullptr) {
        vkDestroySurfaceKHR(pDevice->getInstance(), vulkanSurface, nullptr);
        pDevice = nullptr;
        }

    QWindow::closeEvent(event);
}


bool QtVulkanWindow::event(QEvent *e)
{
    if(e->type() == QEvent::UpdateRequest)
    {
        if(pVulkanCanvas != nullptr) {

        VkCommandBuffer commandBuffer = pVulkanCanvas->startRendering();

        // This is the callback function that child classes should override.
        renderNow(commandBuffer);

        pVulkanCanvas->doneRendering();
        }
    }

    return QWindow::event(e);
}
