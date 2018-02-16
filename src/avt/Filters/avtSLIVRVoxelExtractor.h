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

// ************************************************************************* //
//                            avtSLIVRVoxelExtractor.h                       //
// ************************************************************************* //

#ifndef AVT_SLIVR_VOXEL_EXTRACTOR_H
#define AVT_SLIVR_VOXEL_EXTRACTOR_H

#include <filters_exports.h>

#include <avtVoxelExtractor.h>

#include <avtOpacityMap.h>

#include <DebugStream.h>
class     vtkRectilinearGrid;
class     vtkMatrix4x4;

#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkCamera.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include "imgMetaData.h"

// ****************************************************************************
//  Class: avtSLIVRVoxelExtractor
//
//  Purpose:
//      Extracts sample points from a collection of voxels.  It assumes that 
//      the voxels it has been given are in camera space and does not try to
//      populate points that are not in the cube [-1, 1], [-1, 1], [-1, 1].
//
//  Programmer: Hank Childs
//  Creation:   December 14, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined constructor and destructor definitions to .C files
//    because certain compilers have problems with them.
//
//    Hank Childs, Fri Nov 19 14:50:58 PST 2004
//    Added support for accepting grids that need to do a world space to
//    image space conversion as well.  Also changed API to AVTFILTERS_API.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Hank Childs, Fri Jun  1 15:28:14 PDT 2007
//    Added support for non-scalars.
//
//    Hank Childs, Wed Aug 27 11:24:53 PDT 2008
//    Add support for non-floats.
//
//    Hank Childs, Wed Dec 24 11:24:47 PST 2008
//    Remove data member ProportionSpaceToZBufferSpace, as we now do our
//    sampling in even intervals (wbuffer).
//
//    Kathleen Biagas, Fri Jul 13 09:44:45 PDT 2012
//    Use double internally instead of float.
//
// ****************************************************************************

class AVTFILTERS_API avtSLIVRVoxelExtractor : public avtVoxelExtractor
{
  public:
                     avtSLIVRVoxelExtractor(int, int, int, avtVolume *,
                                            avtCellList *);
    virtual         ~avtSLIVRVoxelExtractor();

    void             Extract(vtkRectilinearGrid *,
                             std::vector<std::string> &varnames,
                             std::vector<int> &varsize);

//    void             SetVariableInformation(std::vector<std::string> &names,
//                                            std::vector<int> varsize);

    //
    // RC SLIVR Specific
    //

    void             SetRayCastingSLIVR(bool s) {rayCastingSLIVR = s; };

    void             SetLighting(bool l) {lighting = l; };
    void             SetLightDirection(double _lightDir[3]) { for (int i=0;i<3;i++) lightDirection[i]=_lightDir[i]; }
    void             SetLightPosition(double _lightPos[4]) { for (int i=0;i<4;i++) lightPosition[i]=_lightPos[i]; }
    void             SetMatProperties(double _matProp[4]) { for (int i=0;i<4;i++) materialProperties[i]=_matProp[i]; }
                        
    void             SetScalarRange(double _range[2]){ scalarRange[0]=_range[0]; scalarRange[1]=_range[1];}
    void             SetTFVisibleRange(double _tfRange[2]){ tFVisibleRange[0]=_tfRange[0]; tFVisibleRange[1]=_tfRange[1];}

    void             SetTransferFn(avtOpacityMap *_transferFn1D) { transferFn1D = _transferFn1D; };

    void             SetViewDirection(double *vD){ for (int i=0; i<3; i++) viewDirection[i]=view_direction[i] = vD[i]; }
    void             SetClipPlanes(double _camClip[2]){ clipPlanes[0]=_camClip[0]; clipPlanes[1]=_camClip[1]; }
    void             SetPanPercentages(double _pan[2]){ panPercentage[0]=_pan[0]; panPercentage[1]=_pan[1]; }
    void             SetDepthExtents(double _depthExtents[2]){ fullVolumeDepthExtents[0]=_depthExtents[0]; fullVolumeDepthExtents[1]=_depthExtents[1]; }
    void             SetMVPMatrix(vtkMatrix4x4 *_mvp){ modelViewProj->DeepCopy(_mvp); vtkMatrix4x4::Invert(modelViewProj, invModelViewProj); }


    // Getting the image
    void             getImageDimensions(int &inUse, int dims[2], int screen_ll[2], int screen_ur[2], float &eyeDepth, float &clipDepth);
    void             getComputedImage(float *image);
    void             setProcIdPatchID(int _proc, int _patch){ proc = _proc; patch = _patch; }

    // Set the background information
    void             setDepthBuffer(float *_zBuffer, int size){ depthBuffer=_zBuffer; }
    void             setRGBBuffer(unsigned char  *_colorBuffer, int width, int height){ rgbColorBuffer=_colorBuffer; };
    void             setBufferExtents(int _extents[4]){ for (int i=0;i<4; i++) bufferExtents[i]=_extents[i]; }

  protected:
    bool            rayCastingSLIVR;

    vtkMatrix4x4    *modelViewProj;
    vtkMatrix4x4    *invModelViewProj;
    double           clipPlanes[2];
    double           panPercentage[2];
    double           fullVolumeDepthExtents[2];
    double           viewDirection[3];
    double           view_direction[3];

    double          *prop_buffer;
    int             *ind_buffer;
    bool            *valid_sample;

    void             SampleAlongSegment(const double *, const double*, int, int);
    void             SampleVariable(int, int, int, int);

    bool             FrustumIntersectsGridSLIVR(int, int, int, int) const;

    //
    // RC SLIVR Specific
    //

    // Color computation
    bool             lighting;
    double           lightPosition[4];
    float            lightDirection[3];
    double           materialProperties[4];
    avtOpacityMap    *transferFn1D;
    float            gradient[3];
    double           scalarRange[2];
    double           tFVisibleRange[2];


    // Background + other plots
    float           *depthBuffer;           // depth buffer for the background and other plots
    unsigned char   *rgbColorBuffer;        // bounding box + pseudo color + ...
    int              bufferExtents[4];      // extents of the buffer( minX, maxX, minY, maxY)

    // Rendering
    int              renderingAreaExtents[4];
    double           renderingDepthsExtents[2];


    // Patch details for one image
    int              patchDrawn;            // whether the patch is drawn or not

    int              imgWidth, imgHeight;
    int              imgDims[2];            // size of the patch

    int              imgLowerLeft[2];       // coordinates in the whole image
    int              imgUpperRight[2];      //

    float            eyeSpaceDepth;         // for blending patches
    float            clipSpaceDepth;        // clip space depth for blending with other visit stuff

    float            *imgArray;             // the image data

    int              proc;                  // id of the processor
    int              patch;                 // id of the patch

    int              fullImgWidth, fullImgHeight;
    int              xMin, xMax, yMin, yMax;


    // RC SLIVR Only
    void             normalize(float vec[3]);
    void             reflect(float vec[3], float normal[3], float refl[3]);
    float            dot(float vecA[3], float vecB[3]){ return ((vecA[0]*vecB[0]) + (vecA[1]*vecB[1]) + (vecA[2]*vecB[2])); }
    void             unProject(int _x, int _y, float _z, double _worldCoordinates[3], int _width, int _height);
    double           project(double _worldCoordinates[3], int pos2D[2], int _width, int _height);

    void             computePixelColor(double source_rgb[4], double dest_rgb[4], float _gradient[3]);

    void             GetSegmentRCSLIVR(int x, int y, double depthsExtents[2], double *_origin, double *_terminus);
    void             SampleVariableRCSLIVR(int first, int last, int intersect, int x, int y);
    void             ExtractWorldSpaceGridRCSLIVR(vtkRectilinearGrid *,  // added for raycasting slivr
                             std::vector<std::string> &varnames,
                             std::vector<int> &varsize);
};

#endif
