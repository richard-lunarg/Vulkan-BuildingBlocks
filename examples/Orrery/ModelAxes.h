#pragma once
#include "ModelBase.h"
#include "VBBUtilsUnitAxes.h"


class ModelAxes : public ModelBase {
  public:
    ModelAxes(VmaAllocator vmaAllocator, VBBDevice* pDevice, VBBCanvas* pCanv);
    ~ModelAxes(void);

    virtual bool initModel(void) override;  // Init/load the model
    virtual bool drawModel(VkCommandBuffer cmdBuffer, glm::mat4 proj,
                           glm::mat4 mv) override;  // Pass in modelview matrix, projection, etc...

  private:
    VBBUtilsUnitAxes* pUnitAxes = nullptr;

};
