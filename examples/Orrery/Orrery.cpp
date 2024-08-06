/* Container class to group all the functionality of his demo together.
        Richard S. Wright Jr.
        rwright@starstonesoftware.com
        richard@lunarg.com

*/

#include "Orrery.h"

// ******************************************************
// Constructor doesn't do much other than store a bunch of important
// Vulkan stuff
Orrery::Orrery(void) {


}

// *******************************************************
// Cleanup stuff
Orrery::~Orrery(void) {

    delete pSun;
    delete pAxes;
    delete pEarth;
    delete pMoon;
    delete pEarthOrbit;
    delete pPlane;

}


bool Orrery::initOrrery(VmaAllocator vmaAllocator, VBBDevice* pDevice, VBBCanvas* pCanv)
{ 
    allocator = vmaAllocator;
    pLogicalDevice = pDevice;
    pCanvas = pCanv;

    pSun = new ModelSun(allocator, pLogicalDevice, pCanvas);
    pSun->initModel();

    pAxes = new ModelAxes(allocator, pLogicalDevice, pCanvas);
    pAxes->initModel();

    pEarth = new ModelEarth(allocator, pLogicalDevice, pCanvas);
    pEarth->initModel();

    pMoon = new ModelMoon(allocator, pLogicalDevice, pCanvas);
    pMoon->initModel();

    pEarthOrbit = new ModelEarthOrbit(allocator, pLogicalDevice, pCanvas);
    pEarthOrbit->initModel();

    pPlane = new ModelPlane(allocator, pLogicalDevice, pCanvas);
    pPlane->initModel();

    return true;
}


bool Orrery::renderOrrery(VkCommandBuffer cmdBuffer, float timeStep) 
{ 
    
    uint32_t w = pCanvas->getWidth();
    uint32_t h = pCanvas->getHeight();
    proj = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.1f, 1000.0f);

    // Sun, Earth, Moon, etc.
    renderSolarSystem(cmdBuffer, timeStep);


    // Just something pretty...
    static float aRot = 0.0f;
    aRot += timeStep * 0.15f;
    glm::mat4 a(1);
    a = glm::translate(a, glm::vec3(0.0f, 0.0f, -10.0f));
    a = glm::rotate(a, glm::radians(7.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    a = glm::translate(a, glm::vec3(2.5f, 2.0f, 0.0f));
    a = glm::rotate(a, aRot, glm::vec3(1.0f, 1.0f, 1.0f));
    pAxes->drawModel(cmdBuffer, proj, a);


    // Disk/platform under the model
    glm::mat4 platMat(1);
    platMat = glm::translate(platMat, glm::vec3(0.0f, 0.0f, -30.0f));
    platMat = glm::translate(platMat, glm::vec3(0.0f, -5.0f, 0.0f));
    platMat = glm::rotate(platMat, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    pPlane->drawModel(cmdBuffer, proj, platMat);



    
    return true;
}


void Orrery::renderSolarSystem(VkCommandBuffer cmdBuffer, float timeStep, bool bMirror) {


    glm::mat4 sunPos(1);

    if (bMirror) sunPos = glm::scale(sunPos, glm::vec3(1.0f, -1.0f, 1.0f));

    sunPos = glm::translate(sunPos, glm::vec3(0.0f, 0.0f, -30.0f));
    sunPos = glm::rotate(sunPos, glm::radians(7.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    sunPos = glm::translate(sunPos, glm::vec3(0.0f, -1.5f, 0.0f));
    pSun->drawModel(cmdBuffer, proj, sunPos);

    glm::mat4 orbitPos = sunPos;
    orbitPos = glm::rotate(orbitPos, glm::radians(7.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    orbitPos = glm::rotate(orbitPos, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    pEarthOrbit->drawModel(cmdBuffer, proj, orbitPos);

    static float earthRot = 0.0f;
    earthRot += timeStep * 0.5f;
    glm::mat4 earthPos = sunPos;
    earthPos = glm::rotate(earthPos, glm::radians(7.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    earthPos = glm::rotate(earthPos, earthRot, glm::vec3(0.0f, 1.0f, 0.0f));
    earthPos = glm::translate(earthPos, glm::vec3(10.0f, 0.0f, 0.0f));
    pEarth->drawModel(cmdBuffer, proj, earthPos);

    static float moonRot = 0.0f;
    moonRot += timeStep * 1.0f;
    glm::mat4 moonPos = earthPos;
    moonPos = glm::rotate(moonPos, glm::radians(17.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    moonPos = glm::rotate(moonPos, moonRot, glm::vec3(0.0f, 1.0f, 0.0f));
    moonPos = glm::translate(moonPos, glm::vec3(1.0f, 0.0f, 0.0f));

    pMoon->drawModel(cmdBuffer, proj, moonPos);
}
