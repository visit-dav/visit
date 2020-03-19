// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtSamplePointExtractor.h                       //
// ************************************************************************* //

#ifndef AVT_SAMPLE_POINT_EXTRACTOR_H
#define AVT_SAMPLE_POINT_EXTRACTOR_H

#include <filters_exports.h>

#include <avtSamplePointExtractorBase.h>

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
class  vtkPolygon;

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
//    Kevin Griffin, Fri Apr 22 16:31:57 PDT 2016
//    Added support for polygons.
//
// ****************************************************************************

class AVTFILTERS_API avtSamplePointExtractor 
    : public avtSamplePointExtractorBase
{
  public:
                              avtSamplePointExtractor(int, int, int);
    virtual                  ~avtSamplePointExtractor();

    virtual const char       *GetType(void)
                                         { return "avtSamplePointExtractor"; }
    virtual const char       *GetDescription(void)
                                         { return "Extracting sample points";}

    void                      RegisterRayFunction(avtRayFunction *rf)
                                         { rayfoo = rf; }
    void                      SendCellsMode(bool);
    void                      Set3DMode(bool m) { modeIs3D = m; }
    void                      SetKernelBasedSampling(bool);
    void                      SetTrilinear(bool t)
                                         { trilinearInterpolation = t; }

  protected:
    bool                      modeIs3D;
    bool                      kernelBasedSampling;
    double                    point_radius;

    avtHexahedronExtractor   *hexExtractor;
    avtHexahedron20Extractor *hex20Extractor;
    avtMassVoxelExtractor    *massVoxelExtractor;
    avtPointExtractor        *pointExtractor;
    avtPyramidExtractor      *pyramidExtractor;
    avtTetrahedronExtractor  *tetExtractor;
    avtWedgeExtractor        *wedgeExtractor;

    bool                      sendCells;
    avtRayFunction           *rayfoo;

    bool                      trilinearInterpolation;

    virtual void              PreExecute(void);
    virtual void              DoSampling(vtkDataSet *, int);
    virtual bool              FilterUnderstandsTransformedRectMesh();
    virtual void              SetUpExtractors(void);
    virtual void              SendJittering(void);

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
    inline void               ExtractPolygon(vtkPolygon *, vtkDataSet *, int,
                                             LoadingInfo &);

    void                      KernelBasedSample(vtkDataSet *);
    void                      RasterBasedSample(vtkDataSet *, int num = 0);
};


#endif


