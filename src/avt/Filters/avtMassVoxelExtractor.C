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
//                            avtMassVoxelExtractor.C                        //
// ************************************************************************* //

#include <avtMassVoxelExtractor.h>

#include <float.h>

#include <avtAccessor.h>
#include <avtCellList.h>
#include <avtVolume.h>

#include <vtkDataArray.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkTemplateAliasMacro.h>
#include <vtkUnsignedCharArray.h>

#include <DebugStream.h>
#include <StackTimer.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cmath>

#define INLINE   inline
#ifdef _MSC_VER
  #define RESTRICT __restrict
inline double v_trunc(double v)
{
  long iv;
  double rv;
  static double maxlv = (double) LONG_MAX;

  if (v < 0.0)
    {
    v  = v > -maxlv ? v : -maxlv;
    iv = (long) fabs(v);
    rv = -((double) iv);
    }
  else
    {
    v  = v < maxlv ? v : maxlv;
    iv = (long) v;
    rv = (double) iv;
    }

  return(rv);
}
#else
  #define RESTRICT __restrict__
  #define v_trunc trunc
#endif
#define EARLY_RAY_TERMINATION

// ****************************************************************************
//  Method: avtMassVoxelExtractor constructor
//
//  Arguments:
//     w     The number of sample points in the x direction (width).
//     h     The number of sample points in the y direction (height).
//     d     The number of sample points in the z direction (depth).
//     vol   The volume to put samples into.
//     cl    The cell list to put cells whose sampling was deferred.
//
//  Programmer: Hank Childs
//  Creation:   December 14, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Nov 19 14:50:58 PST 2004
//    Initialize gridsAreInWorldSpace.
//
//    Jeremy Meredith, Thu Feb 15 13:11:34 EST 2007
//    Added an ability to extract voxels using the world-space version
//    even when they're really in image space.
//
//    Hank Childs, Wed Aug 27 11:11:28 PDT 2008
//    Initialize spatial coordinates array.
//
//    Hank Childs, Wed Dec 24 11:22:43 PST 2008
//    Remove reference to ProportionSpaceToZBufferSpace data member.
//
//    Kathleen Biagas, Fri Jul 13 09:23:55 PDT 2012
//    Use double instead of float.
//
// ****************************************************************************

avtMassVoxelExtractor::avtMassVoxelExtractor(int w, int h, int d,
                                             avtVolume *vol, avtCellList *cl)
    : avtVoxelExtractor(w, h, d, vol, cl)
{
    opacityVariableIndex = -1;                  // Set to an invalid value.
    weightVariableIndex = -1;                   // Set to an invalid value.

    f_arr.Allocate(depth);
    d_arr.Allocate(depth);

    ind_buffer   = (int*)calloc(depth, sizeof(int));
    ind_buffer_I = (int*)calloc(depth, sizeof(int));
    ind_buffer_J = (int*)calloc(depth, sizeof(int));
    ind_buffer_K = (int*)calloc(depth, sizeof(int));
    node_indices = (int*)calloc(depth * 8, sizeof(int));
    node_blend   = (double*)calloc(depth * 8, sizeof(double));

    span_first  = (int*)calloc(depth, sizeof(int));
    span_last = (int*)calloc(depth, sizeof(int));
    span_vals = (double*)calloc(depth * 8, sizeof(double));

    valid_sample = (bool*)calloc(depth, sizeof(bool));

    regularGridSpacing = false;

    transferFn1D = NULL;
}

// ****************************************************************************
//  Method: avtMassVoxelExtractor destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
//  Modifications:
//
//    Hank Childs, Sun Nov 21 10:35:40 PST 2004
//    Delete the view to world transform.
//
//    Hank Childs, Wed Aug 27 11:10:51 PDT 2008
//    Delete the spatial coordinate arrays.
//
//    Hank Childs, Wed Dec 24 11:22:43 PST 2008
//    Remove reference to ProportionSpaceToZBufferSpace data member.
//
// ****************************************************************************

avtMassVoxelExtractor::~avtMassVoxelExtractor()
{
    if (ind_buffer != NULL)
        free(ind_buffer);
    if (ind_buffer_I != NULL)
        free(ind_buffer_I);
    if (ind_buffer_J != NULL)
        free(ind_buffer_J);
    if (ind_buffer_K != NULL)
        free(ind_buffer_K);

    if (valid_sample != NULL)
        free(valid_sample);

    if(node_indices != NULL)
        free(node_indices);
    if(node_blend != NULL)
        free(node_blend);

    if(span_first != NULL)
        free(span_first);
    if(span_last != NULL)
        free(span_last);
    if(span_vals != NULL)
        free(span_vals);
}

// ****************************************************************************
//  Method: avtMassVoxelExtractor::Extract
//
//  Purpose:
//      Extracts the grid into the sample points.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 13:11:34 EST 2007
//    Added an ability to extract voxels using the world-space version
//    even when they're really in image space.
//
//    Hank Childs, Fri Jun  1 16:40:10 PDT 2007
//    Added support for non-scalars.
//
// ****************************************************************************

void
avtMassVoxelExtractor::Extract(vtkRectilinearGrid *rgrid,
                std::vector<std::string> &varnames, std::vector<int> &varsizes)
{
    StackTimer t0("avtMassVoxelExtractor::Extract");
    if (gridsAreInWorldSpace || pretendGridsAreInWorldSpace)
        ExtractWorldSpaceGrid(rgrid, varnames, varsizes);
    else
        ExtractImageSpaceGrid(rgrid, varnames, varsizes);
}

// ****************************************************************************
//  Method: avtMassVoxelExtractor::ExtractWorldSpaceGrid
//
//  Purpose:
//      Extracts a grid that has already been put into world space.  This case
//      typically corresponds to ray casting.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Dec 10 10:36:40 PST 2004
//    Use the "restricted" screen space.  This will normally be the whole
//    screen space, but if we are tiling, then it will correspond to only
//    the tile.
//
//    Hank Childs, Fri Jun  1 15:45:58 PDT 2007
//    Add support for non-scalars.
//
//    Kathleen Biagas, Fri Jul 13 09:23:55 PDT 2012
//    Use double instead of float.
//
// ****************************************************************************

void
avtMassVoxelExtractor::ExtractWorldSpaceGrid(vtkRectilinearGrid *rgrid,
                 std::vector<std::string> &varnames, std::vector<int> &varsize)
{
    StackTimer t0("avtMassVoxelExtractor::ExtractWorldSpaceGrid");

    //
    // Some of our sampling routines need a chance to pre-process the data.
    // Register the grid here so we can do that.
    //
    RegisterGrid(rgrid, varnames, varsize);

    //
    // Set up a list of ranges to look at.
    //
    const int max_ranges = 100; // this should be bigger than log(max(W,H))
    int width_min[max_ranges];
    int width_max[max_ranges];
    int height_min[max_ranges];
    int height_max[max_ranges];
    int curRange = 0;
    width_min[curRange] = restrictedMinWidth;
    width_max[curRange] = restrictedMaxWidth+1;
    height_min[curRange] = restrictedMinHeight;
    height_max[curRange] = restrictedMaxHeight+1;
    curRange++;

    while (curRange > 0)
    {
        //
        // Determine what range we are dealing with on this iteration.
        //
        curRange--;
        int w_min = width_min[curRange];
        int w_max = width_max[curRange];
        int h_min = height_min[curRange];
        int h_max = height_max[curRange];

        //
        // Let's find out if this range can even intersect the dataset.
        // If not, just skip it.
        //
        if (FrustumIntersectsGrid(w_min, w_max, h_min, h_max))
        {
            int num_rays = (w_max-w_min)*(h_max-h_min);
            if (num_rays > 5)
            {
                //
                // Break the range down into smaller subchunks and recurse.
                //
                int w_mid = (w_min+w_max) / 2;
                int h_mid = (h_min+h_max) / 2;
                width_min[curRange] = w_min; width_max[curRange] = w_mid;
                height_min[curRange] = h_min; height_max[curRange] = h_mid;
                curRange++;
                width_min[curRange] = w_mid; width_max[curRange] = w_max;
                height_min[curRange] = h_min; height_max[curRange] = h_mid;
                curRange++;
                width_min[curRange] = w_min; width_max[curRange] = w_mid;
                height_min[curRange] = h_mid; height_max[curRange] = h_max;
                curRange++;
                width_min[curRange] = w_mid; width_max[curRange] = w_max;
                height_min[curRange] = h_mid; height_max[curRange] = h_max;
                curRange++;
            }
            else
            {
                //
                // We have a small amount of rays, so just evaluate them.
                //
                for (int i = w_min ; i < w_max ; i++)
                    for (int j = h_min ; j < h_max ; j++)
                    {
                        double origin[4];
                        double terminus[4];
                        GetSegment(i, j, origin, terminus);
                        int first = 0, last = 0;
                        if(FindSegmentIntersections(origin, terminus, first, last))
                        {
                            if(ComputeSamples(origin, terminus, first, last))
                            {
                                SampleVariable(first, last, i, j);
                            }
                        }
                    }
            }
        } // FrustumIntersectsGrid
    }
}

// ****************************************************************************
//  Method: avtMassVoxelExtractor::RegisterGrid
//
//  Purpose:
//      Registers a rectilinear grid.  This is called in combination with
//      SampleAlongSegment, which assumes that data members in this class have
//      been set up.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jan  2 15:34:40 PST 2018
//
//  Modifications:
//
// ****************************************************************************

void
avtMassVoxelExtractor::RegisterGrid(vtkRectilinearGrid *rgrid,
                                    std::vector<std::string> &varorder,
                                    std::vector<int> &varsize)
{
    // Call the base class version.
    avtVoxelExtractor::RegisterGrid(rgrid, varorder, varsize);

    DetermineGridSpacing();

    // Compute the t value in depth.
    double inv_depth = 1. / static_cast<double>(depth);
    for (int i = 0 ; i < depth ; i++)
    {
        d_arr.t[i] = double(i) * inv_depth;
        f_arr.t[i] = static_cast<float>(d_arr.t[i]);
    }
}
  
// ****************************************************************************
// Method: avtMassVoxelExtractor::DetermineGridSpacing
//
// Purpose:
//   Determines if the X,Y,Z coordinate array values are evenly spaced.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       We set the regularGridSpacing flag, which determines whether
//             we use regular or irregular grid versions of the sample
//             computations.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 17 15:56:15 PST 2017
//
// Modifications:
//
// ****************************************************************************

inline bool
RegularSpacing(const double *arr, int n)
{
    if(n >= 2)
    {
        double dArr = fabs(arr[1] - arr[0]);
        double epsilon = dArr / 1000.;
        for(int i = 1; i < n-1; ++i)
        {
            double dArr2 = fabs(arr[i+1] - arr[i]);
            if((dArr2 - dArr) > epsilon)
                return false;
        }
    }
    return true;
}

void
avtMassVoxelExtractor::DetermineGridSpacing()
{
    bool xRegular = RegularSpacing(X, dims[0]);
    bool yRegular = RegularSpacing(Y, dims[1]);
    bool zRegular = RegularSpacing(Z, dims[2]);
    regularGridSpacing = xRegular && yRegular && zRegular;
}

// ****************************************************************************
//  Method: avtMassVoxelExtractor::SampleVariable
//
//  Purpose:
//      Actually samples the variable into our temporaray structure.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Jun  1 15:45:58 PDT 2007
//    Add support for non-scalars.
//
//    Kathleen Biagas, Fri Jul 13 09:23:55 PDT 2012
//    Use double instead of float.
//
// ****************************************************************************
void
avtMassVoxelExtractor::SampleVariable(int first, int last, int w, int h)
{
    if(trilinearInterpolation)
        SampleVariable_Trilinear(first, last, w, h);
    else
        SampleVariable_Common(first, last, w, h);
}

// ****************************************************************************
// Method: sv_ComputeNodeIndicesForSpan
//
// Purpose:
//   Computes node indices for a span of samples. Each tuple of 8 contains 
///  node indices for a cell.
//
// Arguments:
//   ind_I : The i component of the IJK cell index.
//   ind_J : The j component of the IJK cell index.
//   ind_K : The k component of the IJK cell index.
//   dims  : The mesh's dimensions (nodes in ijk).
//   node_indices : The buffer to contain the computed node indices.
//   N     : The number of samples to process.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 17 15:08:06 PST 2017
//
// Modifications:
//
// ****************************************************************************

void
sv_ComputeNodeIndicesForSpan(
    const int * RESTRICT ind_I,
    const int * RESTRICT ind_J,
    const int * RESTRICT ind_K,
    int dims[3],
    int * RESTRICT node_indices,
    int N)
{
    int dims01 = dims[0]*dims[1];

    int * RESTRICT index = node_indices;
    for (int i = 0 ; i < N ; i++)
    {
        int k_dims01     = ind_K[i]     * dims01;
        int knext_dims01 = (ind_K[i]+1) * dims01;
        int j_dims0      = ind_J[i]     * dims[0];
        int jnext_dims0  = (ind_J[i]+1) * dims[0];
        int ii           = ind_I[i];
        int iinext       = (ind_I[i]+1);

        index[0] = k_dims01     + j_dims0     + ii;
        index[1] = k_dims01     + j_dims0     + iinext;
        index[2] = k_dims01     + jnext_dims0 + ii;
        index[3] = k_dims01     + jnext_dims0 + iinext;
        index[4] = knext_dims01 + j_dims0     + ii;
        index[5] = knext_dims01 + j_dims0     + iinext;
        index[6] = knext_dims01 + jnext_dims0 + ii;
        index[7] = knext_dims01 + jnext_dims0 + iinext;

        index += 8;
    }
}

// ****************************************************************************
// Method: sv_ComputeNodeBlendForSpan
//
// Purpose:
//   Computes node blending for a span of samples. Each tuple of 8 contains 
//   node blend coefficients that we'd use to combine the nodes for a cell into
//   a single sample value.
//
// Arguments:
//   prop_I : The sample's parametric location in the cell for the X coordinate.
//   prop_J : The sample's parametric location in the cell for the Y coordinate.
//   prop_K : The sample's parametric location in the cell for the Z coordinate.
//   dims  : The mesh's dimensions (nodes in ijk).
//   node_blend : The buffer to contain the computed node blend coefficients.
//   N     : The number of samples to process.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 17 15:08:06 PST 2017
//
// Modifications:
//
// ****************************************************************************

void
sv_ComputeNodeBlendForSpan(
    const double * RESTRICT prop_I,
    const double * RESTRICT prop_J,
    const double * RESTRICT prop_K,
    int dims[3],
    double * RESTRICT node_blend,
    int N)
{
    double * RESTRICT blend = node_blend;
    for (int i = 0 ; i < N ; i++)
    {
        double x_right  = prop_I[i];
        double x_left   = 1.f - prop_I[i];
        double y_top    = prop_J[i];
        double y_bottom = 1.f - prop_J[i];
        double z_back   = prop_K[i];
        double z_front  = 1.f - prop_K[i];

        blend[0] = x_left  * y_bottom * z_front;
        blend[1] = x_right * y_bottom * z_front;
        blend[2] = x_left  * y_top    * z_front;
        blend[3] = x_right * y_top    * z_front;
        blend[4] = x_left  * y_bottom * z_back;
        blend[5] = x_right * y_bottom * z_back;
        blend[6] = x_left  * y_top    * z_back;
        blend[7] = x_right * y_top    * z_back;

        blend += 8;
    }
}

#ifdef EARLY_RAY_TERMINATION
// ****************************************************************************
// Method: sv_SamplePointOpacityVariable_ERT
//
// Purpose:
//   Samples a point-centered opacity variable along the ray, accumulating the 
//   alpha, so we can terminate early.
//
// Arguments:
//   index    : The node index array.
//   coeff    : The node blending coefficients array.
//   pt_index : The index of the point variable.
//   data     : The data for the point variable.
//   N        : The number of samples to process.
//   transferFn1D : The opacity map we'll use to map scalar to opacity.
//   sampleDist : The distance between each sample on a ray. 
//   tmpSampleList : The samples to store (output)
//   ii_           : The sample index (from 0) where we terminated.
//   opacity       : The accumulated opacity.
//
// Returns:    True if we terminate early. False otherwise.
//
// Note:       We call this function from a vtk type macro so we can efficiently
//             access the data
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 17 15:14:24 PST 2017
//
// Modifications:
//
//     Alister Maguire, Mon Jun  3 15:22:32 PDT 2019
//     Changed oneSamplesContribution to sampleDist and updated the opacity
//     to use a standard opacity correction
//
// ****************************************************************************

template <typename T>
bool
sv_SamplePointOpacityVariable_ERT(
    const int * RESTRICT index,
    const double *RESTRICT coeff,
    int pt_index,
    const T *data,
    int N,
    const avtOpacityMap *transferFn1D,
    float sampleDist,
    double (*tmpSampleList)[AVT_VARIABLE_LIMIT],
    int &ii_,
    float &opacity)
{
    const float threshold = 254.f/255.f;

    for(int ii = 0; ii < N; ++ii)
    {
        // Get the point values for this sample.
        double vals[8];
        for(int pid = 0; pid < 8; ++pid)
            vals[pid] = static_cast<double>(data[index[pid]]);

        // Blend the point values to get the sample value.
        double val = 
            coeff[0] * vals[0] +
            coeff[1] * vals[1] +
            coeff[2] * vals[2] +
            coeff[3] * vals[3] +
            coeff[4] * vals[4] +
            coeff[5] * vals[5] +
            coeff[6] * vals[6] +
            coeff[7] * vals[7];

        // Save the computed value in case we need it.
        tmpSampleList[ii][pt_index] = val;

        // Query the transfer function to get the alpha 
        // for the sample point.
        float alpha = transferFn1D->QueryAlpha(val);

        if (alpha > 0)
        {
            float samplesOpacity = 1.0;
            if (alpha < 1.0)
            { 
                samplesOpacity = (1.0 - std::pow((1.0 - alpha), sampleDist));
            }
            float ff = (1.f-opacity)*samplesOpacity;
            opacity = opacity + ff;
        }
        bool early_term = (opacity > threshold);
        if(early_term)
        {
            ii_ = ii;
            return true;
        }

        // to the next point.
        index += 8;
        coeff += 8;
    }

    return false;
}

// ****************************************************************************
// Method: sv_SampleCellOpacityVariable_ERT
//
// Purpose:
//   Samples a cell-centered opacity variable along the ray, accumulating the 
//   alpha, so we can terminate early.
//
// Arguments:
//   index    : The node index array.
//   coeff    : The node blending coefficients array.
//   pt_index : The index of the point variable.
//   data     : The data for the point variable.
//   N        : The number of samples to process.
//   transferFn1D : The opacity map we'll use to map scalar to opacity.
//   sampleDist : The distance between each sample on a ray. 
//   tmpSampleList : The samples to store (output)
//   ii_           : The sample index (from 0) where we terminated.
//   opacity       : The accumulated opacity.
//
// Returns:    True if we terminate early. False otherwise.
//
// Note:       We call this function from a vtk type macro so we can efficiently
//             access the data
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 17 15:14:24 PST 2017
//
// Modifications:
//
//     Alister Maguire, Mon Jun  3 15:22:32 PDT 2019
//     Changed oneSamplesContribution to sampleDist and updated the opacity
//     to use a standard opacity correction
//
// ****************************************************************************

template <typename T>
bool
sv_SampleCellOpacityVariable_ERT(
    const int * RESTRICT index,
    int cell_index,
    const T *data,
    int N,
    const avtOpacityMap *transferFn1D,
    float sampleDist,
    double (*tmpSampleList)[AVT_VARIABLE_LIMIT],
    int &ii_,
    float &opacity)
{
    const float threshold = 254.f/255.f;

    for(int ii = 0; ii < N; ++ii)
    {
        // Get the value for this sample.
        double val = static_cast<double>(data[index[ii]]);

        // Save the computed value in case we need it.
        tmpSampleList[ii][cell_index] = val;

        // Query the transfer function to get the alpha 
        // for the sample point.
        float alpha = transferFn1D->QueryAlpha(val);

        if (alpha > 0)
        {
            float samplesOpacity = 1.f;
            if (alpha < 1.0)
            { 
                samplesOpacity = (1.0 - std::pow((1.0 - alpha), sampleDist));
            }

            float ff = (1-opacity)*samplesOpacity;
            opacity = opacity + ff;
        }
        bool early_term = (opacity > threshold);
        if(early_term)
        {
            ii_ = ii;
            return true;
        }
    }

    return false;
}
#endif

// ****************************************************************************
// Method: SampleVariable_Common
//
// Purpose:
//   Samples along a ray.
//
// Arguments:
//   first : The index of the first sample along the ray.
//   last  : The index of the last sample along the ray.
//   w     : The x coordinate of the ray in the image.
//   h     : The y coordinate of the ray in the image.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 17 14:56:38 PST 2017
//
// Modifications:
//
//     Alister Maguire, Mon Jun  3 15:40:31 PDT 2019
//     Replaced oneSamplesContribution with a standard opacity correction. 
//
// ****************************************************************************

void
avtMassVoxelExtractor::SampleVariable_Common(int first, int last, int w, int h)
{
    // Get a ray object for this set of samples.
    avtRay *ray = volume->GetRay(w, h);

    //
    // Calculate cell indices and ghost some samples if needed.
    //
    bool calc_cell_index = ((ncell_arrays > 0) || (ghosts != NULL));
    int cdims[3];
    cdims[0] = dims[0]-1;
    cdims[1] = dims[1]-1;
    cdims[2] = dims[2]-1;
    int dims01 = dims[0]*dims[1];
    if(calc_cell_index)
    {
        // Compute the cell index if we need it.
        int nxny = cdims[0]*cdims[1];
        for (int i = first ; i < last ; i++)
        {
            ind_buffer[i] = ind_buffer_K[i] * nxny + 
                            ind_buffer_J[i] * cdims[0] +
                            ind_buffer_I[i];
        }

        // ghost out some samples. Some samples might be invalid already.
        if (ghosts != NULL)
        {
            for (int i = first ; i < last ; i++)
            {
                if(valid_sample[i])
                    valid_sample[i] = (ghosts[ind_buffer[i]] == 0);
            }
        }
    }

    //
    // Scan through the valid_sample array to produce list of spans to process.
    //
    bool inrun = false;
    int nspans = 0;
    for(int i = first; i < last; ++i)
    {
        if(valid_sample[i])
        {
            if(!inrun)
            {
                // start new run.
                span_first[nspans] = i;
            }
            inrun = true;
        }
        else
        {
            if(inrun)
            {
                span_last[nspans] = i-1; // close out the run.
                nspans++;
            }

            inrun = false;
        }
    }
    if(inrun)
    {
        span_last[nspans] = last-1; // close out the run.
        nspans++;
    }

#ifdef EARLY_RAY_TERMINATION
    //
    // We need to calculate the sample distance so that we can apply
    // opacity correction: 
    // 1 - ((1 - alpha(x))**sampleDist)
    //
    float viewDist = float((view.farPlane - view.nearPlane));
    float sampleDist = viewDist / float(ray->GetNumberOfSamples());
    float opacity = 0.f;
#endif

    //
    // Look through the cell and point variables to see which we need to process
    // for early ray termination. Note, our opacityVariableIndex will only be set
    // when we're doing a compositing ray function.
    //
    int cell_var_handled = -1;
    int pt_var_handled = -1;
#ifdef EARLY_RAY_TERMINATION
    // If we're weighting opacity by another variable then we can't do ERT.
    if(weightVariableIndex == -1)
    {
        for(int i = 0; i < npt_arrays; ++i)
        {
            if(pt_index[i] == opacityVariableIndex)
            {
                pt_var_handled = i;
                break;
            }
        }
        for(int i = 0; i < ncell_arrays; ++i)
        {
            if(cell_index[i] == opacityVariableIndex)
            {
                cell_var_handled = i;
                break;
            }
        }
    }
#endif

    //
    // Now, process each span.
    //
    for(int si = 0; si < nspans; ++si)
    {
        int N = span_last[si] - span_first[si] + 1;
        int N8 = N*8;

        if(npt_arrays > 0)
        {
            // Compute node indices and blending values just for this span.
            sv_ComputeNodeIndicesForSpan(
                ind_buffer_I + span_first[si],
                ind_buffer_J + span_first[si],
                ind_buffer_K + span_first[si],
                dims, 
                node_indices,
                N);
            sv_ComputeNodeBlendForSpan(
                d_arr.prop_buffer_I + span_first[si],
                d_arr.prop_buffer_J + span_first[si],
                d_arr.prop_buffer_K + span_first[si],
                dims,
                node_blend,
                N);
        }

        // Extract point data for the opacity variable first so we can do
        // early ray termination.
#ifdef EARLY_RAY_TERMINATION
        if(transferFn1D != NULL)
        {
            if(pt_var_handled >= 0)
            {
                // The opacity variable is point-centered.
                void  *pt_array = pt_arrays[pt_var_handled];
                int    s = pt_size[pt_var_handled];
                int ii = 0;
                const int *index = node_indices;
                const double *coeff = node_blend;
                switch(pt_vartypes[pt_var_handled])
                {
                vtkTemplateMacro(
                    if(sv_SamplePointOpacityVariable_ERT(index, coeff, pt_index[pt_var_handled],
                       (const VTK_TT *)pt_array, N, transferFn1D, sampleDist,
                       tmpSampleList, ii, opacity))
                    {
                        // We terminated early so trim the span.
                        span_last[si] = span_first[si]+ii;
                        N = ii+1;
                        N8 = N*8;
                        nspans = 0; 
                    }
                );
                }
            }
            if(cell_var_handled >= 0)
            {
                // The opacity variable is cell-centered.
                void  *c_array = cell_arrays[cell_var_handled];
                int    s = cell_size[cell_var_handled];
                int ii = 0;
                const int *index = ind_buffer + span_first[si];
                switch(cell_vartypes[cell_var_handled])
                {
                vtkTemplateMacro(
                    if(sv_SampleCellOpacityVariable_ERT(index, cell_index[cell_var_handled],
                       (const VTK_TT *)c_array, N, transferFn1D, sampleDist,
                       tmpSampleList, ii, opacity))
                    {
                        // We terminated early so trim the span.
                        span_last[si] = span_first[si]+ii;
                        N = ii+1;
                        N8 = N*8;
                        nspans = 0; 
                    }
                );
                }
            }
        }
#endif

        // Extract point data for this span.
        for (int l = 0 ; l < npt_arrays ; l++)
        {
            // If we already did this variable, skip it.
            if(l == pt_var_handled)
                continue;

            void  *pt_array = pt_arrays[l];
            int    s = pt_size[l];

            // Extract all data values for this array+span.
            if(s == 1)
            {
                const int *index = node_indices;
                const double *coeff = node_blend;

                // Extract the 8 node values for samples in this span.
                switch(pt_vartypes[l])
                {
                vtkTemplateMacro({
                    const VTK_TT *data = (const VTK_TT *)pt_array;
                    for(int ii = 0; ii < N8; ++ii)
                        span_vals[ii] = data[index[ii]];
                });
                }

                // Compute the blended value for each sample in the span.
                const double *vals = span_vals;
                for(int v = 0; v < N; ++v)
                {
                    double val = 
                      coeff[0] * vals[0] +
                      coeff[1] * vals[1] +
                      coeff[2] * vals[2] +
                      coeff[3] * vals[3] +
                      coeff[4] * vals[4] +
                      coeff[5] * vals[5] +
                      coeff[6] * vals[6] +
                      coeff[7] * vals[7];

                    tmpSampleList[v][pt_index[l]] = val;

                    coeff += 8;
                    vals += 8;
                }
            }
            else
            {
                // Handle multi-component data.
                for(int m = 0; m < s; ++m)
                {
                    const int *index = node_indices;
                    const double *coeff = node_blend;

                    // Extract the 8 node values for samples in this span.
                    switch(pt_vartypes[l])
                    {
                    vtkTemplateMacro({
                        const VTK_TT *data = (const VTK_TT *)pt_array;
                        for(int ii = 0; ii < N8; ++ii)
                            span_vals[ii] = data[s*index[ii]+m];
                    });
                    }

                    // Compute the blended value for each sample in the span.
                    const double *vals = span_vals;
                    for(int v = 0; v < N; ++v)
                    {
                        double val = 
                            coeff[0] * vals[0] +
                            coeff[1] * vals[1] +
                            coeff[2] * vals[2] +
                            coeff[3] * vals[3] +
                            coeff[4] * vals[4] +
                            coeff[5] * vals[5] +
                            coeff[6] * vals[6] +
                            coeff[7] * vals[7];

// TODO: I could convert this to a strided memory access. Would that be better?
                        tmpSampleList[v][pt_index[l]+m] = val;

                        coeff += 8;
                        vals += 8;
                    }
                }
            }
        }

        // Extract cell data for this span.
        for (int l = 0 ; l < ncell_arrays ; l++)
        {
            // If we already did this variable, skip it.
            if(l == cell_var_handled)
                continue;

            const int *index = ind_buffer + span_first[si];
            int s = cell_size[l];
            void *c_array = cell_arrays[l];
            switch(cell_vartypes[l])
            {
            vtkTemplateMacro(
                if(s == 1)
                {
                    const VTK_TT *data = (const VTK_TT *)c_array;
                    for(int ii = 0; ii < N; ++ii)
                        tmpSampleList[ii][cell_index[l]] = data[index[ii]];
                }
                else
                {
                    const VTK_TT *data = (const VTK_TT *)c_array;
                    for(int ii = 0; ii < N; ++ii)
                    {
                        int ti = s*index[ii];
                        for(int m = 0; m < s; ++m)
                            tmpSampleList[ii][cell_index[l]+m] = data[ti+m];
                    }
                }
            );
            }
        }

        // Now that we have our span fully constructed, add it to the ray.
        ray->SetSamples(span_first[si], span_last[si], tmpSampleList);
    }
}

// ****************************************************************************
// Method: SampleVariable_Trilinear
//
// Purpose:
//   Samples along a ray using trilinear.
//
// Arguments:
//   first : The index of the first sample along the ray.
//   last  : The index of the last sample along the ray.
//   w     : The x coordinate of the ray in the image.
//   h     : The y coordinate of the ray in the image.
//
// Returns:    
//
// Note:       Taken from original SampleVariable.
//
// Programmer: VisIt Team
// Creation:   Fri Feb 17 14:58:18 PST 2017
//
// Modifications:
//   Brad Whitlock, Fri Feb 17 14:58:33 PST 2017
//   I broke this code out into its own function so we can just do the 
//   trilinear case. This eliminates a lot of ifs. I did not replace 
//   inefficient stuff.
//
// ****************************************************************************

void
avtMassVoxelExtractor::SampleVariable_Trilinear(int first, int last, int w, int h)
{
    bool inrun = false;
    int  count = 0;

    avtRay *ray = volume->GetRay(w, h);

    bool calc_cell_index = ((ncell_arrays > 0) || (ghosts != NULL));
    int cdims[3];
    cdims[0] = dims[0]-1;
    cdims[1] = dims[1]-1;
    cdims[2] = dims[2]-1;
    if(calc_cell_index)
    {
        // Compute thee cell index if we need it.
        int nxny = cdims[0]*cdims[1];
        for (int i = first ; i < last ; i++)
        {
            ind_buffer[i] = ind_buffer_K[i] * nxny + 
                            ind_buffer_J[i] * cdims[0] +
                            ind_buffer_I[i];
        }

        // ghost out some samples.
        if (ghosts != NULL)
        {
            for (int i = first ; i < last ; i++)
            {
                if(valid_sample[i])
                    valid_sample[i] = (ghosts[ind_buffer[i]] == 0);
            }
        }
    }

    for (int i = first ; i < last ; i++)
    {
        const int *ind_I = ind_buffer_I + i;
        const int *ind_J = ind_buffer_J + i;
        const int *ind_K = ind_buffer_K + i;

        const double *prop_I = d_arr.prop_buffer_I + i;
        const double *prop_J = d_arr.prop_buffer_J + i;
        const double *prop_K = d_arr.prop_buffer_K + i;

        if (!valid_sample[i] && inrun)
        {
            ray->SetSamples(i-count, i-1, tmpSampleList);
            inrun = false;
            count = 0;
        }

        int index_left, index_right,            index_top, index_bottom,         index_front, index_back;
        float dist_from_left, dist_from_right,  dist_from_top,dist_from_bottom,  dist_from_front, dist_from_back;

        int newInd[3];
        newInd[0] = *ind_I;
        newInd[1] = *ind_J;
        newInd[2] = *ind_K;

        float x_right = *prop_I;       float x_left = 1. - x_right;    
        float y_top = *prop_J;         float y_bottom = 1. - y_top;   
        float z_back = *prop_K;        float z_front = 1. - z_back;   

        // get the index and distance from the center of the neighbouring cells
        getIndexandDistFromCenter(x_right, newInd[0], index_left, index_right,   dist_from_left, dist_from_right);
        getIndexandDistFromCenter(y_top,   newInd[1], index_bottom,index_top,    dist_from_bottom,dist_from_top);
        getIndexandDistFromCenter(z_back,  newInd[2], index_front, index_back,   dist_from_front, dist_from_back);

        int indices[6];
        indices[4] = index_front;       indices[5] = index_back;
        indices[2] = index_bottom;      indices[3] = index_top;
        indices[0] = index_left;        indices[1] = index_right;

        // Constrain the samples more.
        if (indices[0] < 0 || indices[0]>dims[0]-2)
            valid_sample[i] = false;

        if (indices[1] < 0 || indices[1]>dims[0]-2)
            valid_sample[i] = false;


        if (indices[2] < 0 || indices[2]>dims[1]-2)
            valid_sample[i] = false;

        if (indices[3] < 0 || indices[3]>dims[1]-2)
            valid_sample[i] = false;


        if (indices[4] < 0 || indices[4]>dims[2]-2)
            valid_sample[i] = false;

        if (indices[5] < 0 || indices[5]>dims[2]-2)
            valid_sample[i] = false;

        if (!valid_sample[i]){
            continue;
        }

        //
        // Cell centered data
        //
        if (ncell_arrays > 0)
        {
            int indexT[8];
            computeIndices(dims, indices, indexT);
                
            for (int l = 0 ; l < ncell_arrays ; l++) // ncell_arrays: usually 1
            {
                void  *cell_array = cell_arrays[l];
                double values[8];
                for (int m = 0 ; m < cell_size[l] ; m++)
                {   // cell_size[l] usually 1
                    AssignEight(cell_vartypes[l], values, indexT, cell_size[l], m, cell_array);         
                    double scalarValue = trilinearInterpolate(values, dist_from_left, dist_from_bottom, dist_from_front);

                    tmpSampleList[count][cell_index[l]+m] = scalarValue; 
                }
            }
        }

        //
        // Node centered data
        //
        if (npt_arrays > 0)
        {
            int indexT[8];
            computeIndicesVert(dims, indices, indexT);

            for (int l = 0 ; l < npt_arrays ; l++)
            {
                void  *pt_array = pt_arrays[l];
                double values[8];
                for (int m = 0 ; m < pt_size[l] ; m++)
                {
                    AssignEight(pt_vartypes[l], values, indexT, pt_size[l], m, pt_array);
                    double scalarValue = trilinearInterpolate(values, x_left, y_bottom, z_front);

                    tmpSampleList[count][pt_index[l]+m] = scalarValue;
                }
            }
        }

        inrun = true;
        count++;
    }

    //
    // Make sure we get runs at the end.
    //
    if (inrun)
        ray->SetSamples(last-count, last-1, tmpSampleList);
}

// ****************************************************************************
// Method: sas_ComputeIndices
//
// Purpose:
//   Compute the cell and prop values for the samples that intersect the mesh.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 17 16:10:08 PST 2017
//
// Modifications:
//
// ****************************************************************************

template <typename Precision>
static void
sas_ComputeIndices(int       * RESTRICT ind,
                   Precision * RESTRICT dProp,
                   const Precision * RESTRICT t,
                   Precision origin, Precision dist,
                   int first, int last,
                   Precision minExtents, 
                   Precision inv_cellSize)
{
    int N = last - first;
    for (int i = 0 ; i < N ; i++)
    {
        Precision coord = origin + t[i]*dist;

        // The fractional cell index (assumes all cells same size).
        Precision fc = (coord - minExtents) * inv_cellSize;

        // Separate fractional cell index into int,frac parts.
        Precision ipart = trunc(fc);
        dProp[i] = fc - ipart;

        // Save the int part.
        ind[i] = static_cast<int>(ipart);
    }
}

// ****************************************************************************
// Method: sas_SetValidity
//
// Purpose:
//   Examines the IJK values that we computed and makes sure they are within 
//   the mesh. Set the valid_sample values appropriately.
//
// Arguments:
//   
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 17 16:10:52 PST 2017
//
// Modifications:
//
// ****************************************************************************

static bool
sas_SetValidity(bool *      RESTRICT valid_sample,
                const int * RESTRICT ind_I,
                const int * RESTRICT ind_J,
                const int * RESTRICT ind_K,
                const int cdims[3],
                int N)
{
    for (int i = 0 ; i < N ; i++)
    {
        valid_sample[i] = (ind_I[i] >= 0 && ind_I[i] < cdims[0]) &&
                          (ind_J[i] >= 0 && ind_J[i] < cdims[1]) &&
                          (ind_K[i] >= 0 && ind_K[i] < cdims[2]);
    }

    bool hasSamples = false;
    for (int i = 0 ; i < N && !hasSamples; i++)
        hasSamples |= valid_sample[i];

    return hasSamples;
}
   
// ****************************************************************************
// Method: avtMassVoxelExtractor::ComputeSamples_Regular
//
// Purpose:
//   Computes samples along the ray, computing the cell IJK indices and the
//   "prop" IJK values that are the sample's parametric location within the cell.
//
// Arguments:
//   origin   : The origin of the sample ray.
//   terminus : The end of the sample ray.
//   first    : The first sample along the ray that we need to consider.
//   last     : The last sample along the ray that we need to consider.
//
// Returns:    True if there are samples; False otherwise.
//
// Note:       This method assumes that all cells are the same size so we can
//             more easily compute the cell IJK index.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 17 16:05:35 PST 2017
//
// Modifications:
//
// ****************************************************************************

bool
avtMassVoxelExtractor::ComputeSamples_Regular(const double *origin, 
    const double *terminus, int first, int last)
{
    // in between these values, compute with float.
    const double minvalue = 0.00001;
    const double maxvalue = 1000000.;

    double x_dist = (terminus[0]-origin[0]);
    double y_dist = (terminus[1]-origin[1]);
    double z_dist = (terminus[2]-origin[2]);

    if(x_dist > minvalue && x_dist < maxvalue)
    {
        float origin0 = static_cast<float>(origin[0]);
        float dist = static_cast<float>(x_dist);
        float minX = static_cast<float>(X[0]);
        float invCellSizeX = static_cast<float>(divisors_X[0]);

        // Compute the cell indices and fractional indices from the sample points.
        sas_ComputeIndices(ind_buffer_I + first,
                           f_arr.prop_buffer_I + first,
                           f_arr.t + first,
                           origin0, dist,
                           first, last, 
                           minX, invCellSizeX);

        // We have filled in prop_buffer_I store it in the double version.
        for(int i = first; i < last; ++i)
            d_arr.prop_buffer_I[i] = static_cast<double>(f_arr.prop_buffer_I[i]);
    }
    else
    {
        double minX = X[0];
        double invCellSizeX = divisors_X[0];
        sas_ComputeIndices(ind_buffer_I + first,
                           d_arr.prop_buffer_I + first,
                           d_arr.t + first,
                           origin[0], x_dist,
                           first, last, 
                           minX, invCellSizeX);
    }

    if(y_dist > minvalue && y_dist < maxvalue)
    {
        float origin1 = static_cast<float>(origin[1]);
        float dist = static_cast<float>(y_dist);
        float minY = static_cast<float>(Y[0]);
        float invCellSizeY = static_cast<float>(divisors_Y[0]);

        // Compute the cell indices and fractional indices from the sample points.
        sas_ComputeIndices(ind_buffer_J + first,
                           f_arr.prop_buffer_J + first,
                           f_arr.t + first,
                           origin1, dist,
                           first, last, 
                           minY, invCellSizeY);

        // We have filled in prop_buffer_J store it in the double version.
        for(int i = first; i < last; ++i)
            d_arr.prop_buffer_J[i] = static_cast<double>(f_arr.prop_buffer_J[i]);
    }
    else
    {
        double minY = Y[0];
        double invCellSizeY = divisors_Y[0];
        sas_ComputeIndices(ind_buffer_J + first,
                           d_arr.prop_buffer_J + first,
                           d_arr.t + first,
                           origin[1], y_dist,
                           first, last, 
                           minY, invCellSizeY);
    }

    if(z_dist > minvalue && z_dist < maxvalue)
    {
        float origin2 = static_cast<float>(origin[2]);
        float dist = static_cast<float>(z_dist);
        float minZ = static_cast<float>(Z[0]);
        float invCellSizeZ = static_cast<float>(divisors_Z[0]);

        // Compute the cell indices and fractional indices from the sample points.
        sas_ComputeIndices(ind_buffer_K + first,
                           f_arr.prop_buffer_K + first,
                           f_arr.t + first,
                           origin2, dist,
                           first, last, 
                           minZ, invCellSizeZ);

        // We have filled in prop_buffer_J store it in the double version.
        for(int i = first; i < last; ++i)
            d_arr.prop_buffer_K[i] = static_cast<double>(f_arr.prop_buffer_K[i]);
    }
    else
    {
        double minZ = Z[0];
        double invCellSizeZ = divisors_Z[0];
        sas_ComputeIndices(ind_buffer_K + first,
                           d_arr.prop_buffer_K + first,
                           d_arr.t + first,
                           origin[2], z_dist,
                           first, last, 
                           minZ, invCellSizeZ);
    }

    // Look through the samples and set whether they are valid based in
    // the ind[] values.
    int cdims[3];
    cdims[0] = dims[0]-1;
    cdims[1] = dims[1]-1;
    cdims[2] = dims[2]-1;
    bool hasSamples = sas_SetValidity(valid_sample + first,
                                      ind_buffer_I + first,
                                      ind_buffer_J + first, 
                                      ind_buffer_K + first,
                                      cdims, last-first);
    return hasSamples;
}

// ****************************************************************************
//  Method: avtMassVoxelExtractor::ComputeSamples_Irregular
//
//  Purpose:
//      Creates samples for the grid along a line segment.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2004
//
//  Modifications:
//    Hank Childs, Tue Jan  3 17:26:11 PST 2006
//    Fix bug that ultimately led to UMR where sampling occurred along 
//    invalid values.
//
//    Hank Childs, Wed Dec 24 11:22:43 PST 2008
//    No longer use the ProportionSpaceToZBufferSpace data member, as we now 
//    do our sampling in even intervals (wbuffer).
//
//    Kathleen Biagas, Fri Jul 13 09:23:55 PDT 2012
//    Use double instead of float.
//
// ****************************************************************************

bool
avtMassVoxelExtractor::ComputeSamples_Irregular(const double *origin, 
    const double *terminus, 
    int first, int last)
{
    bool foundHit = false;
    int curX = -1;
    int curY = -1;
    int curZ = -1;
    bool xGoingUp = (terminus[0] > origin[0]);
    bool yGoingUp = (terminus[1] > origin[1]);
    bool zGoingUp = (terminus[2] > origin[2]);

    double x_dist = (terminus[0]-origin[0]);
    double y_dist = (terminus[1]-origin[1]);
    double z_dist = (terminus[2]-origin[2]);

    double pt[3];
    bool hasSamples = false;

    for (int i = first ; i < last ; i++)
    {
        valid_sample[i] = false;

        double proportion = ((double)i)/((double)depth);
        pt[0] = origin[0] + proportion*x_dist;
        pt[1] = origin[1] + proportion*y_dist;
        pt[2] = origin[2] + proportion*z_dist;

        ind_buffer_I[i] = -1;
        ind_buffer_J[i] = -1;
        ind_buffer_K[i] = -1;

        if (!foundHit)
        {
            //
            // We haven't found any hits previously.  Exhaustively search
            // through arrays and try to find a hit.
            //
            ind_buffer_I[i] = FindMatch(X, pt[0], dims[0]);
            if (ind_buffer_I[i] >= 0)
                d_arr.prop_buffer_I[i] = (pt[0] - X[ind_buffer_I[i]]) * divisors_X[ind_buffer_I[i]];
            ind_buffer_J[i] = FindMatch(Y, pt[1], dims[1]);
            if (ind_buffer_J[i] >= 0)
                d_arr.prop_buffer_J[i] = (pt[1] - Y[ind_buffer_J[i]]) * divisors_Y[ind_buffer_J[i]];
            ind_buffer_K[i] = FindMatch(Z, pt[2], dims[2]);
            if (ind_buffer_K[i] >= 0)
                d_arr.prop_buffer_K[i] = (pt[2] - Z[ind_buffer_K[i]]) * divisors_Z[ind_buffer_K[i]];
        }
        else
        {
            //
            // We have found a hit before.  Try to locate the next sample 
            // based on what we already found.
            //
            if (xGoingUp)
            {
                for ( ; curX < dims[0]-1 ; curX++)
                {
                    if (pt[0] >= X[curX] && pt[0] <= X[curX+1])
                    {
                        d_arr.prop_buffer_I[i] = (pt[0] - X[curX]) * divisors_X[curX];
                        ind_buffer_I[i] = curX;
                        break;
                    }
                }
            }
            else
            {
                for ( ; curX >= 0 ; curX--)
                {
                    if (pt[0] >= X[curX] && pt[0] <= X[curX+1])
                    {
                        d_arr.prop_buffer_I[i] = (pt[0] - X[curX]) * divisors_X[curX];
                        ind_buffer_I[i] = curX;
                        break;
                    }
                }
            }
            if (yGoingUp)
            {
                for ( ; curY < dims[1]-1 ; curY++)
                {
                    if (pt[1] >= Y[curY] && pt[1] <= Y[curY+1])
                    {
                        d_arr.prop_buffer_J[i] = (pt[1] - Y[curY]) * divisors_Y[curY];
                        ind_buffer_J[i] = curY;
                        break;
                    }
                }
            }
            else
            {
                for ( ; curY >= 0 ; curY--)
                {
                    if (pt[1] >= Y[curY] && pt[1] <= Y[curY+1])
                    {
                        d_arr.prop_buffer_J[i] = (pt[1] - Y[curY]) * divisors_Y[curY];
                        ind_buffer_J[i] = curY;
                        break;
                    }
                }
            }
            if (zGoingUp)
            {
                for ( ; curZ < dims[2]-1 ; curZ++)
                {
                    if (pt[2] >= Z[curZ] && pt[2] <= Z[curZ+1])
                    {
                        d_arr.prop_buffer_K[i] = (pt[2] - Z[curZ]) * divisors_Z[curZ];
                        ind_buffer_K[i] = curZ;
                        break;
                    }
                }
            }
            else
            {
                for ( ; curZ >= 0 ; curZ--)
                {
                    if (pt[2] >= Z[curZ] && pt[2] <= Z[curZ+1])
                    {
                        d_arr.prop_buffer_K[i] = (pt[2] - Z[curZ]) * divisors_Z[curZ];
                        ind_buffer_K[i] = curZ;
                        break;
                    }
                }
            }
        }

        bool intersectedDataset = !(ind_buffer_I[i] < 0 || ind_buffer_J[i] < 0 || ind_buffer_K[i] < 0);
        if (!intersectedDataset)
        {
            if (!foundHit) 
            {
                // We still haven't found the start.  Keep looking.
                continue;
            }
            else
            {
                // This is the true terminus.
                last = i;
                break;
            }
        }
        else  // Did intersect data set.
        {
            if (!foundHit)
            {
                // This is the first true sample.  "The true start"
                first = i;
            }
        }

        valid_sample[i] = true;
        foundHit = true;
        hasSamples = true;

        curX = ind_buffer_I[i];
        curY = ind_buffer_J[i];
        curZ = ind_buffer_K[i];
    }

    return hasSamples;
}

// ****************************************************************************
//  Method: avtMassVoxelExtractor::ComputeSamples
//
//  Purpose:
//      Creates samples for the grid along a line segment.
//
//  Arguments:
//     origin   : The start coordinate of the sampling ray.
//     terminus : The end coordinate of the sampling ray.
//     first    : The index of the first sample along the ray [0,depth)
//     last     : The index of the last sample along the ray [0,depth)
//     
//  Programmer: Brad Whitlock
//  Creation:   Fri Feb 17 15:30:38 PST 2017
//
//  Modifications:
//
// ****************************************************************************

bool
avtMassVoxelExtractor::ComputeSamples(const double *origin, 
    const double *terminus, 
    int first, int last)
{
    bool retval;
    if(regularGridSpacing)
        retval = ComputeSamples_Regular(origin, terminus, first, last);
    else
        retval = ComputeSamples_Irregular(origin, terminus, first, last);
    return retval;
}



