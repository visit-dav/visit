// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtMassVoxelExtractor.h                        //
// ************************************************************************* //

#ifndef AVT_MASS_VOXEL_EXTRACTOR_H
#define AVT_MASS_VOXEL_EXTRACTOR_H

#include <filters_exports.h>

#include <avtVoxelExtractor.h>

#include <avtOpacityMap.h>

class     vtkRectilinearGrid;
class     vtkMatrix4x4;

#include <stdlib.h>
//#include <stdio.h>
//#include <algorithm>    // std::max

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
// ****************************************************************************

class AVTFILTERS_API avtMassVoxelExtractor : public avtVoxelExtractor
{
  public:
                     avtMassVoxelExtractor(int, int, int, avtVolume *,
                                            avtCellList *);
    virtual         ~avtMassVoxelExtractor();

    void             Extract(vtkRectilinearGrid *,
                             std::vector<std::string> &varnames,
                             std::vector<int> &varsize);

    void             SetOpacityVariableIndex(int val) { opacityVariableIndex = val; }
    void             SetWeightVariableIndex(int val) { weightVariableIndex = val; }

    void             SetTransferFn(avtOpacityMap *_transferFn1D) { transferFn1D = _transferFn1D; };

  protected:
    template <typename Precision>
    struct PrecisionArrays
    {
        PrecisionArrays() : prop_buffer_I(NULL),prop_buffer_J(NULL),prop_buffer_K(NULL),
                            t(NULL)
        {
        }

        ~PrecisionArrays()
        {
            Clear();
        }

        void Allocate(int depth)
        {
            prop_buffer_I   = new Precision[depth];
            prop_buffer_J   = new Precision[depth];
            prop_buffer_K   = new Precision[depth];
            t = new Precision[depth];
        }

        void Clear()
        {
            if(prop_buffer_I != NULL) delete [] prop_buffer_I;
            if(prop_buffer_J != NULL) delete [] prop_buffer_J;
            if(prop_buffer_K != NULL) delete [] prop_buffer_K;

            if(t != NULL) delete [] t;
        }

        Precision *prop_buffer_I;
        Precision *prop_buffer_J;
        Precision *prop_buffer_K;
        Precision *t;
    };

  protected:
    int              opacityVariableIndex;
    int              weightVariableIndex;

    PrecisionArrays<float>  f_arr;
    PrecisionArrays<double> d_arr;

    int             *ind_buffer;
    int             *ind_buffer_I;
    int             *ind_buffer_J;
    int             *ind_buffer_K;
    int             *node_indices;
    double          *node_blend;
 
    int             *span_first;
    int             *span_last;
    double          *span_vals;

    bool            *valid_sample;
    bool             regularGridSpacing;

    avtOpacityMap    *transferFn1D;

protected:
    void             ExtractWorldSpaceGrid(vtkRectilinearGrid *,
                             std::vector<std::string> &varnames,
                             std::vector<int> &varsize);

    void             RegisterGrid(vtkRectilinearGrid *rgrid,
                                  std::vector<std::string> &varorder,
                                  std::vector<int> &varsize);
    bool             ComputeSamples(const double *, const double*, int first, int last);
    bool             ComputeSamples_Regular(const double *, const double*, int first, int last);
    bool             ComputeSamples_Irregular(const double *, const double*, int first, int last);
    void             SampleVariable(int, int, int, int);
    void             SampleVariable_Common(int, int, int, int);
    void             SampleVariable_Trilinear(int, int, int, int);

    void             DetermineGridSpacing();
};

#endif
