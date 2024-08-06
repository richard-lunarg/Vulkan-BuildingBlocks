#pragma once
/* Container class to group all the functionality of his demo together. 
	Richard S. Wright Jr.
	rwright@starstonesoftware.com
	richard@lunarg.com

*/

#include "VBBCanvas.h"

#include "ModelSun.h"
#include "ModelEarth.h"
#include "ModelMoon.h"
#include "ModelAxes.h"
#include "ModelEarthOrbit.h"
#include "ModelPlane.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>


class Orrery {
  public:
    Orrery(void);
    ~Orrery(void);

	bool initOrrery(VmaAllocator vmaAllocator, VBBDevice* pDevice, VBBCanvas* pCanv);
    bool renderOrrery(VkCommandBuffer cmdBuffer, float timeStep);

protected:
    VmaAllocator allocator;
    VBBDevice* pLogicalDevice = nullptr;
    VBBCanvas* pCanvas = nullptr;

    ModelSun*   pSun = nullptr;
    ModelEarth* pEarth = nullptr;
    ModelMoon* pMoon = nullptr;
    ModelAxes* pAxes = nullptr;
    ModelEarthOrbit* pEarthOrbit = nullptr;
    ModelPlane* pPlane = nullptr;

    glm::mat4 proj;

    void renderSolarSystem(VkCommandBuffer cmdBuffer, float timeStep, bool bMirror = false);


};
