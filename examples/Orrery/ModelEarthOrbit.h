#pragma once
#include "ModelBase.h"

class ModelEarthOrbit : public ModelBase {
  public:
    ModelEarthOrbit(VmaAllocator vmaAllocator, VBBDevice* pDevice, VBBCanvas* pCanv);
    ~ModelEarthOrbit(void);

    virtual bool initModel(void) override;  // Init/load the model
    virtual bool drawModel(VkCommandBuffer cmdBuffer, glm::mat4 proj,
                           glm::mat4 mv) override;  // Pass in modelview matrix, projection, etc...

  private:
    uint32_t indexCount = 0;
    uint32_t attribCount = 0;

    VBBSimpleIndexedMesh orbit;

    VBBPipelineGraphics* pPipeline = nullptr;
    VBBBufferDynamic* pVertexBuffer = nullptr;
    VBBBufferDynamic* pNormalBuffer = nullptr;
    VBBBufferDynamic* pTexCoordBuffer = nullptr;
    VBBBufferStatic* pIndexBuffer = nullptr;
    VBBDescriptors* pDescriptors = nullptr;
};
