#include "ModelAxes.h"


ModelAxes::ModelAxes(VmaAllocator vmaAllocator, VBBDevice* pDevice, VBBCanvas* pCanv) : ModelBase(vmaAllocator, pDevice, pCanv) {}

ModelAxes::~ModelAxes() {


}

bool ModelAxes::initModel(void) {

    pUnitAxes = new VBBUtilsUnitAxes();
    if (pUnitAxes == nullptr) return false;

    pUnitAxes->createAxes(pCanvas);
    
    return true;
}

bool ModelAxes::drawModel(VkCommandBuffer cmdBuffer, glm::mat4 proj, glm::mat4 modelView) {
    
    if (pUnitAxes == nullptr) return false;

    pUnitAxes->renderAxes(modelView, proj, cmdBuffer);

    
    return true;
}
