#include "ModelBase.h"


ModelBase::ModelBase(VmaAllocator vmaAllocator, VBBDevice* pDevice, VBBCanvas* pCanv) { 
	Allocator = vmaAllocator;
    pLogicalDevice = pDevice;
    pCanvas = pCanv;


}
ModelBase::~ModelBase(void)
{ 
	
	delete pPipeline;



}


