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
//                            avtMassVoxelExtractor.h                        //
// ************************************************************************* //

#ifndef AVT_MASS_VOXEL_EXTRACTOR_H
#define AVT_MASS_VOXEL_EXTRACTOR_H

#include <filters_exports.h>

#include <avtExtractor.h>
#include <avtViewInfo.h>
#include <avtOpacityMap.h>
#include <avtSLIVROSPRayFilter.h>
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
//  Class: avtMassVoxelExtractor
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
//    Qi WU, Wed Jun 20 2018
//    Add support for volume rendering using OSPRay (RayCasting:OSPRay)
//
// ****************************************************************************

class AVTFILTERS_API avtMassVoxelExtractor : public avtExtractor
{
public:
                         avtMassVoxelExtractor(int, int, int, avtVolume *, avtCellList *);
        virtual         ~avtMassVoxelExtractor();

        void             Extract(vtkRectilinearGrid *,
                                 std::vector<std::string> &varnames,
                                 std::vector<int> &varsize);

        void             SetGridsAreInWorldSpace
            (bool, const avtViewInfo &,double, const double *);
        void             SetVariableInformation
            (std::vector<std::string> &names, std::vector<int> varsize);
        void             SetTrilinear(bool t) { trilinearInterpolation = t; };

        //
        // RayCast SLIVR Specific
        //
        void             SetRayCastingSLIVR(bool s) { rayCastingSLIVR = s; };
        void             SetLighting(bool l) { lighting = l; };
        void             SetLightDirection(double lightDir[3])
        { for (int i=0;i<3;i++) { lightDirection[i] = lightDir[i]; } }
        void             SetLightPosition(double lightPos[4]) 
        { for (int i=0;i<4;i++) { lightPosition[i] = lightPos[i]; } }
        void             SetMatProperties(double matProp[4]) 
        { for (int i=0;i<4;i++) { materialProperties[i] = matProp[i]; } }
        void             SetScalarRange(double range[2])
        { scalarRange[0] = range[0]; scalarRange[1] = range[1];}
        void             SetTFVisibleRange(double tfRange[2])
        { tFVisibleRange[0] = tfRange[0]; tFVisibleRange[1] = tfRange[1]; }
        void             SetTransferFn(avtOpacityMap *tf1D) 
        { transferFn1D = tf1D; };
        void             SetViewDirection(double *_vD)
        { for (int i=0; i<3; i++) { viewDirection[i] = view_direction[i] = _vD[i]; } }
        void             SetCameraPosition(double *_cp) 
        { std::copy(_cp, _cp + 3, cameraPosition); }
        void             SetCameraUpVector(double *_cu) 
        { std::copy(_cu, _cu + 3, cameraUpVector); }
        void             SetCameraAspect(double _a) { cameraAspect = _a; }
        void             SetClipPlanes(double _camClip[2])
        { clipPlanes[0] = _camClip[0]; clipPlanes[1] = _camClip[1]; }
        void             SetPanPercentages(double _pan[2])
        { panPercentage[0] = _pan[0]; panPercentage[1] = _pan[1]; }
        void             SetImageZoom(double _zoom) { imageZoom = _zoom; }
        void             SetDepthExtents(double _depthExtents[2])
        { 
            fullVolumeDepthExtents[0] = _depthExtents[0]; 
            fullVolumeDepthExtents[1] = _depthExtents[1]; 
        }
        void             SetMVPMatrix(vtkMatrix4x4 *_mvp)
        { 
            model_to_screen_transform->DeepCopy(_mvp); 
            vtkMatrix4x4::Invert(model_to_screen_transform, screen_to_model_transform); 
        }

        //
        // Getting the image
        //
        void             GetImageDimensions
            (int &, int dims[2], int screen_ll[2], int screen_ur[2], float &, float &);
        void             GetComputedImage(float *);
        void             SetProcIdPatchID(int _proc, int _patch)
        { proc = _proc; patch = _patch; }

        //
        // Set the background information
        //
        void             SetDepthBuffer(float *_zBuffer, int size) { depthBuffer=_zBuffer; }
        void             SetRGBBuffer(unsigned char  *_colorBuffer, int width, int height)
        { rgbColorBuffer=_colorBuffer; };
        void             SetBufferExtents(int _extents[4])
        { for (int i=0;i<4; i++) bufferExtents[i]=_extents[i]; }
        void             SetRendererSampleRate(double r) { rendererSampleRate = r; }
        void             SetOSPRay(OSPVisItContext* o) { ospray = o; }
        void             SetFullImageExtents(int extents[4]) 
        {
            fullImageExtents[0] = extents[0];
            fullImageExtents[1] = extents[1];
            fullImageExtents[2] = extents[2];   
            fullImageExtents[3] = extents[3];
        }

protected:
        int              fullImageExtents[4];
        bool             gridsAreInWorldSpace;
        bool             pretendGridsAreInWorldSpace;
        avtViewInfo      view;
        double           aspect;
        double           cur_clip_range[2];
        vtkMatrix4x4    *view_to_world_transform;
        vtkMatrix4x4    *world_to_view_transform;

        bool             trilinearInterpolation;
        bool             rayCastingSLIVR;

        vtkMatrix4x4    *model_to_screen_transform;
        vtkMatrix4x4    *screen_to_model_transform;
        double           clipPlanes[2];
        double           panPercentage[2];
        double           imageZoom;
        double           fullVolumeDepthExtents[2];
        double           viewDirection[3];  // -->
        double           view_direction[3]; // they are redundant. One of them should be removed
        double           cameraPosition[3]; // (Qi) camera location in world coordinate
        double           cameraUpVector[3]; // (Qi) camera up vector direction
        double           cameraAspect;
        double          *X;
        double          *Y;
        double          *Z;
        unsigned char   *ghosts;
        int              dims[3];
        int              ncell_arrays;
        void            *cell_arrays[AVT_VARIABLE_LIMIT];
        int              cell_size[AVT_VARIABLE_LIMIT];
        int              cell_index[AVT_VARIABLE_LIMIT];
        int              cell_vartypes[AVT_VARIABLE_LIMIT];
        int              npt_arrays;
        void            *pt_arrays[AVT_VARIABLE_LIMIT];
        int              pt_size[AVT_VARIABLE_LIMIT];
        int              pt_index[AVT_VARIABLE_LIMIT];
        int              pt_vartypes[AVT_VARIABLE_LIMIT];

        double          *prop_buffer;
        int             *ind_buffer;
        bool            *valid_sample;

        // We repeatedly divide by the term (X[i+1]-X[i]).  In the interest of
        // performance, cache the term 1./(X[i+1]-X[i]) and use that for faster
        // multiplication.  This sped up total performance by about 5%.
        double           *divisors_X;
        double           *divisors_Y;
        double           *divisors_Z;

        void             ExtractImageSpaceGrid(vtkRectilinearGrid *,
                                               std::vector<std::string> &varnames,
                                               std::vector<int> &varsize);
        void             ExtractWorldSpaceGrid(vtkRectilinearGrid *,
                                               std::vector<std::string> &varnames,
                                               std::vector<int> &varsize);

        void             RegisterGrid(vtkRectilinearGrid*,
                                      std::vector<std::string>&,std::vector<int>&);
        void             SampleAlongSegment(const double *, const double*, int, int);
        void             SampleVariable(int, int, int, int);
        bool             FrustumIntersectsGrid(int, int, int, int) const;
        bool             FrustumIntersectsGridSLIVR(int, int, int, int) const;
        void             GetSegment(int, int, double *, double *) const;
        static void      FindPlaneNormal(const double *, const double *,
                                         const double *, double *);
        bool             GridOnPlusSideOfPlane(const double *, const double *) const;
        bool             FindSegmentIntersections(const double *, const double *,
                                                  int &, int &);

        // Trilinear and RC SLIVR
        double           TrilinearInterpolate(double vals[8], float distRight, float distTop, float distBack);
        void             ComputeIndices(int dims[3], int indices[6], int returnIndices[8]);
        void             ComputeIndicesVert(int dims[3], int indices[6], int returnIndices[8]);
        void             GetIndexandDistFromCenter(float dist, int index,   
                                                   int &index_before, int &index_after,  
                                                   float &dist_before, float &dist_after);

        //
        // OSPRay stuffs
        //
        OSPVisItContext      *ospray;
        double           rendererSampleRate;

        //
        // RC SLIVR Specific
        //
        // Color computation
        bool             lighting;
        double           lightPosition[4];
        float            lightDirection[3];
        double           materialProperties[4];
        avtOpacityMap   *transferFn1D;
        float            gradient[3];
        double           scalarRange[2];
        double           tFVisibleRange[2];
        //
        // Background + other plots
        float           *depthBuffer;           // depth buffer for the background and other plots
        unsigned char   *rgbColorBuffer;        // bounding box + pseudo color + ...
        int              bufferExtents[4];      // extents of the buffer(minX, maxX, minY, maxY)
        //
        // Rendering
        // int              renderingAreaExtents[4];
        double           renderingDepthsExtents[2];
        //
        // Patch details for one image
        int              patchDrawn;            // whether the patch is drawn or not
        int              imgWidth, imgHeight;
        int              imgDims[2];            // size of the patch
        int              imgLowerLeft[2];       // coordinates in the whole image
        int              imgUpperRight[2];      //
        float            eyesSpaceDepth;         // for blending patches
        float            clipSpaceDepth;        // clip space depth for blending with other visit stuff
        float           *imgArray;              // the image data
        int              proc;                  // id of the processor
        int              patch;                 // id of the patch
        int              fullImgWidth, fullImgHeight;
        int              xMin, xMax, yMin, yMax;
        //
        // RC SLIVR Only
        void             ComputePixelColor(double source_rgb[4],
                                           double dest_rgb[4],
                                           float gradient[3]);
        //
        // added for raycasting slivr
        void             GetSegmentRCSLIVR(int x, int y,
                                           double depthsExtents[2],
                                           double *, double *);
        void             SampleVariableRCSLIVR(int first, int last, int intersect, int x, int y);
        void             ExtractWorldSpaceGridRCSLIVR(vtkRectilinearGrid *, 
                                                      std::vector<std::string> &varnames,
                                                      std::vector<int> &varsize);
};

#endif
