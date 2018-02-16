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
