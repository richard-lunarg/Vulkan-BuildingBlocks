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

#include "QtVulkanWindow.h"
#include <QGuiApplication>

#ifdef WIN32
#include <windows.h>
#include "vulkan/vulkan_win32.h"
#endif

#ifdef __APPLE__
extern "C" {
void *makeViewMetalCompatible(void* handle);
}
#endif

#ifdef linux
#include </home/parallels/Dev/Qt6.5.3/qtbase/include/QtGui/6.5.3/QtGui/qpa/qplatformnativeinterface.h>
//#include </home/rwright/Dev/Qt6.3.1/include/QtGui/6.3.1/QtGui/qpa/qplatformnativeinterface.h>
//#include <QtGui/qpa/qplatformnativeinterface.h>
//using namespace QNativeInterface;
#endif

QtVulkanWindow::QtVulkanWindow(VkInstance vulkanInstance, VmaAllocator allocator, QWindow *parent)
    : QWindow(parent), m_vma(allocator)
{

    this->vulkanInstance = vulkanInstance;

#ifdef __APPLE__
    setSurfaceType(QSurface::MetalSurface);
#endif

}

QtVulkanWindow::~QtVulkanWindow()
{
    if (pVulkanCanvas) vkQueueWaitIdle(pVulkanCanvas->getQueue());
    delete pVulkanCanvas;

    if(pDevice != nullptr)
        vkDestroySurfaceKHR(pDevice->getInstance(), vulkanSurface, nullptr);

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

#ifdef linux
    VkXcbSurfaceCreateInfoKHR createInfoXCB;
    createInfoXCB.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createInfoXCB.pNext = NULL;
    createInfoXCB.flags = 0;

    QPlatformNativeInterface *nativeInterface;
    nativeInterface = QGuiApplication::platformNativeInterface();
    //QX11Application app;
    //NativeInterface *Interface = qApp->nativeInterface();
    //qApp->nativeInterface();
   // QGuiApplication::nativeInterface();
    //Interface *nativeInterface = QGuiApplication::platformIntegration()->nativeInterface();
    //QGuiApplicationPrivate::platformIntegration()->nativeInterface();
    //  mDisplay = static_cast<Display *>(nativeInterface->nativeResourceForWindow("Display",m_compositor->window()));

    createInfoXCB.connection = (xcb_connection_t*)nativeInterface->nativeResourceForWindow("connection", this);
    createInfoXCB.window = (xcb_window_t)this->winId();

    VkResult err = vkCreateXcbSurfaceKHR(vulkanInstance, &createInfoXCB, NULL, &vulkanSurface);
    printf("Surface return is %d\n", err);

#endif

    pVulkanCanvas = new VBBCanvas(pDevice, m_vma);
    if(pVulkanCanvas == nullptr)
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;   // TBD: Is this the right one really?

    if(pVulkanCanvas->getLastResult() != VK_SUCCESS)
        return pVulkanCanvas->getLastResult();

    // How many frames in flight do you want?
    pVulkanCanvas->setFramesInFlight(m_framesInFlight);

    // Depth and Stencil are intertwined
    pVulkanCanvas->setWantDepthStencil(m_wantDepthStencil);

    // If we want msaa, set it, and return the actual used value (may not be the same)
    m_msaaSamples = pVulkanCanvas->setMSAA(m_msaaSamples);

    // Y is up like OpenGL
    pVulkanCanvas->setViewportFlip(m_flipViewport);

    // Other values
    pVulkanCanvas->setClearColor(m_clearColor);
    pVulkanCanvas->setClearDepthStencilValues(m_depthStencilClearValue);


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
    if (pVulkanCanvas) vkQueueWaitIdle(pVulkanCanvas->getQueue());
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
