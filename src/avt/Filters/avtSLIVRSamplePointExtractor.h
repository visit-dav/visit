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
//                      avtSLIVRSamplePointExtractor.h                       //
// ************************************************************************* //

#ifndef AVT_SLIVR_SAMPLE_POINT_EXTRACTOR_H
#define AVT_SLIVR_SAMPLE_POINT_EXTRACTOR_H

#include <filters_exports.h>

#include <avtSamplePointExtractorBase.h>

#include <vector>
#include <map>

#include <vtkMatrix4x4.h>

#include <avtSLIVRCommon.h>

class  avtSLIVRVoxelExtractor;


// ****************************************************************************
//  Class: avtSLIVRSamplePointExtractor
//
//  Purpose:
//      This is a component that will take an avtDataset as an input and find
//      all of the sample points from that dataset.
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//
//  Modifications:
//
//    Hank Childs, Sat Jan 27 15:09:34 PST 2001
//    Added support for sending cells when doing parallel volume rendering.
//
//    Kathleen Bonnell, Sat Apr 21, 13:09:27 PDT 2001 
//    Added recursive Execute method to walk down input data tree. 
//
//    Hank Childs, Tue Nov 13 15:51:15 PST 2001
//    Remove boolean argument to Extract<Cell> calls since it is no longer
//    necessary when all of the variables are being extracted.
//
//    Hank Childs, Sun Dec 14 11:07:56 PST 2003
//    Added mass voxel extractor.
//
//    Hank Childs, Fri Nov 19 13:41:56 PST 2004
//    Added view conversion option.
//
//    Hank Childs, Sat Jan 29 13:32:54 PST 2005
//    Added 2D extractors.
//
//    Hank Childs, Sun Dec  4 19:12:42 PST 2005
//    Added support for kernel-based sampling.
//
//    Hank Childs, Sun Jan  1 10:56:19 PST 2006
//    Added RasterBasedSample and KernelBasedSample.
//
//    Hank Childs, Tue Feb 28 08:25:33 PST 2006
//    Added PreExecute.
//
//    Jeremy Meredith, Thu Feb 15 11:44:28 EST 2007
//    Added support for rectilinear grids with an inherent transform.
//
//    Hank Childs, Fri Jun  1 11:47:56 PDT 2007
//    Add method GetLoadingInfoForArrays.
//
//    Hank Childs, Thu Sep 13 14:02:40 PDT 2007
//    Added support for hex-20s.
//
//    Hank Childs, Tue Jan 15 14:17:15 PST 2008
//    Have this class set up custom sample point arbitrators, since it has
//    the most knowledge.
//
//    Hank Childs, Fri Jan  9 14:09:57 PST 2009
//    Add support for jittering.
//
//    Kevin Griffin, Fri Apr 22 16:31:57 PDT 2016
//    Added support for polygons.
//
// ****************************************************************************

class AVTFILTERS_API avtSLIVRSamplePointExtractor 
    : public avtSamplePointExtractorBase
{
  public:
                              avtSLIVRSamplePointExtractor(int, int, int);
    virtual                  ~avtSLIVRSamplePointExtractor();

    virtual const char       *GetType(void)
                                         { return "avtSLIVRSamplePointExtractor"; };
    virtual const char       *GetDescription(void)
                                         { return "Extracting sample points";};


    void                      SetLighting(bool l) {lighting = l; };
    void                      SetLightPosition(double _lightPos[4]) { for (int i=0;i<4;i++) lightPosition[i]=_lightPos[i]; }
    void                      SetLightDirection(double _lightDir[3]) { for (int i=0;i<3;i++) lightDirection[i]=_lightDir[i]; }
    void                      SetMatProperties(double _matProp[4]) { for (int i=0;i<4;i++) materialProperties[i]=_matProp[i]; }
    void                      SetViewDirection(double *vD){ for (int i=0; i<3; i++) view_direction[i] = vD[i]; }
    void                      SetClipPlanes(double _camClip[2]){ clipPlanes[0]=_camClip[0]; clipPlanes[1]=_camClip[1]; }
    void                      SetPanPercentages(double _pan[2]){ panPercentage[0]=_pan[0]; panPercentage[1]=_pan[1]; }
    void                      SetDepthExtents(double _depthExtents[2]){ depthExtents[0]=_depthExtents[0]; depthExtents[1]=_depthExtents[1]; }
    void                      SetMVPMatrix(vtkMatrix4x4 *_mvp){ modelViewProj->DeepCopy(_mvp); }

    void                      getSpatialExtents(double _spatialExtents[6]){ for (int i=0; i<6; i++) _spatialExtents[i] = minMaxSpatialBounds[i]; }
    void                      getAvgPatchExtents(double _avgPatchExtents[6]){ for (int i=0; i<3; i++) _avgPatchExtents[i] = avgPatchExtents[i]; }
    void                      getCellDimension(double _cellDimension[6]){ for (int i=0; i<3; i++) _cellDimension[i] = cellDimension[i]; }

    // Getting image information
    int                       getImgPatchSize(){ return patchCount;}
                       // gets the number of patches
    imgMetaData               getImgMetaPatch(int patchId){ return imageMetaPatchVector.at(patchId);} // gets the metadata
    void                      getnDelImgData(int patchId, imgData &tempImgData);                      // gets the image & erase its existence
    void                      delImgPatches();                                                        // deletes patches

    // Set background buffer
    void                      setDepthBuffer(float *_zBuffer, int size){ depthBuffer=_zBuffer; }
    void                      setRGBBuffer(unsigned char  *_colorBuffer, int width, int height){ rgbColorBuffer=_colorBuffer; }
    void                      setBufferExtents(int _extents[4]){ for (int i=0;i<4; i++) bufferExtents[i]=_extents[i]; }

    // Output data for RC SLIVR
    std::vector<imgMetaData>    imageMetaPatchVector;
    std::multimap<int, imgData> imgDataHashMap;
    typedef std::multimap<int, imgData>::iterator iter_t;

  protected:
    virtual void              InitSampling(avtDataTree_p dt);
    virtual void              DoSampling(vtkDataSet *, int);
    virtual void              SetUpExtractors(void);
    virtual void              SendJittering(void);
    virtual bool              FilterUnderstandsTransformedRectMesh(void);   
    void                      RasterBasedSample(vtkDataSet *, int num=0);
    imgMetaData               initMetaPatch(int id);    // initialize a patch

    double                    minMaxSpatialBounds[6];
    double                    avgPatchExtents[3];
    double                    cellDimension[3];

    // Background + other plots
    float                    *depthBuffer;             // depth buffer for the background and other plots
    unsigned char            *rgbColorBuffer;          // bounding box + pseudo color + ...
    int                       bufferExtents[4];         // extents of the buffer( minX, maxX, minY, maxY)

    avtSLIVRVoxelExtractor   *slivrVoxelExtractor;
    int                       patchCount;
    // Camera stuff
    double                    view_direction[3];
    double                    depthExtents[2];
    double                    clipPlanes[2];
    double                    panPercentage[2];
    vtkMatrix4x4             *modelViewProj;

    // lighting & material
    bool                      lighting;
    double                    lightPosition[4];
    double                    lightDirection[3];
    double                    materialProperties[4];

};


#endif


