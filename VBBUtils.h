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

    void startBuilding(uint16_t estimatedVertexCount, float epsilon = 0.0000001);
    void addVertex(VBBSimpleVertex* pVertex, VBBSimpleNormal* pNormal, VBBSimpleTexCoord* pTexCoord, uint16_t searchOnlyLast = 0);
    void addVertex(void* pVertex, void* pNormal, void* pTexCoord, uint16_t searchOnlyLast = 0);

    VBBSimpleVertex* getVertexPointer(void) { return m_verticies.data(); }
    VBBSimpleNormal* getNomrlaPointer(void) { return m_normals.data(); }
    VBBSimpleTexCoord* getTexCoordPonter(void) { return m_texCoords.data(); }
    uint16_t* getIndexPointer(void) { return m_indexes.data(); }
    uint32_t getIndexCount(void) { return static_cast<uint32_t>(m_indexes.size()); }
    uint32_t getAttributeCount(void) { return static_cast<uint32_t>(m_verticies.size()); }

    // TBD: Make this class streamable so it can be archived
    // save(filename)
    // load(filename)
    // save/load(stream)

  protected:
    std::vector<VBBSimpleVertex> m_verticies;
    std::vector<VBBSimpleNormal> m_normals;
    std::vector<VBBSimpleTexCoord> m_texCoords;
    std::vector<uint16_t> m_indexes;

    float m_epsilon = 0.0000001;

    inline bool closeEnough(float first, float second) { return (fabs(first - second) < m_epsilon) ? true : false; }
};

void VBBMakeTorus(VBBSimpleIndexedMesh& torusBatch, float majorRadius, float minorRadius, uint16_t numMajor, uint16_t numMinor);
void VBBMakeSphere(VBBSimpleIndexedMesh& sphereBatch, double radius, uint32_t iSlices, uint32_t iStacks);
void VBBMakeCylinder(VBBSimpleIndexedMesh& cylinderBatch, float baseRadius, float topRadius, float fLength, uint32_t numSlices,
                     uint32_t numStacks, uint32_t degrees = 360);
void VBBMakeDisk(VBBSimpleIndexedMesh& diskBatch, float innerRadius, float outerRadius, uint32_t nSlices, uint32_t nStacks,
                 uint32_t degrees = 360);

unsigned char* vbbReadTGABits(const char* szFileName, uint32_t* iWidth, uint32_t* iHeight, uint32_t* iComponents, VkFormat* format,
                              unsigned char* pMemoryBuffer = nullptr);
