#include "ModelPlane.h"

// Uniform (push constant actually)
struct pushConstantDef {
    float mvp[16];
    float packMatrix[16];  // 3x3 normal matrix, with color in last column
};

ModelPlane::ModelPlane(VmaAllocator vmaAllocator, VBBDevice* pDevice, VBBCanvas* pCanv) : ModelBase(vmaAllocator, pDevice, pCanv) {}

ModelPlane::~ModelPlane() {
    delete pVertexBuffer;
    delete pTexCoordBuffer;

    delete pIndexBuffer;
    delete pDescriptors;
    delete pTexture;
}

bool ModelPlane::initModel(void) {

    uint32_t w, h, c;
    VkFormat format;
    unsigned char* pBytes = vbbReadTGABits("OrreryData/MilkyWay.tga", &w, &h, &c, &format, nullptr);

    // Incredi-Hac.... remap BGRA to RGBA
    for (int i = 0; i < w * h * 4; i += 4) {
        unsigned char red = pBytes[i];
        pBytes[i] = pBytes[i + 2];
        pBytes[i + 2] = red;
    }


    
    pTexture = new VBBTexture(Allocator, pLogicalDevice);
    int bytes = 4 * w * h;
    pTexture->loadRawTexture(pBytes, format, c, w, h, bytes);

    free(pBytes);

    // **************************************************
    // Create the pipeline
    pPipeline = new VBBPipelineGraphics();
    if (pPipeline == nullptr) return false;

    // What does the attribute data look like, and what is it's location
    pPipeline->addVertexAttributeBinding(sizeof(float) * 3, VK_VERTEX_INPUT_RATE_VERTEX, 0, VK_FORMAT_R32G32B32_SFLOAT);
    pPipeline->addVertexAttributeBinding(sizeof(float) * 2, VK_VERTEX_INPUT_RATE_VERTEX, 1, VK_FORMAT_R32G32_SFLOAT);

    // Push constants are SO FREAKING EASY
    VkPushConstantRange pushConstant;
    pushConstant.offset = 0;
    pushConstant.size = (sizeof(pushConstantDef));
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VBBShaderModule vertexShader;
    VBBShaderModule fragmentShader;
    // If these are undefined #define VBB_USE_SHADER_TOOLCHAIN
    vertexShader.loadGLSLANGFile(pLogicalDevice->getDevice(), "OrreryData/StockShader_TxModulate.vert",
                                 shaderc_glsl_default_vertex_shader);

    fragmentShader.loadGLSLANGFile(pLogicalDevice->getDevice(), "OrreryData/StockShader_TxModulate.frag",
                                   shaderc_glsl_default_fragment_shader);

    pPipeline->setPushConstants(1, &pushConstant);

    pPipeline->setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pPipeline->setFrontFace(VK_FRONT_FACE_COUNTER_CLOCKWISE);
    pPipeline->setCullMode(VK_CULL_MODE_BACK_BIT);
    pPipeline->setPolygonMode(VK_POLYGON_MODE_FILL);
    pPipeline->setEnableDepthTest(VK_TRUE);
    pPipeline->setEnableDepthWrite(VK_TRUE);
    pPipeline->setEnableBlend(VK_TRUE);
    pPipeline->setColorBlendFactors(VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);

    pDescriptors = new VBBDescriptors();
    pDescriptors->init(pCanvas->getLogicalDevice(), 1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT);

    static VkDescriptorSetLayout s[1];
    s[0] = pDescriptors->getLayout();
    pPipeline->setDescriptorSetLayouts(1, s);


    VkResult lastResult = pPipeline->createPipeline(pCanvas, vertexShader.getShaderModule(), fragmentShader.getShaderModule());
    if (lastResult != VK_SUCCESS) return false;

    VBBMakeDisk(plane, 0.0f, 13.0f, 100, 13);
    indexCount = plane.getIndexCount();
    attribCount = plane.getAttributeCount();

    pVertexBuffer = new VBBBufferDynamic(Allocator);
    pVertexBuffer->createBuffer(sizeof(float) * 3 * attribCount);
    void* pMapped = pVertexBuffer->mapMemory();
    memcpy(pMapped, plane.getVertexPointer(), sizeof(float) * 3 * attribCount);
    pVertexBuffer->unmapMemory();

    pTexCoordBuffer = new VBBBufferDynamic(Allocator);
    pTexCoordBuffer->createBuffer(sizeof(float) * 2 * attribCount);
    pMapped = pTexCoordBuffer->mapMemory();
    memcpy(pMapped, plane.getTexCoordPointer(), sizeof(float) * 2 * attribCount);
    pTexCoordBuffer->unmapMemory();

    pIndexBuffer = new VBBBufferStatic(Allocator);
    pIndexBuffer->createBuffer(plane.getIndexPointer(), sizeof(uint32_t) * indexCount, pLogicalDevice);

    return true;
}

bool ModelPlane::drawModel(VkCommandBuffer cmdBuffer, glm::mat4 proj, glm::mat4 modelView) {
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
    pc.packMatrix[15] = 0.7f;


    vkCmdPushConstants(cmdBuffer, pPipeline->getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConstantDef), &pc);

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = pTexture->getLayout();
    imageInfo.imageView = pTexture->getImageView();
    imageInfo.sampler = pTexture->getSampler();

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = pDescriptors->getDescriptorSet();
    descriptorWrite.dstBinding = 1;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.pBufferInfo = nullptr;
    descriptorWrite.pImageInfo = &imageInfo;
    descriptorWrite.pTexelBufferView = nullptr;
    descriptorWrite.descriptorCount = 1;

    vkUpdateDescriptorSets(pCanvas->getLogicalDevice(), 1, &descriptorWrite, 0, nullptr);
  

    VkDescriptorSet s = pDescriptors->getDescriptorSet();
    vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->getPipelineLayout(), 0, 1, &s, 0, nullptr);
 


    // Bind to geometry attribute data and draw
    VkBuffer vertexBuffers[] = {pVertexBuffer->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(cmdBuffer, 0, 1, vertexBuffers, offsets);

    VkBuffer textureBuffer[] = {pTexCoordBuffer->getBuffer()};
    vkCmdBindVertexBuffers(cmdBuffer, 1, 1, textureBuffer, offsets);

    vkCmdBindIndexBuffer(cmdBuffer, pIndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmdBuffer, indexCount, 1, 0, 0, 0);

    return true;
}
