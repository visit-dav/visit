/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef IMG_METADATA_H
#define IMG_METADATA_H

#include <vtkMatrix4x4.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <limits>
#include <cmath>

// ****************************************************************************
//  Namespace:  slivr
//
//  Purpose:
//    
//
//  Programmer:  
//  Creation:   
//
// ****************************************************************************
namespace slivr
{
    double ProjectWorldToScreen
        (const double worldCoord[3], 
         const int screenWidth, const int screenHeight,  
         const double panPercentage[2], const double imageZoom,
         vtkMatrix4x4 *mvp, int screenCoord[2]);
  
    void ProjectScreenToWorld
        (const int screenCoord[2], const double z,
         const int screenWidth, const int screenHeight, 
         const double panPercentage[2], const double imageZoom,
         vtkMatrix4x4 *imvp, double worldCoord[3]);

    void ProjectScreenToCamera
        (const int screenCoord[2], const double z,
         const int screenWidth, const int screenHeight,
         vtkMatrix4x4 *imvp, double cameraCoord[3]);

    inline void ProjectScreenToWorld
        (const int x, const int y, const double z,
         const int screenWidth, const int screenHeight, 
         const double panPercentage[2], const double imageZoom,
         vtkMatrix4x4 *imvp, double worldCoord[3]) 
    {
        int screen_coord[2] = {x, y};
        slivr::ProjectScreenToWorld(screen_coord, z, screenWidth, screenHeight, 
                                    panPercentage, imageZoom, imvp, worldCoord);
    }

    void ProjectWorldToScreenCube
        (const double cube[6], const int screenWidth, const int screenHeight, 
         const double panPercentage[2], const double imageZoom, 
         vtkMatrix4x4 *mvp,int screenExtents[4], double depthExtents[2]);

    void CompositeBackground(int screen[2],
                             int compositedImageExtents[4],
                             int compositedImageWidth,
                             int compositedImageHeight,
                             float *compositedImageBuffer,
                             unsigned char *opaqueImageColor,
                             float         *opaqueImageDepth,
                             unsigned char *&imgFinal);
};

// ****************************************************************************
//  Struct:  ImgMetaData
//
//  Purpose:
//    Holds information about patches but not the image 
//
//  Programmer:  
//  Creation:   
//
// ****************************************************************************
namespace slivr
{
    struct ImgMetaData
    {
        int procId;       // processor that produced the patch
        int patchNumber;  // id of the patch on that processor
        int destProcId;   // destination proc where this patch gets composited
        int inUse;        // whether the patch is composed locally or not
        int dims[2];      // height, width
        int screen_ll[2]; // (lower left)  position in the final image
        int screen_ur[2]; // (upper right)
        float avg_z;      // camera space depth of the patch (average)
        float eye_z;      // camera space z
        float clip_z;     // clip space z
    };
};

// ****************************************************************************
//  Struct:  ImgData
//
//  Purpose:
//    Holds the image data generated
//
//  Programmer:  
//  Creation:    
//
// ****************************************************************************
namespace slivr 
{
    struct ImgData
    {
        // acts as a key
        int procId;        // processor that produced the patch
        int patchNumber;   // id of the patch on that processor
        float *imagePatch; // the image data - RGBA
        ImgData() { imagePatch = NULL; }
        bool operator==(const ImgData &a) {
            return (patchNumber == a.patchNumber);
        }
    };
}

// ****************************************************************************
//  Struct:  convexHull
//
//  Purpose:
//    Holds the image data generated
//
//  Programmer:  
//  Creation:    
//
// ****************************************************************************
namespace slivr 
{
    struct ConvexHull
    {
        int numPatches;
        // [0] rows along x axis, [1] rows along y axis, [2] rows along z axis
        int arrangement[3];
        float extents[6];       // minX, maxX   minY, maxY   minZ, maxZ
        float cellDims[3];      // x, y, z
        float tolerance;  
        // amount of overlap that is considered ok
        // -- typically 2 cells for cell centered data
        // 0: no overlap  
        // 1: overlpa in Z 
        // 2: overlap in Y
        // 3: overlap in Z
        int Overlap(ConvexHull);
    };
};

// ****************************************************************************
//  Template:  
//
//  Purpose:
//    
//
//  Programmer:  
//  Creation:    
//
// ****************************************************************************
#define CLAMP(x, l, h) (x > l ? x < h ? x : h : l)
#define M_MIN(x, r) (x < r ? x : r)
#define M_MAX(x, r) (x > r ? x : r)

// ****************************************************************************
//  Function:  
//
//  Purpose:
//    
//
//  Programmer:  
//  Creation:    
//
// ****************************************************************************
void WriteArrayToPPM
(std::string filename, float *image, int dimX, int dimY);
void WriteArrayToPPM
(std::string filename, unsigned char *image, int dimX, int dimY);
void WriteArrayGrayToPPM
(std::string filename , float * image, int dimX, int dimY);

#endif//IMG_METADATA_H
