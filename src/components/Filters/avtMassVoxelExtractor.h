// ************************************************************************* //
//                            avtMassVoxelExtractor.h                        //
// ************************************************************************* //

#ifndef AVT_MASS_VOXEL_EXTRACTOR_H
#define AVT_MASS_VOXEL_EXTRACTOR_H

#include <filters_exports.h>

#include <avtExtractor.h>
#include <avtViewInfo.h>

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
// ****************************************************************************

class AVTFILTERS_API avtMassVoxelExtractor : public avtExtractor
{
  public:
                     avtMassVoxelExtractor(int, int, int, avtVolume *,
                                            avtCellList *);
    virtual         ~avtMassVoxelExtractor();

    void             Extract(vtkRectilinearGrid *);

    void             SetGridsAreInWorldSpace(bool, const avtViewInfo &,double);

  protected:
    bool             gridsAreInWorldSpace;
    avtViewInfo      view;
    double           aspect;
    double           cur_clip_range[2];
    vtkMatrix4x4    *view_to_world_transform;
    float           *ProportionSpaceToZBufferSpace;

    float           *X;
    float           *Y;
    float           *Z;
    unsigned char   *ghosts;
    int              dims[3];
    int              ncell_arrays;
    float           *cell_arrays[AVT_VARIABLE_LIMIT];
    int              npt_arrays;
    float           *pt_arrays[AVT_VARIABLE_LIMIT];

    float           *prop_buffer;
    int             *ind_buffer;
    bool            *valid_sample;

    // We repeaated divide by the term (X[i+1]-X[i]).  In the interest of
    // performance, cache the term 1./(X[i+1]-X[i]) and use that for faster
    // multiplication.  This speed up total performance by about 5%.
    float           *divisors_X;
    float           *divisors_Y;
    float           *divisors_Z;

    void             ExtractImageSpaceGrid(vtkRectilinearGrid *);
    void             ExtractWorldSpaceGrid(vtkRectilinearGrid *);

    void             RegisterGrid(vtkRectilinearGrid *);
    void             SampleAlongSegment(const float *, const float*, int, int);
    void             SampleVariable(int, int, int, int);
    bool             FrustumIntersectsGrid(int, int, int, int) const;
    void             GetSegment(int, int, float *, float *) const;
    static void      FindPlaneNormal(const float *, const float *, 
                                     const float *, float *);
    bool             GridOnPlusSideOfPlane(const float *, const float *) const;
    bool             FindSegmentIntersections(const float *, const float *, 
                                              int &, int &);

};


#endif


