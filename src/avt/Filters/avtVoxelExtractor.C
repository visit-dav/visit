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
#include <avtVoxelExtractor.h>
#include <avtVolume.h>
#include <avtCellList.h>

#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkMatrix4x4.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkTemplateAliasMacro.h>
#include <vtkUnsignedCharArray.h>

avtVoxelExtractor::avtVoxelExtractor(int w, int h, int d,
     avtVolume *vol, avtCellList *cl)
    : avtExtractor(w, h, d, vol, cl)
{
    gridsAreInWorldSpace = false;
    pretendGridsAreInWorldSpace = false;
    aspect = 1;
    cur_clip_range[0] = 0.;
    cur_clip_range[1] = 1.;
    view_to_world_transform = vtkMatrix4x4::New();
    world_to_view_transform = vtkMatrix4x4::New();
    trilinearInterpolation = false;

    X = NULL;
    Y = NULL;
    Z = NULL;
    divisors_X = NULL;
    divisors_Y = NULL;
    divisors_Z = NULL;
    ghosts = NULL;
    dims[0] = dims[1] = dims[2] = 0;
    ncell_arrays = 0;
    for(int i = 0; i < AVT_VARIABLE_LIMIT; ++i)
    {
        cell_arrays[i] = NULL;
        cell_size[i] = 0;
        cell_index[i] = 0;
        cell_vartypes[i] = 0;
    }
    npt_arrays = 0;
    for(int i = 0; i < AVT_VARIABLE_LIMIT; ++i)
    {
        pt_arrays[i] = NULL;
        pt_size[i] = 0;
        pt_index[i] = 0;
        pt_vartypes[i] = 0;
    }
}

avtVoxelExtractor::~avtVoxelExtractor()
{
    view_to_world_transform->Delete();
    world_to_view_transform->Delete();

    if (X != NULL)
        delete [] X;
    if (Y != NULL)
        delete [] Y;
    if (Z != NULL)
        delete [] Z;

    if (divisors_X != NULL)
        delete [] divisors_X;
    if (divisors_Y != NULL)
        delete [] divisors_Y;
    if (divisors_Z != NULL)
        delete [] divisors_Z;
}

// ****************************************************************************
//  Method: avtVoxelExtractor::SetGridsAreInWorldSpace
//
//  Purpose:
//      Tells the MVE whether or not it is extracting a world space or image
//      space grid.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 13:11:34 EST 2007
//    Added an ability to extract voxels using the world-space version
//    even when they're really in image space.
//
//    Hank Childs, Wed Dec 24 11:22:43 PST 2008
//    No longer calculate deprecated data member ProportionSpaceToZBufferSpace.
//
//    Hank Childs, Fri Nov 18 08:32:45 PST 2011
//    Fix ray cast of rectilinear with panning.
//
// ****************************************************************************

void
avtVoxelExtractor::SetGridsAreInWorldSpace(bool val, const avtViewInfo &v,
                                           double asp, const double *xform)
{
    gridsAreInWorldSpace = val;

    if (!gridsAreInWorldSpace)
    {
        if (xform)
        {
            // We're essentially doing resampling, but we cannot
            // use the faster version because there is another
            // transform to take into consideration.  In this case,
            // just revert to the world space algorithm and
            // fake the necessary parameters.
            pretendGridsAreInWorldSpace = true;
        }
        else
        {
            // We can use the faster version that doesn't depend on the
            // rest of the parameters set in this function, so return;
            return;
        }
    }

    view = v;
    aspect = asp;

    if (pretendGridsAreInWorldSpace)
    {
        view = avtViewInfo();
        view.setScale = true;
        view.parallelScale = 1;
        view.nearPlane = 1;
        view.farPlane = 2;

        aspect = 1.0;
    }

    //
    // Set up a VTK camera.  This will allow us to get the direction of
    // each ray and also the origin of each ray (this is simply the
    // position of the camera for perspective projection).
    //
    vtkCamera *cam = vtkCamera::New();
    // We have *= -1.0 throughout the code.  Here is "yet another".
    view.imagePan[0] *= -1.0;
    view.imagePan[1] *= -1.0;
    view.SetCameraFromView(cam);
    cam->GetClippingRange(cur_clip_range);
    vtkMatrix4x4 *mat = cam->GetCompositeProjectionTransformMatrix(aspect,
                                         cur_clip_range[0], cur_clip_range[1]);

    if (xform)
    {
        vtkMatrix4x4 *rectTrans = vtkMatrix4x4::New();
        rectTrans->DeepCopy(xform);
        vtkMatrix4x4::Multiply4x4(mat, rectTrans, view_to_world_transform);
        world_to_view_transform->DeepCopy(view_to_world_transform);
        view_to_world_transform->Invert();
        rectTrans->Delete();
    }
    else
    {
        // being executed for now for raycasting slivr
        vtkMatrix4x4::Invert(mat, view_to_world_transform);
        world_to_view_transform->DeepCopy(mat);
    }
    cam->Delete();
}

// ****************************************************************************
//  Function:  AssignEight
//
//  Purpose:
//      A templated function to assign 8 values to a floating point array.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2008
//
//  Modifications:
//    Kathleen Biagas, Fri Jul 13 09:23:55 PDT 2012
//    Use double instead of float.
//
// ****************************************************************************

template <class T> static void
AssignEight_(double *vals, const int *index, int s, int m, const T *array)
{
    for (int i = 0 ; i < 8 ; i++)
        vals[i] = (double) array[s*index[i]+m];
}

void
avtVoxelExtractor::AssignEight(int vartype, double *vals, const int *index,
    int s, int m, void *array)
{
    switch (vartype)
    {
      case VTK_CHAR:
        AssignEight_(vals, index, s, m, (const char *) array);
        break;
      case VTK_UNSIGNED_CHAR:
        AssignEight_(vals, index, s, m, (const unsigned char *) array);
        break;
      case VTK_SHORT:
        AssignEight_(vals, index, s, m, (const short *) array);
        break;
      case VTK_UNSIGNED_SHORT:
        AssignEight_(vals, index, s, m, (const unsigned short *) array);
        break;
      case VTK_INT:
        AssignEight_(vals, index, s, m, (const int *) array);
        break;
      case VTK_UNSIGNED_INT:
        AssignEight_(vals, index, s, m, (const unsigned int *) array);
        break;
      case VTK_UNSIGNED_LONG:
        AssignEight_(vals, index, s, m, (const long *) array);
        break;
      case VTK_FLOAT:
        AssignEight_(vals, index, s, m, (const float *) array);
        break;
      case VTK_DOUBLE:
        AssignEight_(vals, index, s, m, (const double *) array);
        break;
      case VTK_ID_TYPE:
        AssignEight_(vals, index, s, m, (vtkIdType *) array);
        break;
    }
}

// ****************************************************************************
//  Function:  ConvertToDouble
//
//  Purpose:
//     A function that performs a cast and conversion to a double.
//
//  Programmer: Hank Childs
//  Creation:   August 26, 2008
//
//  Modifications:
//    Kathleen Biagas, Fri Jul 13 09:23:55 PDT 2012
//    Use double instead of float.
//
// ****************************************************************************

double
avtVoxelExtractor::ConvertToDouble(int vartype, int index, int s, int m, void *array) const
{
    switch (vartype)
    {
      case VTK_CHAR:
        return (double) ((char*)array)[s*index+m];
      case VTK_UNSIGNED_CHAR:
        return (double) ((unsigned char*)array)[s*index+m];
      case VTK_SHORT:
        return (double) ((short*)array)[s*index+m];
      case VTK_UNSIGNED_SHORT:
        return (double) ((unsigned short*)array)[s*index+m];
      case VTK_INT:
        return (double) ((int*)array)[s*index+m];
      case VTK_UNSIGNED_INT:
        return (double) ((unsigned int*)array)[s*index+m];
      case VTK_UNSIGNED_LONG:
        return (double) ((unsigned long*)array)[s*index+m];
      case VTK_FLOAT:
        return (double) ((float*)array)[s*index+m];
      case VTK_DOUBLE:
        return ((double*)array)[s*index+m];
      case VTK_ID_TYPE:
        return (double) ((vtkIdType*)array)[s*index+m];
    }

    return 0.;
}

// ****************************************************************************
//  Function: FindMatch
//
//  Purpose:
//      Traverses an ordered array in logarithmic time.
//
//  Programmer: Hank Childs
//  Creation:   November 22, 2004
//
//  Modifications:
//    Kathleen Biagas, Fri Jul 13 09:23:55 PDT 2012
//    Use double instead of float.
//
// ****************************************************************************

int
avtVoxelExtractor::FindMatch(const double *A, const double &a, const int &nA) const
{
    if ((a < A[0]) || (a > A[nA-1]))
        return -1;

    int low = 0;
    int hi  = nA-1;
    while ((hi - low) > 1)
    {
        int guess = (hi+low)/2;
        if (A[guess] == a)
            return guess;
        if (a < A[guess])
            hi = guess;
        else
            low = guess;
    }


    return low;
}

// ****************************************************************************
//  Method: avtVoxelExtractor::FindIndex
//
//  Purpose:
//      Finds the index that corresponds to a point.
//
//  Programmer:   Hank Childs
//  Creation:     December 14, 2003
//
//  Modifications:
//    Kathleen Biagas, Fri Jul 13 07:44:46 PDT 2012
//    Templatized to handle coordinates of various types.
//
// ****************************************************************************

template <class T> inline int
FindIndex_(const double &pt, const int &last_hit, const int &n,
          T *vals)
{
    for (int i = last_hit ; i < n-1 ; ++i)
    {
        if (pt >= (double)vals[i] && (pt <= (double)vals[i+1]))
            return i;
    }

    for (int i = 0 ; i < last_hit ; ++i)
    {
        if (pt >= (double)vals[i] && (pt <= (double)vals[i+1]))
            return i;
    }

    return -1;
}

int
avtVoxelExtractor::FindIndex(vtkDataArray *coordArray,const double &pt,
    const int &last_hit, const int &n) const
{
    switch(coordArray->GetDataType())
    {
        vtkTemplateAliasMacro(return FindIndex_(pt, last_hit, n,
            static_cast<VTK_TT *>(coordArray->GetVoidPointer(0))));
        default:    return -1;
    }
}

// ****************************************************************************
//  Function:  FindRange
//
//  Purpose:
//      A templated function to find a range.
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 12, 2012
//
// ****************************************************************************

template <class T> inline void
FindRange_(int ind, double c, double &min, double &max, T *coord)
{
    double range = (double)coord[ind+1] - (double)coord[ind];
    min = 1. - (c - (double)coord[ind])/range;
    max = 1. - min;
}

void
avtVoxelExtractor::FindRange(vtkDataArray *coordArray, int ind, double c,
    double &min, double &max) const
{
    switch(coordArray->GetDataType())
    {
        vtkTemplateAliasMacro(FindRange_(ind, c, min, max,
            static_cast<VTK_TT *>(coordArray->GetVoidPointer(0))));
        default:
            EXCEPTION1(VisItException, "Unknown Coordinate type");
    }
}

// ****************************************************************************
//  Method: avtVoxelExtractor::computeIndices
//
//  Purpose:
//              back: 5, front: 4,  top:3, bottom:2,  ;right: 1, left:0
//
//  Programmer: Pascal Grosset
//  Creation:   June 10, 2013
//
//  Modifications:
//    Brad Whitlock, Tue Jan  2 14:36:53 PST 2018
//    optimizations.
//
// ****************************************************************************

void 
avtVoxelExtractor::computeIndices(int dims[3], int indices[6], int returnIndices[8]) const
{
    int K0 = (indices[4])*((dims[0]-1)*(dims[1]-1));
    int K1 = (indices[5])*((dims[0]-1)*(dims[1]-1));
    int J0 = (indices[2])*(dims[0]-1);
    int J1 = (indices[3])*(dims[0]-1);

    returnIndices[0] = K0 + J0 + indices[0]; //(indices[4])*((dims[0]-1)*(dims[1]-1)) + (indices[2])*(dims[0]-1) + (indices[0]);
    returnIndices[1] = K0 + J0 + indices[1]; //(indices[4])*((dims[0]-1)*(dims[1]-1)) + (indices[2])*(dims[0]-1) + (indices[1]);

    returnIndices[2] = K0 + J1 + indices[0]; //(indices[4])*((dims[0]-1)*(dims[1]-1)) + (indices[3])*(dims[0]-1) + (indices[0]);
    returnIndices[3] = K0 + J1 + indices[1]; //(indices[4])*((dims[0]-1)*(dims[1]-1)) + (indices[3])*(dims[0]-1) + (indices[1]);

    returnIndices[4] = K1 + J0 + indices[0]; //(indices[5])*((dims[0]-1)*(dims[1]-1)) + (indices[2])*(dims[0]-1) + (indices[0]);
    returnIndices[5] = K1 + J0 + indices[1]; //(indices[5])*((dims[0]-1)*(dims[1]-1)) + (indices[2])*(dims[0]-1) + (indices[1]);

    returnIndices[6] = K1 + J1 + indices[0]; //(indices[5])*((dims[0]-1)*(dims[1]-1)) + (indices[3])*(dims[0]-1) + (indices[0]);
    returnIndices[7] = K1 + J1 + indices[1]; //(indices[5])*((dims[0]-1)*(dims[1]-1)) + (indices[3])*(dims[0]-1) + (indices[1]);
}

void 
avtVoxelExtractor::computeIndicesVert(int dims[3], int indices[6], int returnIndices[8]) const
{
    int K0 = (indices[4])*((dims[0])*(dims[1]));
    int K1 = (indices[5])*((dims[0])*(dims[1]));
    int J0 = (indices[2])*(dims[0]);
    int J1 = (indices[3])*(dims[0]);

    returnIndices[0] = K0 + J0 + indices[0]; //(indices[4])*((dims[0])*(dims[1])) + (indices[2])*(dims[0]) + (indices[0]);
    returnIndices[1] = K0 + J0 + indices[1]; //(indices[4])*((dims[0])*(dims[1])) + (indices[2])*(dims[0]) + (indices[1]);

    returnIndices[2] = K0 + J1 + indices[0]; //(indices[4])*((dims[0])*(dims[1])) + (indices[3])*(dims[0]) + (indices[0]);
    returnIndices[3] = K0 + J1 + indices[1]; //(indices[4])*((dims[0])*(dims[1])) + (indices[3])*(dims[0]) + (indices[1]);

    returnIndices[4] = K1 + J0 + indices[0]; //(indices[5])*((dims[0])*(dims[1])) + (indices[2])*(dims[0]) + (indices[0]);
    returnIndices[5] = K1 + J0 + indices[1]; //(indices[5])*((dims[0])*(dims[1])) + (indices[2])*(dims[0]) + (indices[1]);

    returnIndices[6] = K1 + J1 + indices[0]; //(indices[5])*((dims[0])*(dims[1])) + (indices[3])*(dims[0]) + (indices[0]);
    returnIndices[7] = K1 + J1 + indices[1]; //(indices[5])*((dims[0])*(dims[1])) + (indices[3])*(dims[0]) + (indices[1]);
}

// ****************************************************************************
//  Method: avtImgCommunicator::trilinearInterpolate
//
//  Purpose:
//      Trilinear interpolation
//
//  Programmer: Pascal Grosset
//  Creation:   June 10, 2013
//
//  Modifications:
//
// ****************************************************************************

double
avtVoxelExtractor::trilinearInterpolate(double vals[8], float distRight, 
    float distTop, float distBack) const
{
    float dist_from_right = 1.0 - distRight;
    float dist_from_left = distRight;

    float dist_from_top = 1.0 - distTop;
    float dist_from_bottom = distTop;

    float dist_from_back = 1.0 - distBack;
    float dist_from_front = distBack;

    double val =    dist_from_right     * dist_from_top         * dist_from_back * vals[0] +
                    dist_from_left      * dist_from_top         * dist_from_back * vals[1] +
                    dist_from_right     * dist_from_bottom      * dist_from_back * vals[2] +
                    dist_from_left      * dist_from_bottom      * dist_from_back * vals[3] +

                    dist_from_right     * dist_from_top         * dist_from_front * vals[4] +
                    dist_from_left      * dist_from_top         * dist_from_front * vals[5] +
                    dist_from_right     * dist_from_bottom      * dist_from_front * vals[6] +
                    dist_from_left      * dist_from_bottom      * dist_from_front * vals[7];
    return val;
}

// ****************************************************************************
//  Method: avtVoxelExtractor::getIndexandDistFromCenter
//
//  Purpose:
//
//  Programmer: 
//  Creation:   
//
//  Modifications:
//
// ****************************************************************************

void 
avtVoxelExtractor::getIndexandDistFromCenter(float dist, 
    int index, int &index_before, int &index_after,
    float &dist_before, float &dist_after) const
{
    float center = 0.5;
    if (dist < center){
        index_before = index-1;
        index_after = index;
        dist_before = dist + center;
        dist_after = 1.0 - dist_before;
    }else{
        index_before = index;
        index_after = index+1;
        dist_before = dist - center;
        dist_after = 1.0 - dist_before;
    }
}


// ****************************************************************************
//  Method: avtVoxelExtractor::RegisterGrid
//
//  Purpose:
//      Registers a rectilinear grid.  This is called in combination with
//      SampleAlongSegment, which assumes that data members in this class have
//      been set up.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2004
//
//  Modifications:
//
//    Hank Childs, Fri Jun  1 15:37:33 PDT 2007
//    Add support for non-scalars.
//
//    Hank Childs, Wed Aug 27 11:06:27 PDT 2008
//    Add support for non-floats.
//
//    Hank Childs, Thu Aug 28 10:52:32 PDT 2008
//    Make sure we only sample the variables that were requested.
//
//    Kathleen Biagas, Fri Jul 13 09:23:55 PDT 2012
//    Use double instead of float.
//
// ****************************************************************************

void
avtVoxelExtractor::RegisterGrid(vtkRectilinearGrid *rgrid,
                                std::vector<std::string> &varorder,
                                std::vector<int> &varsize)
{
    rgrid->GetDimensions(dims);
    if (X != NULL)
        delete [] X;
    if (Y != NULL)
        delete [] Y;
    if (Z != NULL)
        delete [] Z;

    // dims is the size of each of the small 3D patches e.g. 52x16x16 (or grid)
    // X, Y & Z store the "real" coordinates each point in the grid (the above grid) e.g. 0.61075, 0.19536, 0.01936 for 0,0,0
    X = new double[dims[0]];
    for (size_t i = 0 ; i < (size_t)dims[0] ; i++)
        X[i] = rgrid->GetXCoordinates()->GetTuple1(i);
    Y = new double[dims[1]];
    for (size_t i = 0 ; i < (size_t)dims[1] ; i++)
        Y[i] = rgrid->GetYCoordinates()->GetTuple1(i);
    Z = new double[dims[2]];
    for (size_t i = 0 ; i < (size_t)dims[2] ; i++)
        Z[i] = rgrid->GetZCoordinates()->GetTuple1(i);

    vtkDataArray *arr = rgrid->GetCellData()->GetArray("avtGhostZones");
    if (arr != NULL)
        ghosts = (unsigned char *) arr->GetVoidPointer(0);
    else
        ghosts = NULL;

    ncell_arrays = 0;
    for (size_t i = 0 ; i < (size_t) rgrid->GetCellData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = rgrid->GetCellData()->GetArray(i);
        const char *name = arr->GetName();
        int idx = -1;
        for (size_t j = 0 ; j < varorder.size() ; j++)
        {
            if (varorder[j] == name)
            {
                idx = 0;
                for (size_t k = 0 ; k < j ; k++)
                    idx += varsize[k];
                break;
            }
        }
        if (idx < 0)
            continue;
        cell_index[ncell_arrays] = idx;
        cell_vartypes[ncell_arrays] = arr->GetDataType();
        cell_size[ncell_arrays] = arr->GetNumberOfComponents();
        cell_arrays[ncell_arrays++] = arr->GetVoidPointer(0);
    }

    npt_arrays = 0;
    for (size_t i = 0 ; i < (size_t)rgrid->GetPointData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = rgrid->GetPointData()->GetArray(i);
        const char *name = arr->GetName();
        int idx = -1;
        for (size_t j = 0 ; j < varorder.size() ; j++)
        {
            if (varorder[j] == name)
            {
                idx = 0;
                for (size_t k = 0 ; k < j ; k++)
                    idx += varsize[k];
                break;
            }
        }
        if (idx < 0)
            continue;
        pt_index[npt_arrays] = idx;
        pt_vartypes[npt_arrays] = arr->GetDataType();
        pt_size[npt_arrays] = arr->GetNumberOfComponents();
        pt_arrays[npt_arrays++] = arr->GetVoidPointer(0);
    }

    if (divisors_X != NULL)
        delete [] divisors_X;
    if (divisors_Y != NULL)
        delete [] divisors_Y;
    if (divisors_Z != NULL)
        delete [] divisors_Z;

    //
    // We end up dividing by the term A[i+1]/A[i] a whole bunch.  So store
    // out its inverse so that we can do cheap multiplication.  This gives us
    // a 5% performance boost.
    //
    if(dims[0]-1 > 0)
    {
        divisors_X = new double[dims[0]-1];
        for (size_t i = 0 ; i < (size_t)dims[0] - 1 ; i++)
            divisors_X[i] = (X[i+1] == X[i] ? 1. : 1./(X[i+1]-X[i]));
    }
    else
    {
        divisors_X = new double[1];
        divisors_X[0] = 1.;
    }

    if(dims[1]-1 > 0)
    {
        divisors_Y = new double[dims[1]-1];
        for (size_t i = 0 ; i < (size_t)dims[1] - 1 ; i++)
            divisors_Y[i] = (Y[i+1] == Y[i] ? 1. : 1./(Y[i+1]-Y[i]));
    }
    else
    {
        divisors_Y = new double[1];
        divisors_Y[0] = 1.;
    }

    if(dims[2]-1 > 0)
    {
        divisors_Z = new double[dims[2]-1];
        for (size_t i = 0 ; i < (size_t)dims[2] - 1 ; i++)
            divisors_Z[i] = (Z[i+1] == Z[i] ? 1. : 1./(Z[i+1]-Z[i]));
    }
    else
    {
        divisors_Z = new double[1];
        divisors_Z[0] = 1.;
    }
}

// ****************************************************************************
//  Method: avtVoxelExtractor::GetSegment
//
//  Purpose:
//      Gets a line segment based on a pixel location.
//
//  Arguments:
//      w           x; of the width of the render window 
//      h           y; of the height of the render window
//      origin      
//      terminus
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2004
//
//  Modifications:
// 
//    Jeremy Meredith, Fri Feb  9 14:00:51 EST 2007
//    Flip back across the x axis if pretendGridsAreInWorldSpace is set.
//
//    Hank Childs, Fri Jan  9 14:07:49 PST 2009
//    Add support for jittering.
//
//    Kathleen Biagas, Fri Jul 13 09:23:55 PDT 2012
//    Use double instead of float.
//
// ****************************************************************************

void
avtVoxelExtractor::GetSegment(int w, int h, double *origin, double *terminus)
    const
{
    double view[4];

    //
    // The image is being reflected across a center vertical line.  This is the
    // easiest place to fix it.  I haven't tracked down where it is, but it
    // would appear to a problem with the interface from our images to VTK's.
    //
    // Note: If we're only pretending that grids are in world space -- i.e.
    // we're resampling, not raycasting an image -- then flipping across
    // the x axis like this is wrong.  Take this into account here.
    //
    double width_div_2 = width * .5;
    double height_div_2 = height * .5;

    view[0] = (w - width_div_2)/(width_div_2);
    if (pretendGridsAreInWorldSpace)
        view[0] *= -1;
    view[1] = (h - height_div_2)/(height_div_2);
    view[2] = cur_clip_range[0];
    view[3] = 1.;
    view_to_world_transform->MultiplyPoint(view, origin);
    if (origin[3] != 0.)
    {
        double inv_origin3 = 1. / origin[3];
        origin[0] *= inv_origin3;
        origin[1] *= inv_origin3;
        origin[2] *= inv_origin3;
    }

    // view[0] = (w - width/2.)/(width/2.);
    //if (pretendGridsAreInWorldSpace)
    //    view[0] *= -1;
    //view[1] = (h - height/2.)/(height/2.);
    view[2] = cur_clip_range[1];
    //view[3] = 1.;
    view_to_world_transform->MultiplyPoint(view, terminus);
    if (terminus[3] != 0.)
    {
        double inv_terminus3 = 1. / terminus[3];
        terminus[0] *= inv_terminus3;
        terminus[1] *= inv_terminus3;
        terminus[2] *= inv_terminus3;
    }


    if (jitter)
    {
        int reliable_random_number = (13*w*h + 14*w*w + 79*h*h + 247*w + 779*h)%513;
        double jitter = (1.0/depth) * ((reliable_random_number-256) / (256.0));
        double dir[3];
        dir[0] = (terminus[0] - origin[0])*jitter;
        dir[1] = (terminus[1] - origin[1])*jitter;
        dir[2] = (terminus[2] - origin[2])*jitter;
        origin[0] += dir[0];
        origin[1] += dir[1];
        origin[2] += dir[2];
        terminus[0] += dir[0];
        terminus[1] += dir[1];
        terminus[2] += dir[2];
    }
}

// ****************************************************************************
//  Method: avtVoxelExtractor::FindSegmentIntersections
//
//  Purpose:
//      Finds the intersection points of a line segment and a rectilinear grid.
//  
//  Programmer: Hank Childs
//  Creation:   November 21, 2004
//
//  Modifications:
//
//    Hank Childs, Tue Feb  5 15:44:53 PST 2008
//    Fix bugs with the origin or the terminus of the segment being inside 
//    the volume.
//
//    Hank Childs, Wed Dec 24 11:21:57 PST 2008
//    Change the logic for perspective projections to account for w-buffering
//    (that is, even sampling in space).
//
//    Hank Childs, Wed Dec 31 09:08:50 PST 2008
//    For the case where the segment intersects the volume one time, return
//    that the entire segment should be examined, as it is probably more
//    likely that we have floating point error than we have intersected a
//    corner of the data set.
//
//    Kathleen Biagas, Fri Jul 13 09:23:55 PDT 2012
//    Use double instead of float.
//
// ****************************************************************************

bool
avtVoxelExtractor::FindSegmentIntersections(const double *origin, 
                                  const double *terminus, int &start, int &end)
{
    double  t, x, y, z;

    int num_hits = 0;
    double hits[6]; // Should always be 2 or 0.

    double x_min = X[0];
    double x_max = X[dims[0]-1];
    double y_min = Y[0];
    double y_max = Y[dims[1]-1];
    double z_min = Z[0];
    double z_max = Z[dims[2]-1];

    if (x_min <= origin[0] && origin[0] <= x_max &&
        y_min <= origin[1] && origin[1] <= y_max &&
        z_min <= origin[2] && origin[2] <= z_max)
    {
        hits[num_hits++] = 0.0;
    }
    if (x_min <= terminus[0] && terminus[0] <= x_max &&
        y_min <= terminus[1] && terminus[1] <= y_max &&
        z_min <= terminus[2] && terminus[2] <= z_max)
    {
        hits[num_hits++] = 1.0;
    }

    //
    // If the terminus and the origin have the same X, then we will find
    // the intersection at another face.
    //
    if (terminus[0] != origin[0])
    {
        //
        // See if we hit the X-min face.
        //
        t = (x_min - origin[0]) / (terminus[0] - origin[0]);
        y = origin[1] + t*(terminus[1] - origin[1]);
        z = origin[2] + t*(terminus[2] - origin[2]);
        if (y_min <= y && y <= y_max && z_min <= z && z <= z_max &&
            t > 0. && t < 1.)
        {
            hits[num_hits++] = t;
        }

        //
        // See if we hit the X-max face.
        //
        t = (x_max - origin[0]) / (terminus[0] - origin[0]);
        y = origin[1] + t*(terminus[1] - origin[1]);
        z = origin[2] + t*(terminus[2] - origin[2]);
        if (y_min <= y && y <= y_max && z_min <= z && z <= z_max &&
            t > 0. && t < 1.)
        {
            hits[num_hits++] = t;
        }
    }

    //
    // If the terminus and the origin have the same Y, then we will find
    // the intersection at another face.
    //
    if (terminus[1] != origin[1])
    {
        //
        // See if we hit the Y-min face.
        //
        t = (y_min - origin[1]) / (terminus[1] - origin[1]);
        x = origin[0] + t*(terminus[0] - origin[0]);
        z = origin[2] + t*(terminus[2] - origin[2]);
        if (x_min <= x && x <= x_max && z_min <= z && z <= z_max &&
            t > 0. && t < 1.)
        {
            hits[num_hits++] = t;
        }

        //
        // See if we hit the Y-max face.
        //
        t = (y_max - origin[1]) / (terminus[1] - origin[1]);
        x = origin[0] + t*(terminus[0] - origin[0]);
        z = origin[2] + t*(terminus[2] - origin[2]);
        if (x_min <= x && x <= x_max && z_min <= z && z <= z_max &&
            t > 0. && t < 1.)
        {
            hits[num_hits++] = t;
        }
    }

    //
    // If the terminus and the origin have the same Z, then we will find
    // the intersection at another face.
    //
    if (terminus[2] != origin[2])
    {
        //
        // See if we hit the Z-min face.
        //
        t = (z_min - origin[2]) / (terminus[2] - origin[2]);
        x = origin[0] + t*(terminus[0] - origin[0]);
        y = origin[1] + t*(terminus[1] - origin[1]);
        if (x_min <= x && x <= x_max && y_min <= y && y <= y_max &&
            t > 0. && t < 1.)
        {
            hits[num_hits++] = t;
        }

        //
        // See if we hit the Z-max face.
        //
        t = (z_max - origin[2]) / (terminus[2] - origin[2]);
        x = origin[0] + t*(terminus[0] - origin[0]);
        y = origin[1] + t*(terminus[1] - origin[1]);
        if (x_min <= x && x <= x_max && y_min <= y && y <= y_max &&
            t > 0. && t < 1.)
        {
            hits[num_hits++] = t;
        }
    }

    if (num_hits == 0)
        return false;

    //
    // We are expecting exactly two hits.  If we don't get that, then
    // we probably hit an edge of the dataset.  Give up on this optimization.
    //
    if (num_hits != 2)
    {
        start = 0;
        end = depth-1;
        return true;
    }

    if (hits[0] > hits[1])
    {
        double t = hits[0];
        hits[0] = hits[1];
        hits[1] = t;
    }

    if (hits[0] < 0 && hits[1] < 0)
        // Dataset on back side of camera -- no intersection.
        return false;
    if (hits[0] > 1. && hits[1] > 1.)
        // Dataset past far clipping plane -- no intersection.
        return false;

    // This is the correct calculation whether we are using orthographic or
    // perspective projection ... because with perspective we are using
    // w-buffering.
    start = (int) floor(depth*hits[0]);
    end   = (int) ceil(depth*hits[1]);
    if (start < 0)
        start = 0;
    if (end > depth)
        end = depth;

    return true;
}

// ****************************************************************************
//  Method: avtVoxelExtractor::FrustumIntersectsGrid
//
//  Purpose:
//      Determines if a frustum intersects the grid.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2004
//
//  Modifications:
//    Kathleen Biagas, Fri Jul 13 09:23:55 PDT 2012
//    Use double instead of float.
//
// ****************************************************************************

bool
avtVoxelExtractor::FrustumIntersectsGrid(int w_min, int w_max, int h_min,
                                             int h_max) const
{
    //
    // Start off by getting the segments corresponding to the bottom left (bl),
    // upper left (ul), bottom right (br), and upper right (ur) rays.
    //
    double bl_start[4];
    double bl_end[4];
    GetSegment(w_min, h_min, bl_start, bl_end);

    double ul_start[4];
    double ul_end[4];
    GetSegment(w_min, h_max, ul_start, ul_end);

    double br_start[4];
    double br_end[4];
    GetSegment(w_max, h_min, br_start, br_end);

    double ur_start[4];
    double ur_end[4];
    GetSegment(w_max, h_max, ur_start, ur_end);

    //
    // Now use those segments to construct bounding planes.  If the grid is
    // not on the plus side of the bounding planes, then none of the frustum
    // will intersect the grid.
    //
    // Note: the plus side of the plane is dependent on the order that these
    // points are sent into the routine "FindPlaneNormal".  There are some
    // subtleties with putting the arguments in the right order.
    //
    double normal[3];
    FindPlaneNormal(bl_start, bl_end, ul_start, normal);
    if (!GridOnPlusSideOfPlane(bl_start, normal))
        return false;
    FindPlaneNormal(bl_start, br_start, br_end, normal);
    if (!GridOnPlusSideOfPlane(bl_start, normal))
        return false;
    FindPlaneNormal(ur_start, ul_start, ur_end, normal);
    if (!GridOnPlusSideOfPlane(ur_start, normal))
        return false;
    FindPlaneNormal(ur_start, ur_end, br_start, normal);
    if (!GridOnPlusSideOfPlane(ur_start, normal))
        return false;

    return true;
}

// ****************************************************************************
//  Method: avtVoxelExtractor::FindPlaneNormal
//
//  Purpose:
//      Finds the normal to a plane using three points.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2004
//
//  Modifications:
//    Kathleen Biagas, Fri Jul 13 09:23:55 PDT 2012
//    Use double instead of float.
//
// ****************************************************************************

void
avtVoxelExtractor::FindPlaneNormal(const double *pt1, const double *pt2,
                                   const double *pt3, double *normal) const
{
    //
    // Set up vectors P1P2 and P1P3.
    //
    double v1[3];
    double v2[3];

    v1[0] = pt2[0] - pt1[0];
    v1[1] = pt2[1] - pt1[1];
    v1[2] = pt2[2] - pt1[2];
    v2[0] = pt3[0] - pt1[0];
    v2[1] = pt3[1] - pt1[1];
    v2[2] = pt3[2] - pt1[2];

    //
    // The normal is the cross-product of these two vectors.
    //
    normal[0] = v1[1]*v2[2] - v1[2]*v2[1];
    normal[1] = v1[2]*v2[0] - v1[0]*v2[2];
    normal[2] = v1[0]*v2[1] - v1[1]*v2[0];
}


// ****************************************************************************
//  Method: avtVoxelExtractor::GridOnPlusSideOfPlane
//
//  Purpose:
//      Determines if a grid is on the plus side of a plane.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 13:11:34 EST 2007
//    Added an ability to extract voxels using the world-space version
//    even when they're really in image space.
//
//    Kathleen Biagas, Fri Jul 13 09:23:55 PDT 2012
//    Use double instead of float.
//
// ****************************************************************************

bool
avtVoxelExtractor::GridOnPlusSideOfPlane(const double *origin, 
                                             const double *normal) const
{
    double x_min = X[0];
    double x_max = X[dims[0]-1];
    double y_min = Y[0];
    double y_max = Y[dims[1]-1];
    double z_min = Z[0];
    double z_max = Z[dims[2]-1];

    for (int i = 0 ; i < 8 ; i++)
    {
        double pt[3];
        pt[0] = (i & 1 ? x_max : x_min);
        pt[1] = (i & 2 ? y_max : y_min);
        pt[2] = (i & 4 ? z_max : z_min);

        //
        // The plane is of the form Ax + By + Cz - D = 0.
        //
        // Using the origin, we can calculate D:
        // D = A*origin[0] + B*origin[1] + C*origin[2]
        //
        // We want to know if 'pt' gives:
        // A*pt[0] + B*pt[1] + C*pt[2] - D >=? 0.
        //
        // We can substitute in D to get
        // A*(pt[0]-origin[0]) + B*(pt[1]-origin[1]) + C*(pt[2-origin[2]) ?>= 0
        //
        double val  = normal[0]*(pt[0] - origin[0])
                   + normal[1]*(pt[1] - origin[1])
                   + normal[2]*(pt[2] - origin[2]);

        // Note: If we're only pretending that grids are in world space -- i.e.
        // we're resampling, not raycasting an image -- then flipping across
        // the x axis also negates the proper normal values here.
        if (pretendGridsAreInWorldSpace)
            val *= -1;

        if (val >= 0)
            return true;
    }

    return false;
}

// ****************************************************************************
//  Method: avtVoxelExtractor::ExtractImageSpaceGrid
//
//  Purpose:
//      Extracts a grid that has already been put into image space.  This case
//      typically corresponds to resampling.
//
//  Programmer: Hank Childs
//  Creation:   December 14, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Aug 27 16:00:57 PDT 2004
//    Rename ghost data array.
//
//    Hank Childs, Fri Nov 19 14:50:58 PST 2004
//    Renamed from Extract.
//
//    Hank Childs, Mon Jul 11 14:01:28 PDT 2005
//    Fix indexing issue with ghost zones ['5712].
//
//    Hank Childs, Mon Feb  6 12:36:51 PST 2006
//    Fix another issue with ghost zones that only comes up with AMR grids
//    ['6940].
//
//    Hank Childs, Fri Jun  1 15:45:58 PDT 2007
//    Add support for non-scalars.
//
//    Hank Childs, Wed Aug 27 11:07:04 PDT 2008
//    Add support for non-floats.
//
//    Hank Childs, Thu Aug 28 10:52:32 PDT 2008
//    Make sure we only sample the variables that were requested.
//
//    Hank Childs, Sun Nov  1 14:32:46 CST 2009
//    Fix bug where ghost data could cause an extra sample to be put in the
//    avtVolume, with that sample's data be uninitialized memory.
//
//    Kathleen Biagas, Fri Jul 13 09:30:53 PDT 2012
//    Handle Coordinates as their native type. Use double internally.
//
// ****************************************************************************

void
avtVoxelExtractor::ExtractImageSpaceGrid(vtkRectilinearGrid *rgrid,
                std::vector<std::string> &varnames, std::vector<int> &varsizes)
{
    int  i, j, k, l, m;

    int dims[3];
    rgrid->GetDimensions(dims);
    const int nX = dims[0];
    const int nY = dims[1];
    const int nZ = dims[2];

    int last_x_hit = 0;
    int last_y_hit = 0;
    int last_z_hit = 0;

    vtkUnsignedCharArray *ghosts = (vtkUnsignedCharArray *)rgrid->GetCellData()
                                                   ->GetArray("avtGhostZones");
    std::vector<void *>  cell_arrays;
    std::vector<int>     cell_vartypes;
    std::vector<int>     cell_size;
    std::vector<int>     cell_index;
    for (i = 0 ; i < rgrid->GetCellData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = rgrid->GetCellData()->GetArray(i);
        const char *name = arr->GetName();
        int idx = -1;
        for (j = 0 ; j < (int)varnames.size() ; j++)
        {
            if (varnames[j] == name)
            {
                idx = 0;
                for (k = 0 ; k < j ; k++)
                    idx += varsizes[k];
                break;
            }
        }
        if (idx < 0)
            continue;
        cell_index.push_back(idx);
        cell_size.push_back(arr->GetNumberOfComponents());
        cell_vartypes.push_back(arr->GetDataType());
        cell_arrays.push_back(arr->GetVoidPointer(0));
    }

    std::vector<void *>  pt_arrays;
    std::vector<int>     pt_vartypes;
    std::vector<int>     pt_size;
    std::vector<int>     pt_index;
    for (i = 0 ; i < rgrid->GetPointData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = rgrid->GetPointData()->GetArray(i);
        const char *name = arr->GetName();
        int idx = -1;
        for (j = 0 ; j < (int)varnames.size() ; j++)
        {
            if (varnames[j] == name)
            {
                idx = 0;
                for (k = 0 ; k < j ; k++)
                    idx += varsizes[k];
                break;
            }
        }
        if (idx < 0)
            continue;
        pt_index.push_back(idx);
        pt_size.push_back(arr->GetNumberOfComponents());
        pt_vartypes.push_back(arr->GetDataType());
        pt_arrays.push_back(arr->GetVoidPointer(0));
    }

    vtkDataArray *xarray = rgrid->GetXCoordinates();
    vtkDataArray *yarray = rgrid->GetYCoordinates();
    vtkDataArray *zarray = rgrid->GetZCoordinates();

    int startX = SnapXLeft(xarray->GetTuple1(0));
    int stopX  = SnapXRight(xarray->GetTuple1(nX-1));
    int startY = SnapYBottom(yarray->GetTuple1(0));
    int stopY  = SnapYTop(yarray->GetTuple1(nY-1));
    int startZ = SnapZFront(zarray->GetTuple1(0));
    int stopZ  = SnapZBack(zarray->GetTuple1(nZ-1));

    for (j = startY ; j <= stopY ; j++)
    {
        double yc = YFromIndex(j);
        int yind = FindIndex(yarray, yc, last_y_hit, nY);
        if (yind == -1)
            continue;
        last_y_hit = yind;

        double y_bottom  = 0.;
        double y_top = 1.;
        if (pt_arrays.size() > 0)
        {
            FindRange(yarray, yind, yc, y_bottom, y_top);
        }
        for (i = startX ; i <= stopX ; i++)
        {
            double xc = XFromIndex(i);
            int xind = FindIndex(xarray, xc, last_x_hit, nX);
            if (xind == -1)
                continue;
            last_x_hit = xind;

            double x_left  = 0.;
            double x_right = 1.;
            if (pt_arrays.size() > 0)
            {
                FindRange(xarray, xind, xc, x_left, x_right);
            }

            last_z_hit = 0;
            int count = 0;
            int firstZ = -1;
            int lastZ  = stopZ;
            for (k = startZ ; k <= stopZ ; k++)
            {
                double zc = ZFromIndex(k);
                int zind = FindIndex(zarray, zc, last_z_hit, nZ);
                if (zind == -1)
                {
                    if (firstZ == -1)
                        continue;
                    else
                    {
                        lastZ = k-1;
                        break;
                    }
                }
                if ((count == 0) && (firstZ == -1))
                    firstZ = k;
                last_z_hit = zind;

                //
                // Don't sample from ghost zones.
                //
                if (ghosts != NULL)
                {
                    int index = zind*((nX-1)*(nY-1)) + yind*(nX-1) + xind;
                    if (ghosts->GetValue(index) != 0)
                    {
                        if (count > 0)
                        {
                            avtRay *ray = volume->GetRay(i, j);
                            ray->SetSamples(firstZ, k-1, tmpSampleList);
                        }
                        firstZ = -1;
                        count = 0;
                        continue;
                    }
                }

                double z_front  = 0.;
                double z_back = 1.;
                if (pt_arrays.size() > 0)
                {
                    FindRange(zarray, zind, zc, z_front, z_back);
                }

                for (l = 0 ; l < (int)cell_arrays.size() ; l++)
                {
                    int index = zind*((nX-1)*(nY-1)) + yind*(nX-1) + xind;
                    for (m = 0 ; m < cell_size[l] ; m++)
                        tmpSampleList[count][cell_index[l]+m] =
                                  ConvertToDouble(cell_vartypes[l],index,
                                              cell_size[l], m, cell_arrays[l]);
                }
                if (pt_arrays.size() > 0)
                {
                    int index[8];
                    index[0] = (zind)*nX*nY + (yind)*nX + (xind);
                    index[1] = (zind)*nX*nY + (yind)*nX + (xind+1);
                    index[2] = (zind)*nX*nY + (yind+1)*nX + (xind);
                    index[3] = (zind)*nX*nY + (yind+1)*nX + (xind+1);
                    index[4] = (zind+1)*nX*nY + (yind)*nX + (xind);
                    index[5] = (zind+1)*nX*nY + (yind)*nX + (xind+1);
                    index[6] = (zind+1)*nX*nY + (yind+1)*nX + (xind);
                    index[7] = (zind+1)*nX*nY + (yind+1)*nX + (xind+1);
                    for (l = 0 ; l < (int)pt_arrays.size() ; l++)
                    {
                        void  *pt_array = pt_arrays[l];
                        int    s        = pt_size[l];
                        for (m = 0 ; m < s ; m++)
                        {
                            double vals[8];
                            AssignEight(pt_vartypes[l], vals, index,
                                        s, m, pt_array);
                            double val =
                                  x_left*y_bottom*z_front*vals[0] +
                                  x_right*y_bottom*z_front*vals[1] +
                                  x_left*y_top*z_front*vals[2] +
                                  x_right*y_top*z_front*vals[3] +
                                  x_left*y_bottom*z_back*vals[4] +
                                  x_right*y_bottom*z_back*vals[5] +
                                  x_left*y_top*z_back*vals[6] +
                                  x_right*y_top*z_back*vals[7];
                            tmpSampleList[count][pt_index[l]+m] = val;
                        }
                    }
                }
                count++;
            }

            if (count > 0)
            {
                avtRay *ray = volume->GetRay(i, j);
                ray->SetSamples(firstZ, lastZ, tmpSampleList);
            }
        }
    }
}
