// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtSamplePointExtractorBase.h                       //
// ************************************************************************* //

#ifndef AVT_SAMPLE_POINT_EXTRACTOR_BASE_H
#define AVT_SAMPLE_POINT_EXTRACTOR_BASE_H

#include <filters_exports.h>

#include <avtDatasetToSamplePointsFilter.h>
#include <avtVolume.h>

#include <avtViewInfo.h>

#include <avtOpacityMap.h>
#include <vector>
#include <map>

#include <vtkCamera.h>
#include <vtkMatrix4x4.h>


class  vtkDataArray;
class  vtkDataSet;

class  avtRayFunction;


// ****************************************************************************
//  Class: avtSamplePointExtractorBase
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

class AVTFILTERS_API avtSamplePointExtractorBase 
    : public avtDatasetToSamplePointsFilter
{
  public:
                              avtSamplePointExtractorBase(int, int, int);
    virtual                  ~avtSamplePointExtractorBase();

    virtual const char       *GetType(void)
                                  { return "avtSamplePointExtractorBase"; }
    virtual const char       *GetDescription(void)
                                  { return "Extracting sample points";}

    void                      SetRectilinearGridsAreInWorldSpace(bool, 
                                                   const avtViewInfo &,double);
    void                      RestrictToTile(int, int, int, int);
    void                      StopTiling(void) { shouldDoTiling = false; }


    void                      SetUpArbitrator(std::string &name, bool min);

    void                      SetTransferFn(avtOpacityMap *_transferFn1D)
                                  { transferFn1D = _transferFn1D; }

    void                      SetJittering(bool);

  protected:
    int                       width, height, depth;
    int                       currentNode, totalNodes;

    bool                      shouldDoTiling;
    int                       width_min, width_max;
    int                       height_min, height_max;

    bool                      shouldSetUpArbitrator;
    std::string               arbitratorVarName;
    bool                      arbitratorPrefersMinimum;
    avtSamplePointArbitrator *arbitrator;

    bool                      jitter;

    bool                      rectilinearGridsAreInWorldSpace;
    avtViewInfo               view; // controlled by SetRectilinearGridsAreInWorldSpace
    double                    aspect;

    avtOpacityMap             *transferFn1D;
    virtual void              Execute(void);
    virtual void              ExecuteTree(avtDataTree_p);
    virtual void              PreExecute(void);
    virtual void              PostExecute(void);

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

    void                      GetLoadingInfoForArrays(vtkDataSet *,
                                                      LoadingInfo &);

    virtual bool              FilterUnderstandsTransformedRectMesh() = 0;
    virtual void              SetUpExtractors(void) = 0;
    virtual void              SendJittering(void) = 0;
    virtual void              DoSampling(vtkDataSet *ds, int idx) = 0;
    virtual void              InitSampling(avtDataTree_p dt) {};

};


#endif


