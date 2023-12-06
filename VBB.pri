#Vulkan Building Blocks Qt qmake project include file

INCLUDEPATH += $$PWD
           

HEADERS +=	$$PWD/VBBInstance.h \
            $$PWD/VBBPhysicalDevices.h \
    	    $$PWD/VBBDevice.h \
            $$PWD/VBBFence.h \
            $$PWD/VBBCanvas.h \
            $$PWD/VBBDescriptors.h \
            $$PWD/VBBBufferDynamic.h \
            $$PWD/VBBBufferStatic.h \
            $$PWD/VBBBufferUniform.h \
            $$PWD/VBBDescriptors.h \
            $$PWD/VBBPipelineCompute.h \
            $$PWD/VBBPipelineGraphics.h \
            $$PWD/VBBShaderModule.h \
            $$PWD/SingleShotCommand.h \
            $$PWD/VBBTexture.h \
            $$PWD/VBBTextureStreaming.h \
            $$PWD/VBBUtils.h \
            $$PWD/QtVulkanWindow.h


SOURCES +=	$$PWD/VBBInstance.cpp \
            $$PWD/VBBPhysicalDevices.cpp \
            $$PWD/VBBDevice.cpp \
            $$PWD/VBBFence.cpp \
            $$PWD/VBBCanvas.cpp \
            $$PWD/VBBDescriptors.cpp \
            $$PWD/VBBBufferDynamic.cpp \
            $$PWD/VBBBufferStatic.cpp \
            $$PWD/VBBBufferUniform.cpp \
            $$PWD/VBBDescriptors.cpp \
            $$PWD/VBBPipelineCompute.h \
            $$PWD/VBBPipelineGraphics.cpp \
            $$PWD/VBBShaderModule.cpp \
            $$PWD/SingleShotCommand.cpp \
            $$PWD/VBBTexture.cpp \
            $$PWD/VBBTextureStreaming.cpp \
            $$PWD/VBBUtils.cpp \
            $$PWD/QtVulkanWindow.cpp

			
            
