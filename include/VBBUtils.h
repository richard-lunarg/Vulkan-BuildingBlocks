/* Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Copyright © 2023 Richard S. Wright Jr. (richard@lunarg.com)
 *
 * This software is part of the Vulkan Building Blocks
 */

#pragma once

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vma/vk_mem_alloc.h"

#ifdef VK_NO_PROTOTYPES
#include <volk/volk.h>
#else
#include <vulkan/vulkan.h>
#endif


#include <vector>
#include <stdint.h>
#include <math.h>
#include <stdio.h>

// Define targa header. This is only used locally.
#pragma pack(1)
typedef struct {
    unsigned char identsize;        // Size of ID field that follows header (0)
    unsigned char colorMapType;     // 0 = None, 1 = paletted
    unsigned char imageType;        // 0 = none, 1 = indexed, 2 = rgb, 3 = grey, +8=rle
    unsigned short colorMapStart;   // First colour map entry
    unsigned short colorMapLength;  // Number of colors
    unsigned char colorMapBits;     // bits per palette entry
    unsigned short xstart;          // image x origin
    unsigned short ystart;          // image y origin
    unsigned short width;           // width in pixels
    unsigned short height;          // height in pixels
    unsigned char bits;             // bits per pixel (8 16, 24, 32)
    unsigned char descriptor;       // image descriptor
} TGAHEADER;
#pragma pack()

class VBBSimpleIndexedMesh {
  public:
    struct VBBSimpleVertex {
        float x;
        float y;
        float z;
    };

    struct VBBSimpleNormal {
        float x;
        float y;
        float z;
    };

    struct VBBSimpleTexCoord {
        float s;
        float t;
    };

    void startBuilding(uint32_t estimatedVertexCount, float epsilon = 0.0000001);
    void addVertex(VBBSimpleVertex* pVertex, VBBSimpleNormal* pNormal, VBBSimpleTexCoord* pTexCoord, uint32_t searchOnlyLast = 0);
    void addVertex(void* pVertex, void* pNormal, void* pTexCoord, uint32_t searchOnlyLast = 0);

    VBBSimpleVertex* getVertexPointer(void) { return m_vertices.data(); }
    VBBSimpleNormal* getNormalPointer(void) { return m_normals.data(); }
    VBBSimpleTexCoord* getTexCoordPointer(void) { return m_texCoords.data(); }
    uint32_t* getIndexPointer(void) { return m_indexes.data(); }
    uint32_t getIndexCount(void) { return static_cast<uint32_t>(m_indexes.size()); }
    uint32_t getAttributeCount(void) { return static_cast<uint32_t>(m_vertices.size()); }

    bool saveMesh(const char* szMeshFile) {
        if(szMeshFile != nullptr) {
            FILE *pFile;
            pFile = fopen(szMeshFile, "wb");
            if(pFile == NULL)
                return false; // Fail gracefully

            uint32_t nIndexCount = (uint32_t)m_indexes.size();
            uint32_t nVertexCount = (uint32_t)m_vertices.size();
            fwrite(&nIndexCount, sizeof(uint32_t), 1, pFile);
            fwrite(&nVertexCount, sizeof(uint32_t), 1, pFile);

            fwrite(m_indexes.data(), sizeof(uint32_t), nIndexCount, pFile);
            fwrite(m_vertices.data(), sizeof(VBBSimpleVertex), nVertexCount, pFile);
            fwrite(m_normals.data(), sizeof(VBBSimpleNormal), nVertexCount, pFile);
            fwrite(m_texCoords.data(), sizeof(VBBSimpleTexCoord), nVertexCount, pFile);
            fclose(pFile);
            return true;
            }

        return false;
    }


    bool loadMesh(const char* szMeshFile) {
        FILE *pFile;
        pFile = fopen(szMeshFile, "rb");
        if(pFile != NULL) { // Falls through to build

            // Get counts
            uint32_t nIndexCount;
            uint32_t nVertexCount;
            fread(&nIndexCount, sizeof(uint32_t), 1, pFile);
            fread(&nVertexCount, sizeof(uint32_t), 1, pFile);

            // Allocate memory
            m_indexes.resize(nIndexCount);
            m_vertices.resize(nVertexCount);
            m_normals.resize(nVertexCount);
            m_texCoords.resize(nVertexCount);

            // Read it in
            fread(m_indexes.data(), sizeof(uint32_t), nIndexCount, pFile);
            fread(m_vertices.data(), sizeof(VBBSimpleVertex), nVertexCount, pFile);
            fread(m_normals.data(), sizeof(VBBSimpleNormal), nVertexCount, pFile);
            fread(m_texCoords.data(), sizeof(VBBSimpleTexCoord), nVertexCount, pFile);

            fclose(pFile);
            return true;
            }

        return false;
    }

  protected:
    std::vector<VBBSimpleVertex> m_vertices;
    std::vector<VBBSimpleNormal> m_normals;
    std::vector<VBBSimpleTexCoord> m_texCoords;
    std::vector<uint32_t> m_indexes;

    float m_epsilon = 0.0000001;

    // Comparing floats is messy, use this instead of "==" and allow "close enough" to be equivalent
    inline bool closeEnough(float first, float second) { return (fabs(first - second) < m_epsilon) ? true : false; }
};

// ******************************
// Some pre-built objects
void VBBMakeTorus(VBBSimpleIndexedMesh& torusBatch, float majorRadius, float minorRadius, uint16_t numMajor, uint16_t numMinor);
void VBBMakeSphere(VBBSimpleIndexedMesh& sphereBatch, double radius, uint32_t iSlices, uint32_t iStacks);
void VBBMakeCylinder(VBBSimpleIndexedMesh& cylinderBatch, float baseRadius, float topRadius, float fLength, uint32_t numSlices,
                     uint32_t numStacks, uint32_t degrees = 360);
void VBBMakeDisk(VBBSimpleIndexedMesh& diskBatch, float innerRadius, float outerRadius, uint32_t nSlices, uint32_t nStacks,
                 uint32_t degrees = 360);

// ******************************
// Other little tidbits
unsigned char* vbbReadTGABits(const char* szFileName, uint32_t* iWidth, uint32_t* iHeight, uint32_t* iComponents, VkFormat* format,
                              unsigned char* pMemoryBuffer = nullptr);

int getBytesPerPixel(VkFormat format);
