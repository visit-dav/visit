// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                      avtOSPRaySamplePointExtractor.h                      //
// ************************************************************************* //

#ifndef AVT_OSPRAY_SAMPLE_POINT_EXTRACTOR_H
#define AVT_OSPRAY_SAMPLE_POINT_EXTRACTOR_H

#include <filters_exports.h>

#include <avtSamplePointExtractorBase.h>
#include <avtOSPRayCommon.h> // this ensures VISIT_OSPRAY is defined

class     avtOSPRayVoxelExtractor;

#include <vtkMatrix4x4.h>

#include <vector>
#include <map>

// ****************************************************************************
//  Class: avtOSPRaySamplePointExtractor
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
//    Qi Wu, Sun Jul 1 2018
//    Added support for ospray volume rendering.
//
// ****************************************************************************

class AVTFILTERS_API avtOSPRaySamplePointExtractor 
    : public avtSamplePointExtractorBase
{
  public:
                          avtOSPRaySamplePointExtractor(int, int, int);
    virtual              ~avtOSPRaySamplePointExtractor();

    virtual const char   *GetType(void)
                                   { return "avtOSPRaySamplePointExtractor"; };
    virtual const char   *GetDescription(void)
                                         { return "Extracting sample points";};

    void                  SetOSPRay(OSPVisItContext* o)   { ospray_core = o; };
    void                  SetViewInfo(const avtViewInfo & v) { viewInfo = v; };
    void                  SetSamplingRate(double r)      { samplingRate = r; };
    void                  SetRenderingExtents(int extents[4]) 
    {
        renderingExtents[0] = extents[0];
        renderingExtents[1] = extents[1];
        renderingExtents[2] = extents[2];
        renderingExtents[3] = extents[3];
    }
    void                  SetMVPMatrix(vtkMatrix4x4 *mvp)
    {
        modelViewProj->DeepCopy(mvp);
    };

    int                   GetImgPatchSize() { return patchCount; };
    void                  GetAndDelImgData(int patchId,
                                           ospray::ImgData &tempImgData);
    ospray::ImgMetaData   GetImgMetaPatch(int patchId)
                                  { return imageMetaPatchVector.at(patchId); };
    void                  DelImgPatches();
    
    std::vector<ospray::ImgMetaData>    imageMetaPatchVector;
    std::multimap<int, ospray::ImgData> imgDataHashMap;
    typedef std::multimap<int, ospray::ImgData>::iterator iter_t;

  protected:
    
    virtual void              InitSampling(avtDataTree_p dt);
    virtual void              DoSampling(vtkDataSet *, int);
    virtual void              SetUpExtractors(void);
    virtual void              SendJittering(void);
    virtual bool              FilterUnderstandsTransformedRectMesh(void);
    void                      RasterBasedSample(vtkDataSet *, int num = 0);
    ospray::ImgMetaData       InitMetaPatch(int id);

    OSPVisItContext          *ospray_core;
    avtOSPRayVoxelExtractor  *osprayVoxelExtractor;
    avtViewInfo               viewInfo;
    vtkMatrix4x4             *modelViewProj;
    double                    samplingRate;
    int                       renderingExtents[4];
    int                       patchCount;
};


#endif
