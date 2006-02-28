// ************************************************************************* //
//                           avtSamplePointExtractor.h                       //
// ************************************************************************* //

#ifndef AVT_SAMPLE_POINT_EXTRACTOR_H
#define AVT_SAMPLE_POINT_EXTRACTOR_H

#include <filters_exports.h>

#include <avtDatasetToSamplePointsFilter.h>
#include <avtVolume.h>

#include <avtViewInfo.h>

class  vtkHexahedron;
class  vtkPixel;
class  vtkPyramid;
class  vtkQuad;
class  vtkTetra;
class  vtkTriangle;
class  vtkVoxel;
class  vtkWedge;

class  avtHexahedronExtractor;
class  avtMassVoxelExtractor;
class  avtPointExtractor;
class  avtPyramidExtractor;
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

  protected:
    int                       width, height, depth;
    int                       currentNode, totalNodes;

    bool                      shouldDoTiling;
    int                       width_min, width_max;
    int                       height_min, height_max;
    bool                      modeIs3D;
    bool                      kernelBasedSampling;
    double                    point_radius;

    avtHexahedronExtractor   *hexExtractor;
    avtMassVoxelExtractor    *massVoxelExtractor;
    avtPointExtractor        *pointExtractor;
    avtPyramidExtractor      *pyramidExtractor;
    avtTetrahedronExtractor  *tetExtractor;
    avtWedgeExtractor        *wedgeExtractor;

    bool                      sendCells;
    avtRayFunction           *rayfoo;

    bool                      rectilinearGridsAreInWorldSpace;
    avtViewInfo               viewInfo;
    double                    aspect;

    virtual void              Execute(void);
    virtual void              PreExecute(void);
    virtual void              ExecuteTree(avtDataTree_p);
    void                      SetUpExtractors(void);

    inline void               ExtractHex(vtkHexahedron*,vtkDataSet*,int);
    inline void               ExtractVoxel(vtkVoxel *, vtkDataSet *,int);
    inline void               ExtractTet(vtkTetra *, vtkDataSet *, int);
    inline void               ExtractPyramid(vtkPyramid *, vtkDataSet *, int);
    inline void               ExtractWedge(vtkWedge *, vtkDataSet *, int);
    inline void               ExtractTriangle(vtkTriangle *, vtkDataSet *,int);
    inline void               ExtractQuad(vtkQuad *, vtkDataSet *, int);
    inline void               ExtractPixel(vtkPixel *, vtkDataSet *, int);

    void                      KernelBasedSample(vtkDataSet *);
    void                      RasterBasedSample(vtkDataSet *);
};


#endif


