#Vulkan Building Blocks Qt qmake project include file

INCLUDEPATH += $$PWD/../include
INCLUDEPATH += $$PWD
           

HEADERS +=  $$PWD/../include/VBBInstance.h \
            $$PWD/../include/VBBPhysicalDevices.h \
            $$PWD/../include/VBBDevice.h \
            $$PWD/../include/VBBFence.h \
            $$PWD/../include/VBBCanvas.h \
            $$PWD/../include/VBBDescriptors.h \
            $$PWD/../include/VBBBufferDynamic.h \
            $$PWD/../include/VBBBufferStatic.h \
            $$PWD/../include/VBBBufferUniform.h \
            $$PWD/../include/VBBPipelineCompute.h \
            $$PWD/../include/VBBPipelineGraphics.h \
            $$PWD/../include/VBBShaderModule.h \
            $$PWD/../include/VBBSingleShotCommand.h \
            $$PWD/../include/VBBTexture.h \
            $$PWD/../include/VBBTextureStreaming.h \
            $$PWD/../include/VBBUtils.h


SOURCES +=  $$PWD/../src/VBBInstance.cpp \
            $$PWD/../src/VBBPhysicalDevices.cpp \
            $$PWD/../src/VBBDevice.cpp \
            $$PWD/../src/VBBFence.cpp \
            $$PWD/../src/VBBCanvas.cpp \
            $$PWD/../src/VBBBufferDynamic.cpp \
            $$PWD/../src/VBBBufferStatic.cpp \
            $$PWD/../src/VBBBufferUniform.cpp \
            $$PWD/../src/VBBDescriptors.cpp \
            $$PWD/../src/VBBPipelineCompute.cpp \
            $$PWD/../src/VBBPipelineGraphics.cpp \
            $$PWD/../src/VBBShaderModule.cpp \
            $$PWD/../src/VBBTexture.cpp \
            $$PWD/../src/VBBTextureStreaming.cpp \
            $$PWD/../src/VBBUtils.cpp

            
