/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                           avtSamplePointExtractor.h                       //
// ************************************************************************* //

#ifndef AVT_SAMPLE_POINT_EXTRACTOR_H
#define AVT_SAMPLE_POINT_EXTRACTOR_H

#include <filters_exports.h>

#include <avtDatasetToSamplePointsFilter.h>
#include <avtVolume.h>

#include <avtViewInfo.h>

class  vtkDataArray;
class  vtkDataSet;
class  vtkHexahedron;
class  vtkQuadraticHexahedron;
class  vtkPixel;
class  vtkPyramid;
class  vtkQuad;
class  vtkTetra;
class  vtkTriangle;
class  vtkVoxel;
class  vtkWedge;

class  avtHexahedronExtractor;
class  avtHexahedron20Extractor;
class  avtMassVoxelExtractor;
class  avtPointExtractor;
class  avtPyramidExtractor;
class  avtSamplePointArbitrator;
class  avtTetrahedronExtractor;
class  avtWedgeExtractor;

class  avtRayFunction;


// ****************************************************************************
//  Class: avtSamplePointExtractor
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
// ****************************************************************************

class AVTFILTERS_API avtSamplePointExtractor 
    : public avtDatasetToSamplePointsFilter
{
  public:
                              avtSamplePointExtractor(int, int, int);
    virtual                  ~avtSamplePointExtractor();

    virtual const char       *GetType(void)
                                         { return "avtSamplePointExtractor"; };
    virtual const char       *GetDescription(void)
                                         { return "Extracting sample points";};

    void                      RegisterRayFunction(avtRayFunction *rf)
                                         { rayfoo = rf; };
    void                      SendCellsMode(bool);
    void                      SetRectilinearGridsAreInWorldSpace(bool, 
                                                   const avtViewInfo &,double);
    void                      RestrictToTile(int, int, int, int);
    void                      StopTiling(void) { shouldDoTiling = false; };

    void                      Set3DMode(bool m) { modeIs3D = m; };
    void                      SetKernelBasedSampling(bool);
    void                      SetJittering(bool);

    void                      SetUpArbitrator(std::string &name, bool min);

  protected:
    int                       width, height, depth;
    int                       currentNode, totalNodes;

    bool                      shouldDoTiling;
    int                       width_min, width_max;
    int                       height_min, height_max;
    bool                      modeIs3D;
    bool                      kernelBasedSampling;
    double                    point_radius;

    bool                      shouldSetUpArbitrator;
    std::string               arbitratorVarName;
    bool                      arbitratorPrefersMinimum;
    avtSamplePointArbitrator *arbitrator;

    avtHexahedronExtractor   *hexExtractor;
    avtHexahedron20Extractor *hex20Extractor;
    avtMassVoxelExtractor    *massVoxelExtractor;
    avtPointExtractor        *pointExtractor;
    avtPyramidExtractor      *pyramidExtractor;
    avtTetrahedronExtractor  *tetExtractor;
    avtWedgeExtractor        *wedgeExtractor;

    bool                      sendCells;
    bool                      jitter;
    avtRayFunction           *rayfoo;

    bool                      rectilinearGridsAreInWorldSpace;
    avtViewInfo               viewInfo;
    double                    aspect;

    virtual void              Execute(void);
    virtual void              PreExecute(void);
    virtual void              PostExecute(void);
    virtual void              ExecuteTree(avtDataTree_p);
    void                      SetUpExtractors(void);

    typedef struct 
    {
      std::vector<int>                  cellDataIndex;
      std::vector<int>                  pointDataIndex;
      std::vector<int>                  cellDataSize;
      std::vector<int>                  pointDataSize;
      std::vector<vtkDataArray *>       cellArrays;
      std::vector<vtkDataArray *>       pointArrays;
      int                               nVars;
    } LoadingInfo;

    inline void               ExtractHex(vtkHexahedron*,vtkDataSet*, int,
                                           LoadingInfo &);
    inline void               ExtractHex20(vtkQuadraticHexahedron*,vtkDataSet*, int,
                                           LoadingInfo &);
    inline void               ExtractVoxel(vtkVoxel *, vtkDataSet *, int,
                                           LoadingInfo &);
    inline void               ExtractTet(vtkTetra *, vtkDataSet *, int,
                                           LoadingInfo &);
    inline void               ExtractPyramid(vtkPyramid *, vtkDataSet *, int,
                                           LoadingInfo &);
    inline void               ExtractWedge(vtkWedge *, vtkDataSet *, int,
                                           LoadingInfo &);
    inline void               ExtractTriangle(vtkTriangle *, vtkDataSet *, int,
                                           LoadingInfo &);
    inline void               ExtractQuad(vtkQuad *, vtkDataSet *, int,
                                           LoadingInfo &);
    inline void               ExtractPixel(vtkPixel *, vtkDataSet *, int, 
                                           LoadingInfo &);

    void                      KernelBasedSample(vtkDataSet *);
    void                      RasterBasedSample(vtkDataSet *);

    virtual bool              FilterUnderstandsTransformedRectMesh();

    void                      GetLoadingInfoForArrays(vtkDataSet *,
                                                      LoadingInfo &);
};


#endif


