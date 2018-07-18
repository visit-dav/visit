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

#include "avtSLIVRImgMetaData.h"
#include "avtSLIVROSPRayFilter.h"
#include <avtMemory.h>
#include <avtParallel.h>
#include <ImproperUseException.h>
#include <DebugStream.h>
#include <TimingsManager.h>

// ***************************************************************************
// Threaded Blending
// ***************************************************************************

inline bool CheckThreadedBlend_MetaData()
{
    bool use = true;
    const char* env_use = std::getenv("SLIVR_NOT_USE_THREADED_BLEND");
    if (env_use) { 
        use = atoi(env_use) <= 0; 
    }
    if (!use) {
        ospout << "[avtSLIVRImgMetaData] "
               << "Not Using Multi-Threading for Blending"
               << std::endl;
    } else {
        ospout << "[avtSLIVRImgMetaData] "
               << "Using Multi-Threading for Blending"
               << std::endl;
    }
    return use;
}
#ifdef VISIT_OSPRAY
bool UseThreadedBlend_MetaData = CheckThreadedBlend_MetaData();
#else
bool UseThreadedBlend_MetaData = false;
#endif

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
double slivr::ProjectWorldToScreen
(const double worldCoord[3], const int screenWidth, const int screenHeight,
 const double panPercentage[2], const double imageZoom,
 vtkMatrix4x4 *mvp, int screenCoord[2])
{
    // world space coordinate in homogeneous coordinate
    double worldHCoord[4] = {
        worldCoord[0],
        worldCoord[1],
        worldCoord[2],
        1.0
    };

    // world to clip space (-1 ~ 1)
    double clipHCoord[4];
    mvp->MultiplyPoint(worldHCoord, clipHCoord);
    if (clipHCoord[3] == 0.0)
    {
        std::cerr << "slivr::ProjectWorldToScreen "
                  << "Zero Division During Projection" 
                  << std::endl;
        std::cerr << "world coordinates: (" 
                  << worldHCoord[0] << ", " 
                  << worldHCoord[1] << ", " 
                  << worldHCoord[2] << ", " 
                  << worldHCoord[3] << ")" << std::endl
                  << "clip space coordinate: ("
                  << clipHCoord[0] << ", " 
                  << clipHCoord[1] << ", " 
                  << clipHCoord[2] << ", "
                  << clipHCoord[3] << std::endl;
        std::cerr << "Matrix: " << *mvp << std::endl;
        EXCEPTION1(VisItException, "Zero Division During Projection");
    }

    // normalize clip space coordinate
    double clipCoord[3] = {
        clipHCoord[0]/clipHCoord[3],
        clipHCoord[1]/clipHCoord[3],
        clipHCoord[2]/clipHCoord[3]
    };

    // screen coordinates (int integer)
    screenCoord[0] = round(clipCoord[0]*(screenWidth /2.0)+(screenWidth /2.0));
    screenCoord[1] = round(clipCoord[1]*(screenHeight/2.0)+(screenHeight/2.0));

    // add panning
    screenCoord[0] += round(screenWidth  * panPercentage[0] * imageZoom);
    screenCoord[1] += round(screenHeight * panPercentage[1] * imageZoom); 

    // return point depth
    return clipCoord[2];
}

void
slivr::ProjectScreenToWorld
(const int screenCoord[2], const double z,
 const int screenWidth, const int screenHeight, 
 const double panPercentage[2], const double imageZoom,
 vtkMatrix4x4 *imvp, double worldCoord[3])
{
    // remove panning
    const int x = 
        screenCoord[0] - round(screenWidth*panPercentage[0]*imageZoom);
    const int y = 
        screenCoord[1] - round(screenHeight*panPercentage[1]*imageZoom);
    
    // do projection
    double worldHCoord[4] = {0,0,0,1};
    double clipHCoord[4] = {
        (x - screenWidth/2.0) /(screenWidth/2.0),
        (y - screenHeight/2.0)/(screenHeight/2.0),
        z, 1.0};
    imvp->MultiplyPoint(clipHCoord, worldHCoord);
    if (worldHCoord[3] == 0) {
        debug5 << "slivr::ProjectScreenToWorld "
               << "Zero Division During Projection" 
               << std::endl;
        std::cerr << "world coordinates: (" 
                  << worldHCoord[0] << ", " 
                  << worldHCoord[1] << ", " 
                  << worldHCoord[2] << ", " 
                  << worldHCoord[3] << ")" << std::endl
                  << "clip space coordinate: ("
                  << clipHCoord[0] << ", " 
                  << clipHCoord[1] << ", " 
                  << clipHCoord[2] << ", "
                  << clipHCoord[3] << std::endl;
        std::cerr << "Matrix: " << *imvp << std::endl;
        EXCEPTION1(VisItException, "Zero Division During Projection");
    }
    
    // normalize world space coordinate 
    worldCoord[0] = worldHCoord[0]/worldHCoord[3];
    worldCoord[1] = worldHCoord[1]/worldHCoord[3];
    worldCoord[2] = worldHCoord[2]/worldHCoord[3];
}

void
slivr::ProjectScreenToCamera
(const int screenCoord[2], const double z,
 const int screenWidth, const int screenHeight, 
 vtkMatrix4x4 *imvp, double cameraCoord[3])
{
    // remove panning
    const int x = screenCoord[0];
    const int y = screenCoord[1];
    
    // do projection
    double cameraHCoord[4] = {0,0,0,1};
    double clipHCoord[4] = {
        (x - screenWidth /2.0)/(screenWidth /2.0),
        (y - screenHeight/2.0)/(screenHeight/2.0),
        z,
        1.0};
    imvp->MultiplyPoint(clipHCoord, cameraHCoord);
    if (cameraHCoord[3] == 0) {
        debug5 << "slivr::ProjectScreenToWorld "
               << "Zero Division During Projection" 
               << std::endl;
        std::cerr << "world coordinates: (" 
                  << cameraHCoord[0] << ", " 
                  << cameraHCoord[1] << ", " 
                  << cameraHCoord[2] << ", " 
                  << cameraHCoord[3] << ")" << std::endl
                  << "clip space coordinate: ("
                  << clipHCoord[0] << ", " 
                  << clipHCoord[1] << ", " 
                  << clipHCoord[2] << ", "
                  << clipHCoord[3] << std::endl;
        std::cerr << "Matrix: " << *imvp << std::endl;
        EXCEPTION1(VisItException, "Zero Division During Projection");
    }
    
    // normalize world space coordinate 
    cameraCoord[0] = cameraHCoord[0]/cameraHCoord[3];
    cameraCoord[1] = cameraHCoord[1]/cameraHCoord[3];
    cameraCoord[2] = cameraHCoord[2]/cameraHCoord[3];
}

void
slivr::ProjectWorldToScreenCube
(const double cube[6], const int screenWidth, const int screenHeight, 
 const double panPercentage[2], const double imageZoom, vtkMatrix4x4 *mvp, 
 int screenExtents[4], double depthExtents[2])
{
    int xMin = std::numeric_limits<int>::max();
    int xMax = std::numeric_limits<int>::min();
    int yMin = std::numeric_limits<int>::max();
    int yMax = std::numeric_limits<int>::min();
    double zMin = std::numeric_limits<double>::max();
    double zMax = std::numeric_limits<double>::min();

    float coordinates[8][3];
    coordinates[0][0] = cube[0];   
    coordinates[0][1] = cube[2];   
    coordinates[0][2] = cube[4];        

    coordinates[1][0] = cube[1];   
    coordinates[1][1] = cube[2];   
    coordinates[1][2] = cube[4];        

    coordinates[2][0] = cube[1];  
    coordinates[2][1] = cube[3];
    coordinates[2][2] = cube[4];        

    coordinates[3][0] = cube[0]; 
    coordinates[3][1] = cube[3]; 
    coordinates[3][2] = cube[4];

    coordinates[4][0] = cube[0];
    coordinates[4][1] = cube[2];
    coordinates[4][2] = cube[5];

    coordinates[5][0] = cube[1]; 
    coordinates[5][1] = cube[2]; 
    coordinates[5][2] = cube[5];        

    coordinates[6][0] = cube[1]; 
    coordinates[6][1] = cube[3];
    coordinates[6][2] = cube[5];

    coordinates[7][0] = cube[0]; 
    coordinates[7][1] = cube[3]; 
    coordinates[7][2] = cube[5];

    double worldCoord[3];
    int screenCoord[2]; double depth;
    for (int i=0; i<8; i++)
    {
        worldCoord[0] = coordinates[i][0];
        worldCoord[1] = coordinates[i][1];
        worldCoord[2] = coordinates[i][2];
        depth = slivr::ProjectWorldToScreen
            (worldCoord, screenWidth, screenHeight, 
             panPercentage, imageZoom, mvp, screenCoord);
        // clamp values
        screenCoord[0] = CLAMP(screenCoord[0], 0, screenWidth);
        screenCoord[1] = CLAMP(screenCoord[1], 0, screenHeight);
        screenExtents[0] = xMin = std::min(xMin, screenCoord[0]);
        screenExtents[1] = xMax = std::max(xMax, screenCoord[0]);
        screenExtents[2] = yMin = std::min(yMin, screenCoord[1]);
        screenExtents[3] = yMax = std::max(yMax, screenCoord[1]);
        depthExtents[0] = zMin = std::min(zMin, depth);
        depthExtents[1] = zMax = std::max(zMax, depth);
    }
}


void
slivr::CompositeBackground(int screen[2],
                         int compositedImageExtents[4],
                         int compositedImageWidth,
                         int compositedImageHeight,
                         float *compositedImageBuffer,
                         unsigned char *opaqueImageColor,
                         float         *opaqueImageDepth,
                         unsigned char *&imgFinal)
{
#ifdef VISIT_OSPRAY
    if (UseThreadedBlend_MetaData) {
    visit::CompositeBackground(screen,
                               compositedImageExtents,
                               compositedImageWidth,
                               compositedImageHeight,
                               compositedImageBuffer,
                               opaqueImageColor,
                               opaqueImageDepth,
                               imgFinal);
    } else {
#endif
    for (int y = 0; y < screen[1]; y++)
    {
        for (int x = 0; x < screen[0]; x++)
        {
            int indexScreen     = y * screen[0] + x;
            int indexComposited =
                (y - compositedImageExtents[2]) * compositedImageWidth +
                (x - compositedImageExtents[0]);

            bool insideComposited = 
                ((x >= compositedImageExtents[0] && 
                  x < compositedImageExtents[1]) &&
                 (y >= compositedImageExtents[2] && 
                  y < compositedImageExtents[3]));

            if (insideComposited)
            {
                if (compositedImageBuffer[indexComposited*4 + 3] == 0)
                {
                    // No data from rendering here! - Good
                    imgFinal[indexScreen * 3 + 0] = 
                        opaqueImageColor[indexScreen * 3 + 0];
                    imgFinal[indexScreen * 3 + 1] = 
                        opaqueImageColor[indexScreen * 3 + 1];
                    imgFinal[indexScreen * 3 + 2] = 
                        opaqueImageColor[indexScreen * 3 + 2];
                }
                else
                {
                    // Volume in front
                    float alpha = 
                        (1.0 - compositedImageBuffer[indexComposited * 4 + 3]);
                    imgFinal[indexScreen * 3 + 0] = 
                        CLAMP(opaqueImageColor[indexScreen * 3 + 0] * alpha +
                              compositedImageBuffer[indexComposited * 4 + 0] *
                              255.f,
                              0.f, 255.f);
                    imgFinal[indexScreen * 3 + 1] = 
                        CLAMP(opaqueImageColor[indexScreen * 3 + 1] * alpha +
                              compositedImageBuffer[indexComposited * 4 + 1] *
                              255.f,
                              0.f, 255.f);
                    imgFinal[indexScreen * 3 + 2] =
                        CLAMP(opaqueImageColor[indexScreen * 3 + 2] * alpha +
                              compositedImageBuffer[indexComposited * 4 + 2] *
                              255.f,
                              0.f, 255.f);
                }
            }
            else
            {
                // Outside bounding box: Use the background : Good
                imgFinal[indexScreen * 3 + 0] = 
                    opaqueImageColor[indexScreen * 3 + 0];
                imgFinal[indexScreen * 3 + 1] =
                    opaqueImageColor[indexScreen * 3 + 1];
                imgFinal[indexScreen * 3 + 2] =
                    opaqueImageColor[indexScreen * 3 + 2];
            }
        }
    }
#ifdef VISIT_OSPRAY
    }
#endif
}


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

void WriteArrayToPPM(std::string filename, float * image, int dimX, int dimY)
{
    std::ofstream outputFile((filename+ ".ppm").c_str(), 
                             std::ios::out | std::ios::binary);
    outputFile <<  "P6\n" << dimX << "\n" << dimY << "\n" << 255 << "\n"; 
    for (int y=dimY-1; y>=0; --y)
    {
        for (int x=0; x<dimX; ++x)
        {
            int index = (y * dimX + x)*4;
            char color[3];
            float alpha = image[index + 3];
            color[0] = CLAMP(image[index + 0]*alpha, 0.0f, 1.0f) * 255;
            color[1] = CLAMP(image[index + 1]*alpha, 0.0f, 1.0f) * 255;
            color[2] = CLAMP(image[index + 2]*alpha, 0.0f, 1.0f) * 255;
            outputFile.write(color,3);
        }
    } 
    outputFile.close();
}

void WriteArrayToPPM(std::string filename, 
                     unsigned char *image, 
                     int dimX, int dimY)
{
    std::ofstream outputFile((filename+ ".ppm").c_str(), 
                             std::ios::out | std::ios::binary);
    outputFile <<  "P6\n" << dimX << "\n" << dimY << "\n" << 255 << "\n"; 
    for (int y=dimY-1; y>=0; --y)
    {
        outputFile.write(reinterpret_cast<char*>(&image[y * dimX * 3]), 
                         dimX * 3);
    } 
    outputFile.close();
}

void WriteArrayGrayToPPM(std::string filename, 
                         float* image, 
                         int dimX, int dimY)
{
    std::ofstream outputFile((filename+ ".ppm").c_str(), 
                             std::ios::out | std::ios::binary);
    outputFile <<  "P6\n" << dimX << "\n" << dimY << "\n" << 255 << "\n"; 
    for (int y=dimY-1; y>=0; --y)
    {
        for (int x=0; x<dimX; ++x)
        {
            int index = (y * dimX + x);
            char var = CLAMP(image[index], 0.f, 1.f) * 255;
            char color[3];
            color[0] = var;
            color[1] = var;
            color[2] = var;
            outputFile.write(color,3);
        }
    } 
    outputFile.close();
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
int slivr::ConvexHull::Overlap(ConvexHull _hull) 
{
    if ( (_hull.extents[1] < extents[0]) || 
         (_hull.extents[0] > extents[1]) )   // No overlap in X
    {
        if ( (_hull.extents[3] < extents[2]) || 
             (_hull.extents[2] > extents[3]) )   // No overlap in Y
        {
            if ( (_hull.extents[5] < extents[4]) ||
                 (_hull.extents[4] > extents[5]) )   // No overlap in Z
            {
                return 0;
            }
            else
            {
                return 3;
            }
        }
        else
        {
            return 2;
        }
    }
    else
    {
        return 1;
    }
}
