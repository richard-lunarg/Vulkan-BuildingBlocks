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
#include <assert.h>

#ifdef VK_NO_PROTOTYPES
#include <volk/volk.h>
#else
#include <vulkan/vulkan.h>
#endif

#include "VBBUtils.h"
#include <array>

static const double VBB_PI = 3.14159265358979323846;

// Just takes a stab at reserving enough space ahead of time to avoid re-allocation
void VBBSimpleIndexedMesh::startBuilding(uint32_t estimatedVertexCount, float epsilon) {
    m_vertices.reserve(estimatedVertexCount);
    m_normals.reserve(estimatedVertexCount);
    m_texCoords.reserve(estimatedVertexCount);
    m_indexes.reserve(estimatedVertexCount);

    m_epsilon = epsilon;
}

void VBBSimpleIndexedMesh::addVertex(void* pVertex, void* pNormal, void* pTexCoord, uint32_t searchOnlyLast) {
    addVertex(static_cast<VBBSimpleVertex*>(pVertex), static_cast<VBBSimpleNormal*>(pNormal),
              static_cast<VBBSimpleTexCoord*>(pTexCoord), searchOnlyLast);
}

// *********************************************************************************************************
// This function builds an index array for use as a geometry mesh. It takes up to 4 arrays, all of which are
// optional, except for the vertex positon data
// searchOnlyLast = 0 -> search ALL of the previously submitted vertices
// searchOnlyLast > 0 -> search only the last <searchOnlyLast> values for a match
void VBBSimpleIndexedMesh::addVertex(VBBSimpleVertex* pVertex, VBBSimpleNormal* pNormal, VBBSimpleTexCoord* pTexCoord,
                                     uint32_t searchOnlyLast) {
    uint32_t index = 0;  // Index is currently unknown

    // Start at the beginning, or use the hint
    uint32_t startIndex = 0;
    if(searchOnlyLast != 0)
        if (searchOnlyLast < m_indexes.size()) startIndex = static_cast<uint32_t>(m_vertices.size()) - searchOnlyLast;

    for (uint32_t i = startIndex; i < m_vertices.size(); i++) {
        index = i;
        
        // All we have is vertex positions
        if (pNormal == nullptr && pTexCoord == nullptr) {
            if (closeEnough(m_vertices[index].x, pVertex->x) && closeEnough(m_vertices[index].y, pVertex->y) &&
                closeEnough(m_vertices[index].z, pVertex->z)) {
                m_indexes.push_back(index);
                m_vertices.push_back(*pVertex);
                return;
            }
            else continue;
        }

        // We have/want all three
        if (pNormal != nullptr && pTexCoord != nullptr) {
            if (closeEnough(m_vertices[index].x, pVertex->x) && closeEnough(m_vertices[index].y, pVertex->y) &&
                closeEnough(m_vertices[index].z, pVertex->z) &&

                closeEnough(m_normals[index].x, pNormal->x) && closeEnough(m_normals[index].y, pNormal->y) &&
                closeEnough(m_normals[index].z, pNormal->z) &&

                closeEnough(m_texCoords[index].s, pTexCoord->s) && closeEnough(m_texCoords[index].t, pTexCoord->t)) {
                m_indexes.push_back(index);
                return;
            }
            else continue;
        }

        // We have normals, but no texture coordinates
        if (pNormal != nullptr && pTexCoord == nullptr) {
            if (closeEnough(m_vertices[index].x, pVertex->x) && closeEnough(m_vertices[index].y, pVertex->y) &&
                closeEnough(m_vertices[index].z, pVertex->z) &&

                closeEnough(m_normals[index].x, pNormal->x) && closeEnough(m_normals[index].y, pNormal->y) &&
                closeEnough(m_normals[index].z, pNormal->z)) {
                m_indexes.push_back(index);
                return;
            }
            else continue;
        }

        // We have no normals, but we do have texture coordinates
        if (pNormal == nullptr && pTexCoord != nullptr) {
            if (closeEnough(m_vertices[index].x, pVertex->x) && closeEnough(m_vertices[index].y, pVertex->y) &&
                closeEnough(m_vertices[index].z, pVertex->z) &&

                closeEnough(m_texCoords[index].s, pTexCoord->s) && closeEnough(m_texCoords[index].t, pTexCoord->t)) {
                m_indexes.push_back(index);
                return;
            }
            else continue;
        }
    }

    m_vertices.push_back(*pVertex);
    if (pNormal != nullptr) m_normals.push_back(*pNormal);
    if (pTexCoord != nullptr) m_texCoords.push_back(*pTexCoord);
    if(m_vertices.size() == 1)
        m_indexes.push_back(0);
    else
        m_indexes.push_back((index+1));
    
    assert(m_vertices.size() < 65535);
}


// Pass in a normal by reference, and normalize it in place
static inline void normalize(VBBSimpleIndexedMesh::VBBSimpleNormal& normal) {
    double sum = (normal.x * normal.x) + (normal.y * normal.y) + (normal.z * normal.z);
    double len = sqrt(sum);
    normal.x /= len;
    normal.y /= len;
    normal.z /= len;
}

static inline void normalize(float normal[3]) {
    double sum = (normal[0] * normal[0]) + (normal[1] * normal[1]) + (normal[2] * normal[2]);
    double len = sqrt(sum);
    normal[0] /= len;
    normal[1] /= len;
    normal[2] /= len;
}

// Draw a torus (doughnut)  at z = fZVal... torus is in xy plane
void VBBMakeTorus(VBBSimpleIndexedMesh& torusBatch, float majorRadius, float minorRadius, uint16_t numMajor, uint16_t numMinor) {
    double majorStep = 2.0f * VBB_PI / numMajor;
    double minorStep = 2.0f * VBB_PI / numMinor;
    int i, j;

    torusBatch.startBuilding(numMajor * (numMinor + 1) * 6);
    for (i = 0; i < numMajor; ++i) {
        double a0 = i * majorStep;
        double a1 = a0 + majorStep;
        double x0 = cos(a0);
        double y0 = sin(a0);
        double x1 = cos(a1);
        double y1 = sin(a1);

        VBBSimpleIndexedMesh::VBBSimpleVertex vVertex[4];
        VBBSimpleIndexedMesh::VBBSimpleNormal vNormal[4];
        VBBSimpleIndexedMesh::VBBSimpleTexCoord vTexture[4];

        for (j = 0; j <= numMinor; ++j) {
            double b = j * minorStep;
            double c = cos(b);
            double r = minorRadius * c + majorRadius;
            double z = minorRadius * sin(b);

            // First point
            vTexture[0].s = double(i) / double(numMajor);
            vTexture[0].t = double(j) / double(numMinor);
            vNormal[0].x = x0 * c;
            vNormal[0].y = y0 * c;
            vNormal[0].z = z / minorRadius;
            normalize(vNormal[0]);
            vVertex[0].x = x0 * r;
            vVertex[0].y = y0 * r;
            vVertex[0].z = z;

            // Second point
            vTexture[1].s = double(i + 1) / double(numMajor);
            vTexture[1].t = double(j) / double(numMinor);
            vNormal[1].x = x1 * c;
            vNormal[1].y = y1 * c;
            vNormal[1].z = z / minorRadius;
            normalize(vNormal[1]);
            vVertex[1].x = x1 * r;
            vVertex[1].y = y1 * r;
            vVertex[1].z = z;

            // Next one over
            b = (j + 1) * minorStep;
            c = cos(b);
            r = minorRadius * c + majorRadius;
            z = minorRadius * sin(b);

            // Third (based on first)
            vTexture[2].s = (double)(i) / (double)(numMajor);
            vTexture[2].t = (double)(j + 1) / (double)(numMinor);
            vNormal[2].x = x0 * c;
            vNormal[2].y = y0 * c;
            vNormal[2].z = z / minorRadius;
            normalize(vNormal[2]);
            vVertex[2].x = x0 * r;
            vVertex[2].y = y0 * r;
            vVertex[2].z = z;

            // Fourth (based on second)
            vTexture[3].s = (double)(i + 1) / (double)(numMajor);
            vTexture[3].t = (double)(j + 1) / (double)(numMinor);
            vNormal[3].x = x1 * c;
            vNormal[3].y = y1 * c;
            vNormal[3].z = z / minorRadius;
            normalize(vNormal[3]);
            vVertex[3].x = x1 * r;
            vVertex[3].y = y1 * r;
            vVertex[3].z = z;

            torusBatch.addVertex(&vVertex[0], &vNormal[0], &vTexture[0]);
            torusBatch.addVertex(&vVertex[1], &vNormal[1], &vTexture[1]);
            torusBatch.addVertex(&vVertex[2], &vNormal[2], &vTexture[2]);

            torusBatch.addVertex(&vVertex[1], &vNormal[1], &vTexture[1]);
            torusBatch.addVertex(&vVertex[3], &vNormal[3], &vTexture[3]);
            torusBatch.addVertex(&vVertex[2], &vNormal[2], &vTexture[2]);
        }
    }
}

void VBBMakeSphere(VBBSimpleIndexedMesh& sphereBatch, double radius, uint32_t iSlices, uint32_t iStacks) {
    double drho = 3.141592653589 / iStacks;
    double dtheta = (2.0 * 3.141592653589) / double(iSlices);
    double ds = 1.0 / double(iSlices);
    double dt = 1.0 / double(iStacks);
    double t = 1.0;
    double s = 0.0;
    uint32_t i, j;  // Looping variables

    sphereBatch.startBuilding(iSlices * iStacks * 6);
    for (i = 0; i < iStacks; i++) {
        double rho = double(i) * drho;
        double srho = sin(rho);
        double crho = cos(rho);
        double srhodrho = sin(rho + drho);
        double crhodrho = cos(rho + drho);

        s = 0.0f;
        std::array<float, 3> vVertex[4];
        std::array<float, 3> vNormal[4];
        std::array<float, 2> vTexture[4];

        for (j = 0; j < iSlices; j++) {
            double theta = (j == iSlices) ? 0.0f : j * dtheta;
            double stheta = -sin(theta);
            double ctheta = cos(theta);

            double x = stheta * srho;
            double y = ctheta * srho;
            double z = crho;

            vTexture[0][0] = s;
            vTexture[0][1] = t;
            vNormal[0][0] = x;
            vNormal[0][1] = y;
            vNormal[0][2] = z;
            vVertex[0][0] = x * radius;
            vVertex[0][1] = y * radius;
            vVertex[0][2] = z * radius;

            x = stheta * srhodrho;
            y = ctheta * srhodrho;
            z = crhodrho;

            vTexture[1][0] = s;
            vTexture[1][1] = t - dt;
            vNormal[1][0] = x;
            vNormal[1][1] = y;
            vNormal[1][2] = z;
            vVertex[1][0] = x * radius;
            vVertex[1][1] = y * radius;
            vVertex[1][2] = z * radius;

            theta = ((j + 1) == iSlices) ? 0.0f : (j + 1) * dtheta;
            stheta = -sin(theta);
            ctheta = cos(theta);

            x = stheta * srho;
            y = ctheta * srho;
            z = crho;

            s += ds;
            vTexture[2][0] = s;
            vTexture[2][1] = t;
            vNormal[2][0] = x;
            vNormal[2][1] = y;
            vNormal[2][2] = z;
            vVertex[2][0] = x * radius;
            vVertex[2][1] = y * radius;
            vVertex[2][2] = z * radius;

            x = stheta * srhodrho;
            y = ctheta * srhodrho;
            z = crhodrho;

            vTexture[3][0] = s;
            vTexture[3][1] = t - dt;
            vNormal[3][0] = x;
            vNormal[3][1] = y;
            vNormal[3][2] = z;
            vVertex[3][0] = x * radius;
            vVertex[3][1] = y * radius;
            vVertex[3][2] = z * radius;

            sphereBatch.addVertex(vVertex[0].data(), vNormal[0].data(), vTexture[0].data(),1);
            sphereBatch.addVertex(vVertex[1].data(), vNormal[1].data(), vTexture[1].data(),1);
            sphereBatch.addVertex(vVertex[2].data(), vNormal[2].data(), vTexture[2].data(),1);

            sphereBatch.addVertex(vVertex[1].data(), vNormal[1].data(), vTexture[1].data(),1);
            sphereBatch.addVertex(vVertex[3].data(), vNormal[3].data(), vTexture[3].data(),1);
            sphereBatch.addVertex(vVertex[2].data(), vNormal[2].data(), vTexture[2].data(),1);
        }
        t -= dt;
    }
}

void VBBMakeCylinder(VBBSimpleIndexedMesh& cylinderBatch, float baseRadius, float topRadius, float fLength, uint32_t numSlices,
                     uint32_t numStacks, uint32_t degrees) {
    double fRadiusStep = (topRadius - baseRadius) / double(numStacks);

    double fStepSizeSlice = (double(degrees) * (3.14159265 / 180.0)) / double(numSlices);

    std::array<float, 3> vVertex[4];
    std::array<float, 3> vNormal[4];
    std::array<float, 2> vTexture[4];

    cylinderBatch.startBuilding(numSlices * numStacks * 6);

    double ds = 1.0 / double(numSlices);
    double dt = 1.0 / double(numStacks);
    float s;
    float t;

    for (uint32_t i = 0; i < numStacks; i++) {
        if (i == 0)
            t = 0.0f;
        else
            t = float(i) * dt;

        float tNext;
        if (i == (numStacks - 1))
            tNext = 1.0f;
        else
            tNext = float(i + 1) * dt;

        double fCurrentRadius = baseRadius + (fRadiusStep * double(i));
        double fNextRadius = baseRadius + (fRadiusStep * double(i + 1));
        double theyta;
        double theytaNext;

        double fCurrentZ = double(i) * (fLength / double(numStacks));
        double fNextZ = double(i + 1) * (fLength / double(numStacks));

        double zNormal = 0.0f;
        if (fabs(baseRadius - topRadius) > 0.00000001) {
            // Rise over run...
            zNormal = (baseRadius - topRadius);
        }

        for (uint32_t j = 0; j < numSlices; j++) {
            if (j == 0)
                s = 0.0f;
            else
                s = float(j) * ds;

            float sNext;
            if (j == (numSlices - 1))
                sNext = 1.0f;
            else
                sNext = float(j + 1) * ds;

            theyta = fStepSizeSlice * float(j);
            if (j == (numSlices - 1) && degrees == 360)
                theytaNext = 0.0f;
            else
                theytaNext = fStepSizeSlice * (float(j + 1));

            // Inner First
            vVertex[1][0] = cos(theyta) * fCurrentRadius;  // X
            vVertex[1][1] = sin(theyta) * fCurrentRadius;  // Y
            vVertex[1][2] = fCurrentZ;                     // Z

            vNormal[1][0] = vVertex[1][0];  // Surface Normal, same for everybody
            vNormal[1][1] = vVertex[1][1];
            vNormal[1][2] = zNormal;
            normalize(vNormal[1].data());

            vTexture[1][0] = s;  // Texture Coordinates, I have no idea...
            vTexture[1][1] = t;

            // Outer First
            vVertex[0][0] = cos(theyta) * fNextRadius;  // X
            vVertex[0][1] = sin(theyta) * fNextRadius;  // Y
            vVertex[0][2] = fNextZ;                     // Z

            if (fabs(fNextRadius) > 0.00001f) {
                vNormal[0][0] = vVertex[0][0];  // Surface Normal, same for everybody
                vNormal[0][1] = vVertex[0][1];  // For cones, tip is tricky
                vNormal[0][2] = zNormal;
                normalize(vNormal[0].data());
            } else
                vNormal[0] = vNormal[1];

            vTexture[0][0] = s;  // Texture Coordinates, I have no idea...
            vTexture[0][1] = tNext;

            // Inner second
            vVertex[3][0] = cos(theytaNext) * fCurrentRadius;  // X
            vVertex[3][1] = sin(theytaNext) * fCurrentRadius;  // Y
            vVertex[3][2] = fCurrentZ;                         // Z

            vNormal[3][0] = vVertex[3][0];  // Surface Normal, same for everybody
            vNormal[3][1] = vVertex[3][1];
            vNormal[3][2] = zNormal;
            normalize(vNormal[3].data());

            vTexture[3][0] = sNext;  // Texture Coordinates, I have no idea...
            vTexture[3][1] = t;

            // Outer second
            vVertex[2][0] = cos(theytaNext) * fNextRadius;  // X
            vVertex[2][1] = sin(theytaNext) * fNextRadius;  // Y
            vVertex[2][2] = fNextZ;                         // Z

            if (fabs(fNextRadius) > 0.00001f) {
                vNormal[2][0] = vVertex[2][0];  // Surface Normal, same for everybody
                vNormal[2][1] = vVertex[2][1];
                vNormal[2][2] = zNormal;
                normalize(vNormal[2].data());
            } else
                vNormal[2] = vNormal[1];

            vTexture[2][0] = sNext;  // Texture Coordinates, I have no idea...
            vTexture[2][1] = tNext;

            cylinderBatch.addVertex(vVertex[0].data(), vNormal[0].data(), vTexture[0].data());
            cylinderBatch.addVertex(vVertex[1].data(), vNormal[1].data(), vTexture[1].data());
            cylinderBatch.addVertex(vVertex[2].data(), vNormal[2].data(), vTexture[2].data());

            cylinderBatch.addVertex(vVertex[1].data(), vNormal[1].data(), vTexture[1].data());
            cylinderBatch.addVertex(vVertex[3].data(), vNormal[3].data(), vTexture[3].data());
            cylinderBatch.addVertex(vVertex[2].data(), vNormal[2].data(), vTexture[2].data());
        }
    }
}

void VBBMakeDisk(VBBSimpleIndexedMesh& diskBatch, float innerRadius, float outerRadius, uint32_t nSlices, uint32_t nStacks,
                 uint32_t degrees) {
    // How much to step out each stack
    double fStepSizeRadial = outerRadius - innerRadius;
    if (fStepSizeRadial < 0.0) fStepSizeRadial *= -1.0;

    fStepSizeRadial /= double(nStacks);

    float fStepSizeSlice = (double(degrees) * (3.14159265 / 180.0)) / double(nSlices);

    diskBatch.startBuilding(nSlices * nStacks * 6);

    std::array<float, 3> vVertex[4];
    std::array<float, 3> vNormal[4];
    std::array<float, 2> vTexture[4];

    double fRadialScale = 1.0 / outerRadius;

    for (uint32_t i = 0; i < nStacks; i++)  // Stacks
    {
        double theyta;
        double theytaNext;
        for (uint32_t j = 0; j < nSlices; j++)  // Slices
        {
            double inner = innerRadius + (double(i)) * fStepSizeRadial;
            double outer = innerRadius + (double(i + 1)) * fStepSizeRadial;

            theyta = fStepSizeSlice * double(j);
            if (j == (nSlices - 1) && degrees == 360)
                theytaNext = 0.0;
            else
                theytaNext = fStepSizeSlice * (double(j + 1));

            // Inner First
            vVertex[0][0] = cos(theyta) * inner;  // X
            vVertex[0][1] = sin(theyta) * inner;  // Y
            vVertex[0][2] = 0.0f;                 // Z

            vNormal[0][0] = 0.0f;  // Surface Normal, same for everybody
            vNormal[0][1] = 0.0f;
            vNormal[0][2] = 1.0f;

            vTexture[0][0] = ((vVertex[0][0] * fRadialScale) + 1.0f) * 0.5f;
            vTexture[0][1] = ((vVertex[0][1] * fRadialScale) + 1.0f) * 0.5f;

            // Outer First
            vVertex[1][0] = cos(theyta) * outer;  // X
            vVertex[1][1] = sin(theyta) * outer;  // Y
            vVertex[1][2] = 0.0f;                 // Z

            vNormal[1][0] = 0.0f;  // Surface Normal, same for everybody
            vNormal[1][1] = 0.0f;
            vNormal[1][2] = 1.0f;

            vTexture[1][0] = ((vVertex[1][0] * fRadialScale) + 1.0f) * 0.5f;
            vTexture[1][1] = ((vVertex[1][1] * fRadialScale) + 1.0f) * 0.5f;

            // Inner Second
            vVertex[2][0] = cos(theytaNext) * inner;  // X
            vVertex[2][1] = sin(theytaNext) * inner;  // Y
            vVertex[2][2] = 0.0f;                     // Z

            vNormal[2][0] = 0.0f;  // Surface Normal, same for everybody
            vNormal[2][1] = 0.0f;
            vNormal[2][2] = 1.0f;

            vTexture[2][0] = ((vVertex[2][0] * fRadialScale) + 1.0f) * 0.5f;
            vTexture[2][1] = ((vVertex[2][1] * fRadialScale) + 1.0f) * 0.5f;

            // Outer Second
            vVertex[3][0] = cos(theytaNext) * outer;  // X
            vVertex[3][1] = sin(theytaNext) * outer;  // Y
            vVertex[3][2] = 0.0f;                     // Z

            vNormal[3][0] = 0.0f;  // Surface Normal, same for everybody
            vNormal[3][1] = 0.0f;
            vNormal[3][2] = 1.0f;

            vTexture[3][0] = ((vVertex[3][0] * fRadialScale) + 1.0f) * 0.5f;
            vTexture[3][1] = ((vVertex[3][1] * fRadialScale) + 1.0f) * 0.5f;

            diskBatch.addVertex(vVertex[0].data(), vNormal[0].data(), vTexture[0].data());
            diskBatch.addVertex(vVertex[1].data(), vNormal[1].data(), vTexture[1].data());
            diskBatch.addVertex(vVertex[2].data(), vNormal[2].data(), vTexture[2].data());

            diskBatch.addVertex(vVertex[1].data(), vNormal[1].data(), vTexture[1].data());
            diskBatch.addVertex(vVertex[3].data(), vNormal[3].data(), vTexture[3].data());
            diskBatch.addVertex(vVertex[2].data(), vNormal[2].data(), vTexture[2].data());
        }
    }
}

////////////////////////////////////////////////////////////////////
// Allocate memory and load targa bits. Returns pointer to new buffer,
// height, and width of texture, and the OpenGL format of data.
// Call free() on buffer when finished!
// This only works on pretty vanilla targas... 8, 24, or 32 bit color
// only, no palettes, no RLE encoding.
unsigned char* vbbReadTGABits(const char* szFileName, uint32_t* iWidth, uint32_t* iHeight, uint32_t* iComponents, VkFormat* format,
                              unsigned char* pMemoryBuffer) {
    FILE* pFile;                  // File pointer
    TGAHEADER tgaHeader;          // TGA file header
    unsigned long lImageSize;     // Size in bytes of image
    short sDepth;                 // Pixel depth;
    unsigned char* pBits = NULL;  // Pointer to bits

    // Default/Failed values
    *iWidth = 0;
    *iHeight = 0;
    *iComponents = 3;

    // Attempt to open the file
    pFile = fopen(szFileName, "rb");
    if (pFile == NULL) return nullptr;

    // Read in header (binary)
    fread(&tgaHeader, 18 /* sizeof(TGAHEADER)*/, 1, pFile);

    // Get width, height, and depth of texture
    *iWidth = tgaHeader.width;
    *iHeight = tgaHeader.height;
    sDepth = tgaHeader.bits / 8;

    // Put some validity checks here. Very simply, I only understand
    // or care about 8, 24, or 32 bit targa's.
    if (tgaHeader.bits != 8 && tgaHeader.bits != 24 && tgaHeader.bits != 32) return nullptr;

    // Calculate size of image buffer
    lImageSize = tgaHeader.width * tgaHeader.height * sDepth;

    // Allocate memory and check for success
    if (pMemoryBuffer == nullptr)
        pBits = (unsigned char*)malloc(lImageSize * sizeof(unsigned char));
    else
        pBits = pMemoryBuffer;

    if (pBits == nullptr) return nullptr;

    // Read in the bits
    // Check for read error. This should catch RLE or other
    // weird formats that I don't want to recognize
    if (fread(pBits, lImageSize, 1, pFile) != 1) {
        if (pMemoryBuffer == nullptr)  // Only free if we also allocated
            free(pBits);

        return nullptr;
    }

    // Set format expected
    switch (sDepth) {
        case 3:  // Most likely case
            *iComponents = 3;
            *format = VK_FORMAT_R8G8B8_UNORM;
            break;
        case 4:
            *iComponents = 4;
            *format = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        case 1:
            *iComponents = 1;
            *format = VK_FORMAT_R8_UNORM;
            //*format = VK_FORMAT_B8G8R8_UNORM;
            break;
        default:  // RGB
            break;
    }

    // Done with File
    fclose(pFile);

    // Return pointer to image data
    return pBits;
}
