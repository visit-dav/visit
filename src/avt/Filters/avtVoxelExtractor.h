// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.
#ifndef AVT_VOXEL_EXTRACTOR_H
#define AVT_VOXEL_EXTRACTOR_H
#include <filters_exports.h>
#include <avtExtractor.h>
#include <avtViewInfo.h>

class avtVolume;
class avtCellList;
class vtkDataArray;
class vtkMatrix4x4;
class vtkRectilinearGrid;

// ****************************************************************************
// Class: avtVoxelExtractor
//
// Purpose:
//   Base class for mass voxel extractor and the SLIVR version of that class.
//
// Notes:    I split avtMassVoxelExtractor to try and isolate the SLIVR stuff
//           out of it and this is the base class now for avtMassVoxelExtractor
//           and avtSLIVRVoxelExtractor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan  2 14:34:01 PST 2018
//
// Modifications:
//
// ****************************************************************************

class AVTFILTERS_API avtVoxelExtractor : public avtExtractor
{
public:
    avtVoxelExtractor(int w, int h, int d, avtVolume *vol, avtCellList *cl);
    virtual ~avtVoxelExtractor();

    void    SetTrilinear(bool t) {trilinearInterpolation = t;   };
    void    SetGridsAreInWorldSpace(bool, const avtViewInfo &,double,
                                    const double *);

protected:
    bool             gridsAreInWorldSpace;
    bool             pretendGridsAreInWorldSpace;
    avtViewInfo      view;
    double           aspect;
    double           cur_clip_range[2];
    vtkMatrix4x4    *view_to_world_transform;
    vtkMatrix4x4    *world_to_view_transform;
    bool             trilinearInterpolation;

    double           *X;
    double           *Y;
    double           *Z;
    double           *divisors_X;
    double           *divisors_Y;
    double           *divisors_Z;

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
protected:
    static void AssignEight(int vartype, double *vals, const int *index, int s, int m, void *array);
    double ConvertToDouble(int vartype, int index, int s, int m, void *array) const;

    int    FindMatch(const double *A, const double &a, const int &nA) const;
    int    FindIndex(vtkDataArray *coordArray,const double &pt, const int &last_hit,
                     const int &n) const;
    void   FindRange(vtkDataArray *coordArray, int ind, double c, double &min, double &max) const;

    void   computeIndices(int dims[3], int indices[6], int returnIndices[8]) const;
    void   computeIndicesVert(int dims[3], int indices[6], int returnIndices[8]) const;
    double trilinearInterpolate(double vals[8], float distRight, float distTop, float distBack) const;
    void   getIndexandDistFromCenter(float dist, 
                                     int index, int &index_before, int &index_after,
                                     float &dist_before, float &dist_after) const;

    void   RegisterGrid(vtkRectilinearGrid *rgrid,
                        std::vector<std::string> &varorder,
                        std::vector<int> &varsize);
    void   GetSegment(int w, int h, double *origin, double *terminus) const;
    bool   FindSegmentIntersections(const double *origin, 
                                    const double *terminus, int &start, int &end);
    bool   FrustumIntersectsGrid(int, int, int, int) const;
    void   FindPlaneNormal(const double *, const double *, 
                           const double *, double *) const;
    bool   GridOnPlusSideOfPlane(const double *, const double *) const;

    // Same for both subclasses.
    void   ExtractImageSpaceGrid(vtkRectilinearGrid *rgrid,
                                 std::vector<std::string> &varnames,
                                 std::vector<int> &varsizes);
};

#endif
