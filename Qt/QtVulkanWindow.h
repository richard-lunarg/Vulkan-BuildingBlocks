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

#ifndef QTVULKANWINDOW_H
#define QTVULKANWINDOW_H

#include <QWindow>
#include <QResizeEvent>
#include <QPaintEvent>

#include <VBBCanvas.h>

QT_BEGIN_NAMESPACE
namespace Ui { class QtVulkanWindow; }
QT_END_NAMESPACE

class QtVulkanWindow : public QWindow
{
    Q_OBJECT

public:
    QtVulkanWindow(VkInstance vulkanInstance, VmaAllocator allocator, QWindow *parent = nullptr);
    virtual ~QtVulkanWindow();

    // After this is called, Vulkan is ready for rendering. Any additional initialization should be done after this
    // Call these functions before creating the canvas to set desired samples, call the get functions after
    // to see how many samples were actually used
    void                        setMSAA(VkSampleCountFlagBits samples) { m_msaaSamples = samples; }
    VkSampleCountFlagBits       getMSAASamples(void) { return m_msaaSamples; }

    void                        setDepthStencil(VkBool32 want) { m_wantDepthStencil = want; }
    void                        setFramesInFlight(uint32_t fif) { m_framesInFlight = fif; }
    void                        setFlipViewport(VkBool32 flip) { m_flipViewport = flip; }
    void                        setClearColor(VkClearValue val) { m_clearColor = val; }
    void                        setDepthStencilClearValue(VkClearValue val) { m_depthStencilClearValue = val; }

    VBBCanvas*                  getCanvas(void) { return pVulkanCanvas; }

    inline VkResult getLastResult(void) { return lastResult; }

    virtual VkResult createCanvas(VBBDevice *pLogicalDevice);
    virtual VkResult renderNow(VkCommandBuffer cmdBuffer) { (void)cmdBuffer; return VK_SUCCESS; }


protected:
    Ui::QtVulkanWindow *ui;

    VmaAllocator    m_vma = nullptr;
    VBBCanvas       *pVulkanCanvas = nullptr;
    VBBDevice       *pDevice = nullptr;
    VkResult        lastResult = VK_SUCCESS;
    VkSurfaceKHR    vulkanSurface = VK_NULL_HANDLE;
    VkInstance      vulkanInstance = VK_NULL_HANDLE;


    // User set's these before creating the canvas
    VkSampleCountFlagBits       m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    VkBool32                    m_wantDepthStencil = VK_FALSE;
    uint32_t                    m_framesInFlight = 2;
    VkBool32                    m_flipViewport = VK_FALSE;
    VkClearValue                m_clearColor = {{{0.0f, 0.0f, 0.2f, 0.0f}}};
    VkClearValue                m_depthStencilClearValue = {{{1.0f, 0}}};



    // Overridden functions
    virtual void	resizeEvent(QResizeEvent *ev) override;     // Derived classes should call the base version of this first
    virtual void    closeEvent(QCloseEvent *event) override;    // Derived classes should call the base version of this last

    virtual bool event(QEvent *e) override;


};
#endif // QTVULKANWINDOW_H
