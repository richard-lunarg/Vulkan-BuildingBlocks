#include "ModelEarthOrbit.h"

// Uniform (push constant actually)
struct pushConstantDef {
    float mvp[16];
    float packMatrix[16];  // 3x3 normal matrix, with color in last column
};

ModelEarthOrbit::ModelEarthOrbit(VmaAllocator vmaAllocator, VBBDevice* pDevice, VBBCanvas* pCanv)
    : ModelBase(vmaAllocator, pDevice, pCanv) {}

ModelEarthOrbit::~ModelEarthOrbit() {
    delete pVertexBuffer;
    delete pNormalBuffer;
    delete pTexCoordBuffer;

    delete pIndexBuffer;
    delete pDescriptors;
}

bool ModelEarthOrbit::initModel(void) {
    // **************************************************
    // Create the pipeline
    pPipeline = new VBBPipelineGraphics();
    if (pPipeline == nullptr) return false;

    // What does the attribute data look like, and what is it's location
    pPipeline->addVertexAttributeBinding(sizeof(float) * 3, 0, VK_VERTEX_INPUT_RATE_VERTEX, 0, VK_FORMAT_R32G32B32_SFLOAT);
    pPipeline->addVertexAttributeBinding(sizeof(float) * 3, 0, VK_VERTEX_INPUT_RATE_VERTEX, 1, VK_FORMAT_R32G32B32_SFLOAT);

    // Push constants are SO FREAKING EASY
    VkPushConstantRange pushConstant;
    pushConstant.offset = 0;
    pushConstant.size = (sizeof(pushConstantDef));
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VBBShaderModule vertexShader;
    VBBShaderModule fragmentShader;
    // If these are undefined #define VBB_USE_SHADER_TOOLCHAIN
    vertexShader.loadGLSLANGFile(pLogicalDevice->getDevice(), "OrreryData/StockShader_FakeLight.vert",
                                 shaderc_glsl_default_vertex_shader);

    fragmentShader.loadGLSLANGFile(pLogicalDevice->getDevice(), "OrreryData/StockShader_FakeLight.frag",
                                   shaderc_glsl_default_fragment_shader);

    pPipeline->setPushConstants(1, &pushConstant);

    pPipeline->setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pPipeline->setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
    pPipeline->setCullMode(VK_CULL_MODE_BACK_BIT);
    pPipeline->setPolygonMode(VK_POLYGON_MODE_FILL);
    pPipeline->setEnableDepthTest(VK_TRUE);
    pPipeline->setEnableDepthWrite(VK_TRUE);

    VkResult lastResult = pPipeline->createPipeline(pCanvas, vertexShader.getShaderModule(), fragmentShader.getShaderModule());
    if (lastResult != VK_SUCCESS) return false;

    VBBMakeTorus(orbit, 10.0f, 0.04f, 100, 13);
    indexCount = orbit.getIndexCount();
    attribCount = orbit.getAttributeCount();

    pVertexBuffer = new VBBBufferDynamic(Allocator);
    pVertexBuffer->createBuffer(sizeof(float) * 3 * attribCount);
    void* pMapped = pVertexBuffer->mapMemory();
    memcpy(pMapped, orbit.getVertexPointer(), sizeof(float) * 3 * attribCount);
    pVertexBuffer->unmapMemory();

    pNormalBuffer = new VBBBufferDynamic(Allocator);
    pNormalBuffer->createBuffer(sizeof(float) * 3 * attribCount);
    pMapped = pNormalBuffer->mapMemory();
    memcpy(pMapped, orbit.getNormalPointer(), sizeof(float) * 3 * attribCount);
    pNormalBuffer->unmapMemory();

    pTexCoordBuffer = new VBBBufferDynamic(Allocator);
    pTexCoordBuffer->createBuffer(sizeof(float) * 2 * attribCount);
    pMapped = pTexCoordBuffer->mapMemory();
    memcpy(pMapped, orbit.getTexCoordPointer(), sizeof(float) * 2 * attribCount);
    pTexCoordBuffer->unmapMemory();

    pIndexBuffer = new VBBBufferStatic(Allocator);
    pIndexBuffer->createBuffer(orbit.getIndexPointer(), sizeof(uint32_t) * indexCount, pLogicalDevice);

    return true;
}

bool ModelEarthOrbit::drawModel(VkCommandBuffer cmdBuffer, glm::mat4 proj, glm::mat4 modelView) {
    // Bind to the pipeline that we want to use
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->getPipeline());

    // Red lines
    pushConstantDef pc;

    glm::mat4 mvp = proj * modelView;
    memcpy(pc.mvp, glm::value_ptr(mvp), sizeof(float) * 16);

    // Used to transform normals
    pc.packMatrix[0] = glm::value_ptr(modelView)[0];
    pc.packMatrix[1] = glm::value_ptr(modelView)[1];
    pc.packMatrix[2] = glm::value_ptr(modelView)[2];

    pc.packMatrix[4] = glm::value_ptr(modelView)[4];
    pc.packMatrix[5] = glm::value_ptr(modelView)[5];
    pc.packMatrix[6] = glm::value_ptr(modelView)[6];

    pc.packMatrix[8] = glm::value_ptr(modelView)[8];
    pc.packMatrix[9] = glm::value_ptr(modelView)[9];
    pc.packMatrix[10] = glm::value_ptr(modelView)[10];

    // Store color of object
    pc.packMatrix[12] = 1.0f;
    pc.packMatrix[13] = 1.0f;
    pc.packMatrix[14] = 1.0f;
    pc.packMatrix[15] = 1.0f;

    vkCmdPushConstants(cmdBuffer, pPipeline->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConstantDef), &pc);

    // Bind to geometry attribute data and draw
    VkBuffer vertexBuffers[] = {pVertexBuffer->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);

    VkBuffer normalBuffers[] = {pNormalBuffer->getBuffer()};
    vkCmdBindVertexBuffers(cmdBuffer, 1, 1, normalBuffers, offsets);

    vkCmdBindIndexBuffer(cmdBuffer, pIndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmdBuffer, indexCount, 1, 0, 0, 0);

    return true;
}
