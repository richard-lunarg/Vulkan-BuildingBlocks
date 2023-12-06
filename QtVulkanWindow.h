/*
 *  Richard S. Wright Jr.
 *  richard@lunarg.com
 *
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
    ~QtVulkanWindow();

    // After this is called, Vulkan is ready for rendering. Any additional initialization should be done after this
    // Call these functions before creating the canvas to set desired samples, call the get functions after
    // to see how many samples were actually used
    void                        setMSAA(VkSampleCountFlagBits samples) { msaaSamples = samples; }
    VkSampleCountFlagBits       getMSAASamples(void) { return msaaSamples; }

    void                        setDepthStencil(VkBool32 want) { wantDepthStencil = want; }
    void                        setFramesInFlight(uint32_t fif) { framesInFlight = fif; }
    void                        setFlipViewport(VkBool32 flip) { flipViewport = flip; }

    inline VkResult getLastResult(void) { return lastResult; }

    virtual VkResult createCanvas(VBBDevice *pLogicalDevice);
    virtual void    renderNow(VkCommandBuffer cmdBuffer) { (void)cmdBuffer; }


protected:
    Ui::QtVulkanWindow *ui;

    VmaAllocator    m_vma = nullptr;
    VBBCanvas       *pVulkanCanvas = nullptr;
    VBBDevice       *pDevice = nullptr;
    VkResult        lastResult = VK_SUCCESS;
    VkSurfaceKHR    vulkanSurface = VK_NULL_HANDLE;
    VkInstance      vulkanInstance = VK_NULL_HANDLE;


    // User set's these before creating the canvas
    VkSampleCountFlagBits       msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    VkBool32                    wantDepthStencil = VK_FALSE;
    uint32_t                    framesInFlight = 2;
    VkBool32                    flipViewport = VK_FALSE;

    // Overridden functions
    virtual void	resizeEvent(QResizeEvent *ev) override;     // Derived classes should call the base version of this first
    virtual void    closeEvent(QCloseEvent *event) override;    // Derived classes should call the base version of this last

    virtual bool event(QEvent *e) override;


};
#endif // QTVULKANWINDOW_H
