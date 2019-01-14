/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
//                            avtOSPRayVoxelExtractor.h                      //
// ************************************************************************* //

#ifndef AVT_OSPRAY_VOXEL_EXTRACTOR_H
#define AVT_OSPRAY_VOXEL_EXTRACTOR_H

#include <filters_exports.h>

#include <avtVoxelExtractor.h>
#include <avtOSPRayCommon.h>
#include <avtOpacityMap.h>

#include <DebugStream.h>

#include <vtkMatrix3x3.h>
#include <vtkMatrix4x4.h>
#include <vtkCamera.h>

#include <stdlib.h>
#include <stdio.h>
#include <algorithm>

class     vtkRectilinearGrid;
class     vtkMatrix4x4;

// ****************************************************************************
//  Class: avtOSPRayVoxelExtractor
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
//    Qi Wu, Sun Jul 1 2018
//    Added support for ospray volume rendering.
//
// ****************************************************************************

class AVTFILTERS_API avtOSPRayVoxelExtractor : public avtVoxelExtractor
{
  public:
                     avtOSPRayVoxelExtractor(int, int, int, avtVolume *,
                                            avtCellList *);
    virtual         ~avtOSPRayVoxelExtractor();

    void             Extract(vtkRectilinearGrid *,
                             std::vector<std::string> &varnames,
                             std::vector<int> &varsize);

    // void             SetVariableInformation(std::vector<std::string> &names,
    //                                         std::vector<int> varsize);

    void             SetProcIdPatchID(int c, int p)   { proc = c; patch = p; };
    void             SetOSPRay(OSPVisItContext* o)        { ospray_core = o; };
    void             SetViewInfo(const avtViewInfo & v)      { viewInfo = v; };
    void             SetSamplingRate(double r)           { samplingRate = r; };
    void             SetRenderingExtents(int extents[4]) 
    {
        renderingExtents[0] = extents[0];
        renderingExtents[1] = extents[1];
        renderingExtents[2] = extents[2];       
        renderingExtents[3] = extents[3];
    };
    void             SetMVPMatrix(vtkMatrix4x4 *mvp)
    {
        model_to_screen_transform->DeepCopy(mvp); 
        vtkMatrix4x4::Invert(model_to_screen_transform, 
                             screen_to_model_transform); 
    };
    void             SetScalarRange(double r[2])
    {
        scalarRange[0] = r[0];
        scalarRange[1] = r[1];
    };
    void             SetTFVisibleRange(double r[2])
    {
        tFVisibleRange[0] = r[0];
        tFVisibleRange[1] = r[1];
    };
    void             GetImageDimensions(int&,int dims[2],int screen_ll[2],
                                        int screen_ur[2],float &, float &);
    void             GetComputedImage(float *image);

  protected:
    // the output image
    float           *finalImage;
    // some meta information
    bool             drawn;  // whether the patch is drawn or not
    int              patch;  // id of the patch
    int              proc;   // id of the processor
    // fields
    avtViewInfo      viewInfo;
    OSPVisItContext *ospray_core;    
    double           samplingRate;    
    int              renderingExtents[4];
    // matrix
    vtkMatrix4x4    *model_to_screen_transform;
    vtkMatrix4x4    *screen_to_model_transform;
    // others
    double           scalarRange[2];
    double           tFVisibleRange[2];
    int              imgWidth;
    int              imgHeight;
    int              imgDims[2];       // size of the patch
    int              imgLowerLeft[2];  // coordinates in the whole image
    int              imgUpperRight[2]; // coordinates in the whole image
    float            eyeSpaceDepth;    // for blending patches
    float            clipSpaceDepth;   // clip space depth for blending with bg
    int              xMin, xMax, yMin, yMax;

    void             ExtractWorldSpaceGridOSPRay(vtkRectilinearGrid *,  
                                   std::vector<std::string> &varnames,
                                           std::vector<int> &varsize);
};

#endif
