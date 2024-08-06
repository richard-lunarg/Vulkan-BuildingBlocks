#pragma once
/* All models are derived from this base class. They basically init
and draw themselves.
*/

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "vma/vk_mem_alloc.h"

#include "VBBDevice.h"
#include "VBBPipelineGraphics.h"
#include "VBBBufferDynamic.h"
#include "VBBBufferStatic.h"
#include "VBBDescriptors.h"
#include "VBBCanvas.h"
#include "VBBUtils.h"


class ModelBase {
  public:
    ModelBase(VmaAllocator vmaAllocator, VBBDevice* pDevice, VBBCanvas* pCanv);
    virtual ~ModelBase(void);

    virtual bool initModel(void) = 0;  // Init/load the model
    virtual bool drawModel(VkCommandBuffer cmdBuffer, glm::mat4 proj, glm::mat4 mv) = 0;  // Pass in modelview matrix, projection, etc...


 protected:
    // **************************** Passed these in *********************
    VmaAllocator        Allocator;
    VBBDevice*          pLogicalDevice = nullptr;
    VBBCanvas*          pCanvas = nullptr;


    // **************************** Build these **************************
    VBBPipelineGraphics* pPipeline = nullptr;
};

