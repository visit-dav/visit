// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtSLIVRVoxelExtractor.C                       //
// ************************************************************************* //

#include <avtSLIVRVoxelExtractor.h>

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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <limits>
#include <math.h>


// ****************************************************************************
//  Method: avtSLIVRVoxelExtractor constructor
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

avtSLIVRVoxelExtractor::avtSLIVRVoxelExtractor(int w, int h, int d,
                                             avtVolume *vol, avtCellList *cl)
    : avtVoxelExtractor(w, h, d, vol, cl)
{
    fullImgWidth = w;
    fullImgHeight = h;

    debug5 << "fullImgWidth: " << fullImgWidth << "    fullImgHeight: " << fullImgHeight << std::endl;
    rayCastingSLIVR = true;

    modelViewProj = vtkMatrix4x4::New();
    invModelViewProj = vtkMatrix4x4::New();

    prop_buffer   = new double[3*depth];
    ind_buffer    = new int[3*depth];
    valid_sample  = new bool[depth];
    lighting = false;
    lightPosition[0] = lightPosition[1] = lightPosition[2] = 0.0;   lightPosition[3] = 1.0;
    materialProperties[0] = 0.4; materialProperties[1] = 0.75; materialProperties[3] = 0.0; materialProperties[3] = 15.0;
    gradient[0] = gradient[1] = gradient[2] = 0;

    proc = patch = 0;
    patchDrawn = 0;
    imgDims[0] = imgDims[1] = 0;                // size of the patch
    imgLowerLeft[0] = imgLowerLeft[1] = 0;      // coordinates in the whole image
    imgUpperRight[0] = imgUpperRight[1] = 0;    //
    eyeSpaceDepth = -1;
    clipSpaceDepth = -1;
    imgArray = NULL;                            // the image data

    depthBuffer = NULL;
    rgbColorBuffer = NULL;
}


// ****************************************************************************
//  Method: avtSLIVRVoxelExtractor destructor
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

avtSLIVRVoxelExtractor::~avtSLIVRVoxelExtractor()
{
    if (prop_buffer != NULL)
        delete [] prop_buffer;
    if (ind_buffer != NULL)
        delete [] ind_buffer;
    if (valid_sample != NULL)
        delete [] valid_sample;
    if (imgArray != NULL)
        delete []imgArray;

    imgArray = NULL;
}

// ****************************************************************************
//  Method: avtSLIVRVoxelExtractor::Extract
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
avtSLIVRVoxelExtractor::Extract(vtkRectilinearGrid *rgrid,
                std::vector<std::string> &varnames, std::vector<int> &varsizes)
{
    if (gridsAreInWorldSpace || pretendGridsAreInWorldSpace)
        ExtractWorldSpaceGridRCSLIVR(rgrid, varnames, varsizes);
    else
        ExtractImageSpaceGrid(rgrid, varnames, varsizes);
}

// ****************************************************************************
//  Method: avtSLIVRVoxelExtractor::SampleVariable
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
avtSLIVRVoxelExtractor::SampleVariable(int first, int last, int w, int h)
{
    bool inrun = false;
    int  count = 0;

    avtRay *ray = volume->GetRay(w, h);
    bool calc_cell_index = ((ncell_arrays > 0) || (ghosts != NULL));

    double dest_rgb[4] = {0.0,0.0,0.0, 0.0};     // to store the computed color
    for (int i = first ; i < last ; i++)
    {
        const int *ind = ind_buffer + 3*i;
        const double *prop = prop_buffer + 3*i;

        int index = 0;
        if (calc_cell_index)
            index = ind[2]*((dims[0]-1)*(dims[1]-1)) + ind[1]*(dims[0]-1) + ind[0];

        if (ghosts != NULL)
        {
            if (ghosts[index] != 0)
               valid_sample[i] = false;
        }

        if (!valid_sample[i] && inrun)
        {
            ray->SetSamples(i-count, i-1, tmpSampleList);
            inrun = false;
            count = 0;
        }


        //
        // Trilinear SETUP
        int indices[6];
        float dist_from_left, dist_from_right,  dist_from_top,dist_from_bottom,  dist_from_front, dist_from_back;
        float x_left, y_bottom, z_front;

        if (trilinearInterpolation)
        {
            int index_left, index_right,            index_top, index_bottom,         index_front, index_back;

            int newInd[3];
            newInd[0] = ind[0];
            newInd[1] = ind[1];
            newInd[2] = ind[2];

            float x_right = prop[0];        x_left   = 1. - x_right;
            float y_top   = prop[1];        y_bottom = 1. - y_top;
            float z_back  = prop[2];        z_front  = 1. - z_back;

            // get the index and distance from the center of the neighbouring cells
            getIndexandDistFromCenter(x_right, newInd[0], index_left, index_right,   dist_from_left, dist_from_right);
            getIndexandDistFromCenter(y_top,   newInd[1], index_bottom,index_top,    dist_from_bottom,dist_from_top);
            getIndexandDistFromCenter(z_back,  newInd[2], index_front, index_back,   dist_from_front, dist_from_back);


            indices[4] = index_front;       indices[5] = index_back;
            indices[2] = index_bottom;      indices[3] = index_top;
            indices[0] = index_left;        indices[1] = index_right;


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
        }

        if (!valid_sample[i])
            continue;

        //
        // Trilinear RUN
        if (trilinearInterpolation)
        {
            //
            // Cell centered data
            //
            if (ncell_arrays > 0){
                int indexT[8];
                computeIndices(dims, indices, indexT);

                for (int l = 0 ; l < ncell_arrays ; l++)            // ncell_arrays: usually 1
                {

                    void  *cell_array = cell_arrays[l];
                    double values[8];
                    for (int m = 0 ; m < cell_size[l] ; m++){       // cell_size[l] usually 1
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
        }
        else
        {
            if (ncell_arrays > 0)
            {
                for (int l = 0 ; l < ncell_arrays ; l++)
                {
                    for (int m = 0 ; m < cell_size[l] ; m++)
                        tmpSampleList[count][cell_index[l]+m] =
                                         ConvertToDouble(cell_vartypes[l], index,
                                                      cell_size[l], m, cell_arrays[l]);
                }
            }

            if (npt_arrays > 0)
            {
                int index[8];
                index[0] = (ind[2])  *dims[0]*dims[1] + (ind[1])  *dims[0] + (ind[0]);
                index[1] = (ind[2])  *dims[0]*dims[1] + (ind[1])  *dims[0] + (ind[0]+1);
                index[2] = (ind[2])  *dims[0]*dims[1] + (ind[1]+1)*dims[0] + (ind[0]);
                index[3] = (ind[2])  *dims[0]*dims[1] + (ind[1]+1)*dims[0] + (ind[0]+1);
                index[4] = (ind[2]+1)*dims[0]*dims[1] + (ind[1])  *dims[0] + (ind[0]);
                index[5] = (ind[2]+1)*dims[0]*dims[1] + (ind[1])  *dims[0] + (ind[0]+1);
                index[6] = (ind[2]+1)*dims[0]*dims[1] + (ind[1]+1)*dims[0] + (ind[0]);
                index[7] = (ind[2]+1)*dims[0]*dims[1] + (ind[1]+1)*dims[0] + (ind[0]+1);

                double x_right = prop[0];
                double x_left = 1. - prop[0];
                double y_top = prop[1];
                double y_bottom = 1. - prop[1];
                double z_back = prop[2];
                double z_front = 1. - prop[2];
                for (int l = 0 ; l < npt_arrays ; l++)
                {
                    void  *pt_array = pt_arrays[l];
                    int    s = pt_size[l];
                    for (int m = 0 ; m < s ; m++)
                    {
                        double vals[8];
                        AssignEight(pt_vartypes[l], vals, index, s, m, pt_array);
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
//  Method: avtSLIVRVoxelExtractor::SampleAlongSegment
//
//  Purpose:
//      Samples the grid along a line segment.
//
//  Programmer: Hank Childs
//  Creation:   November 20, 2004
//
//  Modifications:
//
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

void
avtSLIVRVoxelExtractor::SampleAlongSegment(const double *origin,
                                          const double *terminus, int w, int h)
{
    int first = 0;
    int last = 0;
    bool hasIntersections = FindSegmentIntersections(origin, terminus,
                                                     first, last);

    if (!hasIntersections)
        return;

    //
    // Determine if there is intersection with buffer
    bool rcSLIVRBufferMerge = false;
    double _worldOpaqueCoordinates[3];
    bool intesecting = false;
    int intersect = -1;
    float posAlongVector = 0;

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

    if (rayCastingSLIVR)
    {
        int screenX = bufferExtents[1] - bufferExtents[0];
        int screenY = bufferExtents[3] - bufferExtents[2];

        int _index = h*screenX + w;

        if (depthBuffer[_index] != 1)  // There is some other things to blend with at this location ...
        {
            float normalizedDepth = depthBuffer[_index]*2 - 1;  // switching from (0 - 1) to (-1 - 1)

            //debug5 << "normalizedDepth: " <<normalizedDepth << "  renderingDepthsExtents[0]: " << renderingDepthsExtents[0] << "   renderingDepthsExtents[1]:" << renderingDepthsExtents[1] << std::endl;
            if ( (normalizedDepth >= renderingDepthsExtents[0]) && (normalizedDepth <= renderingDepthsExtents[1]) )  // ... and it's within this patch
            {
                rcSLIVRBufferMerge = true;
                unProject(w,h, depthBuffer[_index], _worldOpaqueCoordinates, fullImgWidth, fullImgHeight);
                //debug5 << "Location: " << w << ", " << h << std::endl;

                double start[3];
                start[0] = terminus[0];
                start[1] = terminus[1];
                start[2] = terminus[2];
                if (xGoingUp)
                    start[0] = origin[0];

                if (yGoingUp)
                    start[1] = origin[1];

                if (zGoingUp)
                    start[1] = origin[2];

                float distOriginTerminus_Squared = (origin[0]-terminus[0])*(origin[0]-terminus[0]) + (origin[1]-terminus[1])*(origin[1]-terminus[1]) + (origin[2]-terminus[2])*(origin[2]-terminus[2]);
                float distCoordStart_Squared = (_worldOpaqueCoordinates[0]-start[0])*(_worldOpaqueCoordinates[0]-start[0]) + (_worldOpaqueCoordinates[1]-start[1])*(_worldOpaqueCoordinates[1]-start[1]) + (_worldOpaqueCoordinates[2]-start[2])*(_worldOpaqueCoordinates[2]-start[2]);


                if (distCoordStart_Squared < distOriginTerminus_Squared)     // lies along the vector
                {
                    intesecting = true;
                    posAlongVector = sqrt(distCoordStart_Squared)/sqrt(distOriginTerminus_Squared);
                }
            }
        }
    }




    for (int i = first ; i < last ; i++)
    {
        int *ind = ind_buffer + 3*i;
        double *dProp = prop_buffer + 3*i;
        valid_sample[i] = false;

        double proportion = ((double)i)/((double)depth);
        pt[0] = origin[0] + proportion*x_dist;
        pt[1] = origin[1] + proportion*y_dist;
        pt[2] = origin[2] + proportion*z_dist;

        ind[0] = -1;
        ind[1] = -1;
        ind[2] = -1;

        if (!foundHit)
        {
            //
            // We haven't found any hits previously.  Exhaustively search
            // through arrays and try to find a hit.
            //
            ind[0] = FindMatch(X, pt[0], dims[0]);
            if (ind[0] >= 0)
                dProp[0] = (pt[0] - X[ind[0]]) * divisors_X[ind[0]];
            ind[1] = FindMatch(Y, pt[1], dims[1]);
            if (ind[1] >= 0)
                dProp[1] = (pt[1] - Y[ind[1]]) * divisors_Y[ind[1]];
            ind[2] = FindMatch(Z, pt[2], dims[2]);
            if (ind[2] >= 0)
                dProp[2] = (pt[2] - Z[ind[2]]) * divisors_Z[ind[2]];
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
                        dProp[0] = (pt[0] - X[curX]) * divisors_X[curX];
                        ind[0] = curX;
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
                        dProp[0] = (pt[0] - X[curX]) * divisors_X[curX];
                        ind[0] = curX;
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
                        dProp[1] = (pt[1] - Y[curY]) * divisors_Y[curY];
                        ind[1] = curY;
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
                        dProp[1] = (pt[1] - Y[curY]) * divisors_Y[curY];
                        ind[1] = curY;
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
                        dProp[2] = (pt[2] - Z[curZ]) * divisors_Z[curZ];
                        ind[2] = curZ;
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
                        dProp[2] = (pt[2] - Z[curZ]) * divisors_Z[curZ];
                        ind[2] = curZ;
                        break;
                    }
                }
            }
        }

        bool intersectedDataset = !(ind[0] < 0 || ind[1] < 0 || ind[2] < 0);
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

        curX = ind[0];
        curY = ind[1];
        curZ = ind[2];
    }

    //debug5 << "First: " << first << "  last: " << last << std::endl;

    if (intesecting){
        intersect = posAlongVector * (last-first) + first;
        debug5 << "intersect: " << intersect << "   First: " << first << "  last: " << last << std::endl;
    }

    if (hasSamples)
    {
        if (rayCastingSLIVR)
            SampleVariableRCSLIVR(first, last, intersect, w, h);
        else
            SampleVariable(first, last, w, h);
    }
}


// ****************************************************************************
//  Method: avtSLIVRVoxelExtractor::ExtractWorldSpaceGridRCSLIVR
//
//  Purpose:
//      Compute region that patch covers
//
//  Programmer: Pascal Grosset
//  Creation:   August 14, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtSLIVRVoxelExtractor::ExtractWorldSpaceGridRCSLIVR(vtkRectilinearGrid *rgrid,
                 std::vector<std::string> &varnames, std::vector<int> &varsize)
{
    patchDrawn = 0;
    debug5 << "materialProperties:" << materialProperties[0] << ", " << materialProperties[1] << ", " << materialProperties[2] << ", " << materialProperties[3] << std::endl;

    //
    // Some of our sampling routines need a chance to pre-process the data.
    // Register the grid here so we can do that.
    //
    RegisterGrid(rgrid, varnames, varsize);   // stores the values in a structure so that it can be used

    //
    // Determine what range we are dealing with on this iteration.
    //
    int w_min = restrictedMinWidth;
    int w_max = restrictedMaxWidth+1;
    int h_min = restrictedMinHeight;
    int h_max = restrictedMaxHeight+1;

    imgWidth = imgHeight = 0;


    //
    // Let's find out if this range can even intersect the dataset.
    // If not, just skip it.
    //
    if (!FrustumIntersectsGrid(w_min, w_max, h_min, h_max))
       return;

    //
    // Determine the screen size of the patch being processed
    //
    xMin = yMin = std::numeric_limits<int>::max();
    xMax = yMax = std::numeric_limits<int>::min();

    float coordinates[8][3];
    coordinates[0][0] = X[0];           coordinates[0][1] = Y[0];           coordinates[0][2] = Z[0];
    coordinates[1][0] = X[dims[0]-1];   coordinates[1][1] = Y[0];           coordinates[1][2] = Z[0];
    coordinates[2][0] = X[dims[0]-1];   coordinates[2][1] = Y[dims[1]-1];   coordinates[2][2] = Z[0];
    coordinates[3][0] = X[0];           coordinates[3][1] = Y[dims[1]-1];   coordinates[3][2] = Z[0];

    coordinates[4][0] = X[0];           coordinates[4][1] = Y[0];           coordinates[4][2] = Z[dims[2]-1];
    coordinates[5][0] = X[dims[0]-1];   coordinates[5][1] = Y[0];           coordinates[5][2] = Z[dims[2]-1];
    coordinates[6][0] = X[dims[0]-1];   coordinates[6][1] = Y[dims[1]-1];   coordinates[6][2] = Z[dims[2]-1];
    coordinates[7][0] = X[0];           coordinates[7][1] = Y[dims[1]-1];   coordinates[7][2] = Z[dims[2]-1];

    //debug5 << "Extents - Min: " << X[0] << ", " << Y[0] << ", " << Z[0] << "   Max: " << X[dims[0]-1] << ", " << Y[dims[1]-1] << ", " << Z[dims[2]-1] << "    ";


    //
    // Compute z order for blending patches
    double _center[3];
    _center[0] = (X[0] + X[dims[0]-1])/2.0;
    _center[1] = (Y[0] + Y[dims[1]-1])/2.0;
    _center[2] = (Z[0] + Z[dims[2]-1])/2.0;
    double __depth = sqrt( (_center[0]-view.camera[0])*(_center[0]-view.camera[0]) +  (_center[1]-view.camera[1])*(_center[1]-view.camera[1]) + (_center[2]-view.camera[2])*(_center[2]-view.camera[2]) );
    eyeSpaceDepth = __depth;



    double _clipSpaceZ = 0;
    double _world[3];
    for (int i=0; i<8; i++)
    {
        int pos2D[2];
        float tempZ;

        _world[0] = coordinates[i][0];
        _world[1] = coordinates[i][1];
        _world[2] = coordinates[i][2];

        tempZ = project(_world, pos2D, fullImgWidth, fullImgHeight);

        // Clamp values
        pos2D[0] = std::min( std::max(pos2D[0], 0), w_max-1);
        pos2D[0] = std::min( std::max(pos2D[0], 0), w_max-1);
        pos2D[1] = std::min( std::max(pos2D[1], 0), h_max-1);
        pos2D[1] = std::min( std::max(pos2D[1], 0), h_max-1);

        // Get min max
        xMin = std::min(xMin, pos2D[0]);
        xMax = std::max(xMax, pos2D[0]);
        yMin = std::min(yMin, pos2D[1]);
        yMax = std::max(yMax, pos2D[1]);


        if (i == 0)
        {
            _clipSpaceZ = tempZ;
            renderingDepthsExtents[0] = tempZ;
            renderingDepthsExtents[1] = tempZ;
        }
        else
        {
            if ( _clipSpaceZ > tempZ )
                _clipSpaceZ = tempZ;

            if (renderingDepthsExtents[0] > tempZ)      // min z
                renderingDepthsExtents[0] = tempZ;

            if (renderingDepthsExtents[1] < tempZ)      // max z
                renderingDepthsExtents[1] = tempZ;
        };
    }

    renderingAreaExtents[0] = xMin;
    renderingAreaExtents[1] = xMax;
    renderingAreaExtents[2] = yMin;
    renderingAreaExtents[3] = yMax;

    clipSpaceDepth = _clipSpaceZ;

    imgWidth  = xMax-xMin;
    imgHeight = yMax-yMin;


    //debug5 << "Initialize memory" << std::endl;

    //
    // Initialize memory
    imgArray =  new float[((imgWidth)*4) * imgHeight]();   // image

    //
    // Send rays
    imgDims[0] = imgWidth;       imgDims[1] = imgHeight;
    imgLowerLeft[0] = xMin;      imgLowerLeft[1] = yMin;
    imgUpperRight[0] = xMax;     imgUpperRight[1] = yMax;

    //debug5 << "Send rays ~ screen:" << xMin << ", " << xMax << "    "  << yMin << ", " << yMax <<  "    " << renderingDepthsExtents[0] << ", " << renderingDepthsExtents[1] <<  "   Buffer extents: " << bufferExtents[0] << ", " << bufferExtents[1] << "   " << bufferExtents[2] << ", " << bufferExtents[3] << std::endl;

    for (int _x = xMin ; _x < xMax ; _x++)
        for (int _y = yMin ; _y < yMax ; _y++)
        {
            int index = (_y-yMin)*imgWidth + (_x-xMin);


            if ( (scalarRange[1] < tFVisibleRange[0]) || (scalarRange[0] > tFVisibleRange[1]) )     // outside visible range
            {
                int fullIndex = ( (_y-bufferExtents[2]) * (bufferExtents[1]-bufferExtents[0]) + (_x-bufferExtents[0]) );

                if ( depthBuffer[fullIndex] != 1)
                {
                    double clipDepth = depthBuffer[fullIndex]*2 - 1;

                    if ( clipDepth >= renderingDepthsExtents[0] && clipDepth < renderingDepthsExtents[1])
                    {
                        patchDrawn = 1;

                        imgArray[(_y-yMin)*(imgWidth*4) + (_x-xMin)*4 + 0] = rgbColorBuffer[fullIndex*3 + 0] / 255.0;
                        imgArray[(_y-yMin)*(imgWidth*4) + (_x-xMin)*4 + 1] = rgbColorBuffer[fullIndex*3 + 1] / 255.0;
                        imgArray[(_y-yMin)*(imgWidth*4) + (_x-xMin)*4 + 2] = rgbColorBuffer[fullIndex*3 + 2] / 255.0;
                        imgArray[(_y-yMin)*(imgWidth*4) + (_x-xMin)*4 + 3] = 1.0;
                    }
                }
            }
            else
            {
                double _origin[3], _terminus[3];
                double origin[4]  = {0,0,0,1};      // starting point where we start sampling
                double terminus[4]= {0,0,0,1};      // ending point where we stop sampling

                GetSegmentRCSLIVR(_x, _y, fullVolumeDepthExtents, _origin, _terminus);    // find the starting point & ending point of the ray

                for (int i=0; i<3; i++){
                    origin[i] = _origin[i];
                    terminus[i] = _terminus[i];
                }

                SampleAlongSegment(origin, terminus, _x, _y);             // Go get the segments along this ray and store them in
            }
        }

    //
    // Deallocate memory if not used
    if (patchDrawn == 0)
    {
        if (imgArray != NULL)
            delete []imgArray;

        imgArray = NULL;
    }
}


// ****************************************************************************
//  Method: avtSLIVRVoxelExtractor::GetSegmentRCSLIVR
//
//  Purpose:
//      Determine ray start and end
//
//  Programmer: Pascal Grosset
//  Creation:   August 14, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtSLIVRVoxelExtractor::GetSegmentRCSLIVR(int x, int y, double depthsExtents[2], double *_origin, double *_terminus)
{
    unProject(x,y, depthsExtents[0], _origin,   fullImgWidth, fullImgHeight);
    unProject(x,y, depthsExtents[1], _terminus, fullImgWidth, fullImgHeight);
}




// ****************************************************************************
//  Method: avtSLIVRVoxelExtractor::SampleVariableRCSLIVR
//
//  Purpose:
//      Sample each ray
//
//  Programmer: Pascal Grosset
//  Creation:   August 14, 2016
//
//
// ****************************************************************************

void
avtSLIVRVoxelExtractor::SampleVariableRCSLIVR(int first, int last, int intersect, int x, int y)
{
    int  count = 0;
    bool calc_cell_index = ((ncell_arrays > 0) || (ghosts != NULL));

    double dest_rgb[4] = {0.0,0.0,0.0, 0.0};     // to store the computed color
    for (int i = first ; i < last ; i++)
    {
        // If we intersect a value in the z buffer
        if (i == intersect)
        {
            int fullIndex = (y * (bufferExtents[1]-bufferExtents[0]) + x) * 3.0;

            float bufferColor[4];
            bufferColor[0] = rgbColorBuffer[fullIndex + 0] / 255.0;
            bufferColor[1] = rgbColorBuffer[fullIndex + 1] / 255.0;
            bufferColor[2] = rgbColorBuffer[fullIndex + 2] / 255.0;
            bufferColor[3] = 1.0;

            for (int j=0; j<4; j++)
                dest_rgb[j] = bufferColor[j] * (1.0 - dest_rgb[3]) + dest_rgb[j];

            //debug5 << x << ", " << y << "   ~ First: " << first << "  i:  " << i << "   intersect: " << intersect << "  bufferColor: " << bufferColor[0] << ", " << bufferColor[1] << ", " << bufferColor[2] << "   dest_rgb: " << dest_rgb[0] << ", " << dest_rgb[1] << ", " << dest_rgb[2] << ", " << dest_rgb[3] << std::endl;
            break;
        }

        const int *ind = ind_buffer + 3*i;
        const double *prop = prop_buffer + 3*i;

        int index = 0;
        if (calc_cell_index)
            index = ind[2]*((dims[0]-1)*(dims[1]-1)) + ind[1]*(dims[0]-1) + ind[0];

        if (ghosts != NULL)
        {
            if (ghosts[index] != 0)
               valid_sample[i] = false;
        }

        int index_left, index_right,            index_top, index_bottom,         index_front, index_back;
        float dist_from_left, dist_from_right,  dist_from_top,dist_from_bottom,  dist_from_front, dist_from_back;

        int newInd[3];
        newInd[0] = ind[0];
        newInd[1] = ind[1];
        newInd[2] = ind[2];

        float x_right = prop[0];        float x_left   = 1. - x_right;
        float y_top   = prop[1];        float y_bottom = 1. - y_top;
        float z_back  = prop[2];        float z_front  = 1. - z_back;

        //std::cout << "x_right: " << x_right <<  "  x_left: " << x_left << "   y_top:" <<  y_top << "  y_bottom: " <<  "   z_back: " << z_back<< "  z_front: " <<  z_front << std::endl;
        

        // get the index and distance from the center of the neighbouring cells
        getIndexandDistFromCenter(x_right, newInd[0], index_left, index_right,   dist_from_left, dist_from_right);
        getIndexandDistFromCenter(y_top,   newInd[1], index_bottom,index_top,    dist_from_bottom,dist_from_top);
        getIndexandDistFromCenter(z_back,  newInd[2], index_front, index_back,   dist_from_front, dist_from_back);

        int indices[6];
        indices[4] = index_front;       indices[5] = index_back;
        indices[2] = index_bottom;      indices[3] = index_top;
        indices[0] = index_left;        indices[1] = index_right;


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


        if (!valid_sample[i])
            continue;


        //
        // Cell centered data
        //
        if (ncell_arrays > 0)
        {
            int indexT[8];
            computeIndices(dims, indices, indexT);

            for (int l = 0 ; l < ncell_arrays ; l++)            // ncell_arrays: usually 1
            {
                void  *cell_array = cell_arrays[l];
                double values[8];
                for (int m = 0 ; m < cell_size[l] ; m++)        // cell_size[l] usually 1
                {
                    AssignEight(cell_vartypes[l], values, indexT, cell_size[l], m, cell_array);
                    double scalarValue = trilinearInterpolate(values, dist_from_left, dist_from_bottom, dist_from_front);
                    double source_rgb[4];
                    int retVal = transferFn1D->QueryTF(scalarValue,source_rgb);

                    if ( ((retVal == 0)||(source_rgb[3]==0)) || (source_rgb[0]==0 && source_rgb[1]==0 && source_rgb[2]==0) )
                    {
                        // no need to do anything more if there will be no color
                    }
                    else
                    {
                        //
                        // Compute Lighting (if needed)
                        //
                        if (lighting == true)
                        {
                            double vals[6];

                            // h = offset = 1/2 the distance between grids
                            // grad = 1/2*h * ( f(x+h,y,z)-f(x-h,y,z)    f(x,y+h,z)-f(x,y-h,z)   f(x,y,z-h)-f(x,y,z-h)  )

                            float distFromRight, distFromLeft, distFromTop, distFromBottom, distFromFront, distFromBack;
                            int indexLeft, indexRight, indexTop, indexBottom, indexFront, indexBack;
                            float gradientOffset = 0.25;

                            double gradVals[8];
                            int indexGrad[8], gradInd[3], gradIndices[6];
                            float xRight, yTop, zBack;

                            void  *cell_array = cell_arrays[0];

                            //
                            // X
                            //
                            for (int i=0; i<6; i++)
                                gradIndices[i] = indices[i];

                            //
                            // find x-h
                            //
                            if (x_right - gradientOffset < 0.0){
                                xRight = (x_right - gradientOffset)+1.0;
                                gradInd[0] = ind[0]-1;
                            }
                            else{
                                xRight = x_right - gradientOffset;
                                gradInd[0] = ind[0];
                            }

                            getIndexandDistFromCenter(xRight, gradInd[0],  indexLeft, indexRight,  distFromLeft, distFromRight);
                            gradIndices[0] = indexLeft;    gradIndices[1] = indexRight;
                            computeIndices(dims, gradIndices, indexGrad);
                            AssignEight(cell_vartypes[0], gradVals, indexGrad, 1, 0, cell_array);
                            vals[0] = trilinearInterpolate(gradVals, distFromLeft, dist_from_bottom, dist_from_front);

                            //
                            // find x+h
                            //
                            if (x_right + gradientOffset > 1.0){
                                xRight = (x_right + gradientOffset)-1.0;
                                gradInd[0] = ind[0]+1;
                            }else{
                                xRight = x_right + gradientOffset;
                                gradInd[0] = ind[0];
                            }

                            getIndexandDistFromCenter(xRight, gradInd[0],  indexLeft, indexRight,  distFromLeft, distFromRight);
                            gradIndices[0] = indexLeft;    gradIndices[1] = indexRight;
                            computeIndices(dims, gradIndices, indexGrad);
                            AssignEight(cell_vartypes[0], gradVals, indexGrad, 1, 0, cell_array);
                            vals[1] = trilinearInterpolate(gradVals, distFromLeft, dist_from_bottom, dist_from_front);



                            //
                            // Y
                            //
                            for (int i=0; i<6; i++)
                                gradIndices[i] = indices[i];

                            //
                            // find y-h
                            //
                            if (y_top - gradientOffset < 0.0){
                                yTop = (y_top - gradientOffset)+1.0;
                                gradInd[1] = ind[1]-1;
                            }
                            else{
                                yTop = y_top - gradientOffset;
                                gradInd[1] = ind[1];
                            }

                            getIndexandDistFromCenter(yTop, gradInd[1],  indexBottom, indexTop,  distFromBottom, distFromTop);
                            gradIndices[2] = indexBottom ;    gradIndices[3] = indexTop;
                            computeIndices(dims, gradIndices, indexGrad);
                            AssignEight(cell_vartypes[0], gradVals, indexGrad, 1, 0, cell_array);
                            vals[2] = trilinearInterpolate(gradVals, dist_from_left, distFromBottom, dist_from_front);

                            //
                            // find y+h
                            //
                            yTop = y_top;
                            if (y_top + gradientOffset > 1.0){
                                yTop = (y_top + gradientOffset)-1.0;
                                gradInd[1] = ind[1]+1;
                            }else{
                                yTop = y_top + gradientOffset;
                                gradInd[1] = ind[1];
                            }

                            getIndexandDistFromCenter(yTop, gradInd[1],  indexBottom, indexTop,  distFromBottom, distFromTop);
                            gradIndices[2] = indexBottom;    gradIndices[3] = indexTop;
                            computeIndices(dims, gradIndices, indexGrad);
                            AssignEight(cell_vartypes[0], gradVals, indexGrad, 1, 0, cell_array);
                            vals[3] = trilinearInterpolate(gradVals, dist_from_left, distFromBottom, dist_from_front);


                            //
                            // Z
                            //
                            for (int i=0; i<6; i++)
                                gradIndices[i] = indices[i];

                            //
                            // z-h
                            //
                            if (z_back - gradientOffset < 0.0){
                                zBack = (z_back - gradientOffset)+1.0;
                                gradInd[2] = ind[2]-1;
                            }
                            else{
                                zBack = z_back - gradientOffset;
                                gradInd[2] = ind[2];
                            }

                            getIndexandDistFromCenter(zBack, gradInd[2],  indexFront, indexBack,  distFromFront, distFromBack);
                            gradIndices[4] = indexFront;    gradIndices[5] = indexBack;
                            computeIndices(dims, gradIndices, indexGrad);
                            AssignEight(cell_vartypes[0], gradVals, indexGrad, 1, 0, cell_array);
                            vals[4] = trilinearInterpolate(gradVals, dist_from_left, dist_from_bottom, distFromFront);

                            //
                            // z+h
                            //
                            if (z_back + gradientOffset > 1.0){
                                zBack = (z_back + gradientOffset)-1.0;
                                gradInd[2] = ind[2]+1;
                            }else{
                                zBack = z_back + gradientOffset;
                                gradInd[2] = ind[2];
                            }

                            getIndexandDistFromCenter(zBack, gradInd[2],  indexFront, indexBack,  distFromFront, distFromBack);
                            gradIndices[4] = indexFront;    gradIndices[5] = indexBack;
                            computeIndices(dims, gradIndices, indexGrad);
                            AssignEight(cell_vartypes[0], gradVals, indexGrad, 1, 0, cell_array);
                            vals[5] = trilinearInterpolate(gradVals, dist_from_left, dist_from_bottom, distFromFront);


                            gradient[0] = (1.0/(2.0*gradientOffset)) * (vals[1] - vals[0]);
                            gradient[1] = (1.0/(2.0*gradientOffset)) * (vals[3] - vals[2]);
                            gradient[2] = (1.0/(2.0*gradientOffset)) * (vals[5] - vals[4]);

                            normalize(gradient);
                        }

                        //
                        // Compute the color
                        //
                        computePixelColor(source_rgb, dest_rgb, gradient);
                    }

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
                    double source_rgb[4];
                    int retVal = transferFn1D->QueryTF(scalarValue,source_rgb);
                    if ( ((retVal == 0)||(source_rgb[3]==0)) || (source_rgb[0]==0 && source_rgb[1]==0 && source_rgb[2]==0) )
                    {
                        // no need to do anything more if there will be no color
                    }
                    else
                    {
                        //
                        // Compute Lighting (if needed)
                        //
                        if (lighting == true)
                        {
                            double vals[6];

                            // h = offset = 1/2 the distance between grids
                            // grad = 1/2*h * ( f(x+h,y,z)-f(x-h,y,z)    f(x,y+h,z)-f(x,y-h,z)   f(x,y,z-h)-f(x,y,z-h)  )

                            float distFromRight, distFromLeft, distFromTop, distFromBottom, distFromFront, distFromBack;
                            int indexLeft, indexRight, indexTop, indexBottom, indexFront, indexBack;
                            float gradientOffset = 0.5;

                            double gradVals[8];
                            int indexGrad[8], gradInd[3], gradIndices[6];
                            float xRight, yTop, zBack = 0;



                            //
                            // X
                            //
                            for (int i=0; i<6; i++)
                                gradIndices[i] = indices[i];

                            //
                            // find x-h
                            //
                            if (x_right - gradientOffset < 0.0){
                                xRight = (x_right - gradientOffset)+1.0;
                                gradInd[0] = ind[0]-1;
                            }
                            else{
                                xRight = x_right - gradientOffset;
                                gradInd[0] = ind[0];
                            }

                            getIndexandDistFromCenter(xRight, gradInd[0],  indexLeft, indexRight,  distFromLeft, distFromRight);
                            gradIndices[0] = indexLeft;    gradIndices[1] = indexRight;
                            computeIndicesVert(dims, gradIndices, indexGrad);
                            AssignEight(pt_vartypes[0], gradVals, indexGrad, 1, 0, pt_array);
                            vals[0] = trilinearInterpolate(gradVals, x_left-gradientOffset, y_bottom, z_front);

                            //
                            // find x+h
                            //
                            if (x_right + gradientOffset > 1.0){
                                xRight = (x_right + gradientOffset)-1.0;
                                gradInd[0] = ind[0]+1;
                            }else{
                                xRight = x_right + gradientOffset;
                                gradInd[0] = ind[0];
                            }

                            getIndexandDistFromCenter(xRight, gradInd[0],  indexLeft, indexRight,  distFromLeft, distFromRight);
                            gradIndices[0] = indexLeft;    gradIndices[1] = indexRight;
                            computeIndices(dims, gradIndices, indexGrad);
                            AssignEight(pt_vartypes[0], gradVals, indexGrad, 1, 0, pt_array);
                            vals[1] = trilinearInterpolate(gradVals, x_left+gradientOffset, y_bottom, z_front);



                            //
                            // Y
                            //
                            for (int i=0; i<6; i++)
                                gradIndices[i] = indices[i];

                            //
                            // find y-h
                            //
                            if (y_top - gradientOffset < 0.0){
                                yTop = (y_top - gradientOffset)+1.0;
                                gradInd[1] = ind[1]-1;
                            }
                            else{
                                yTop = y_top - gradientOffset;
                                gradInd[1] = ind[1];
                            }

                            getIndexandDistFromCenter(yTop, gradInd[1],  indexBottom, indexTop,  distFromBottom, distFromTop);
                            gradIndices[2] = indexBottom ;    gradIndices[3] = indexTop;
                            computeIndices(dims, gradIndices, indexGrad);
                            AssignEight(pt_vartypes[0], gradVals, indexGrad, 1, 0, pt_array);
                            vals[2] = trilinearInterpolate(gradVals, x_left, y_bottom-gradientOffset, z_front);

                            //
                            // find y+h
                            //
                            yTop = y_top;
                            if (y_top + gradientOffset > 1.0){
                                yTop = (y_top + gradientOffset)-1.0;
                                gradInd[1] = ind[1]+1;
                            }else{
                                yTop = y_top + gradientOffset;
                                gradInd[1] = ind[1];
                            }

                            getIndexandDistFromCenter(yTop, gradInd[1],  indexBottom, indexTop,  distFromBottom, distFromTop);
                            gradIndices[2] = indexBottom;    gradIndices[3] = indexTop;
                            computeIndices(dims, gradIndices, indexGrad);
                            AssignEight(pt_vartypes[0], gradVals, indexGrad, 1, 0, pt_array);
                            vals[3] = trilinearInterpolate(gradVals, x_left, y_bottom+gradientOffset, z_front);


                            //
                            // Z
                            //
                            for (int i=0; i<6; i++)
                                gradIndices[i] = indices[i];

                            //
                            // z-h

                            if (z_back - gradientOffset < 0.0){
                                zBack = (z_back - gradientOffset)+1.0;
                                gradInd[2] = ind[2]-1;
                            }
                            else
                            {
                                zBack = z_back - gradientOffset;
                                gradInd[2] = ind[2];
                            }

                            getIndexandDistFromCenter(zBack, gradInd[2],  indexFront, indexBack,  distFromFront, distFromBack);
                            gradIndices[4] = indexFront;    gradIndices[5] = indexBack;
                            computeIndices(dims, gradIndices, indexGrad);
                            AssignEight(pt_vartypes[0], gradVals, indexGrad, 1, 0, pt_array);
                            vals[4] = trilinearInterpolate(gradVals, x_left, y_bottom, z_front-gradientOffset);

                            //
                            // z+h
                            //
                            if (z_back + gradientOffset > 1.0){
                                zBack = (z_back + gradientOffset)-1.0;
                                gradInd[2] = ind[2]+1;
                            }else{
                                zBack = z_back + gradientOffset;
                                gradInd[2] = ind[2];
                            }

                            getIndexandDistFromCenter(zBack, gradInd[2],  indexFront, indexBack,  distFromFront, distFromBack);
                            gradIndices[4] = indexFront;    gradIndices[5] = indexBack;
                            computeIndices(dims, gradIndices, indexGrad);
                            AssignEight(pt_vartypes[0], gradVals, indexGrad, 1, 0, pt_array);
                            vals[5] = trilinearInterpolate(gradVals, x_left, y_bottom, z_front+gradientOffset);


                            gradient[0] = (1.0/(2.0*gradientOffset)) * (vals[1] - vals[0]);
                            gradient[1] = (1.0/(2.0*gradientOffset)) * (vals[3] - vals[2]);
                            gradient[2] = (1.0/(2.0*gradientOffset)) * (vals[5] - vals[4]);

                            normalize(gradient);
                        }

                        //
                        // Compute the color
                        //
                        computePixelColor(source_rgb, dest_rgb, gradient);
                    }
                }
            }
        }
        count++;
    }

    //
    // Set color
    imgArray[(y-yMin)*(imgWidth*4) + (x-xMin)*4 + 0] = std::min(std::max(dest_rgb[0],0.0),1.0);
    imgArray[(y-yMin)*(imgWidth*4) + (x-xMin)*4 + 1] = std::min(std::max(dest_rgb[1],0.0),1.0);
    imgArray[(y-yMin)*(imgWidth*4) + (x-xMin)*4 + 2] = std::min(std::max(dest_rgb[2],0.0),1.0);
    imgArray[(y-yMin)*(imgWidth*4) + (x-xMin)*4 + 3] = std::min(std::max(dest_rgb[3],0.0),1.0);
}


// ****************************************************************************
//  Method: avtSLIVRVoxelExtractor::normalize
//
//  Purpose:
//      Normalize vector
//
//  Programmer: Pascal Grosset
//  Creation:   August 14, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtSLIVRVoxelExtractor::normalize(float vec[3])
{
    float inverse_sqrt_sum_squared = sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
    if (inverse_sqrt_sum_squared != 0)
        inverse_sqrt_sum_squared = 1.0/inverse_sqrt_sum_squared;

    for (int i=0;i<3; i++)
        vec[i] = vec[i]*inverse_sqrt_sum_squared;
}

// ****************************************************************************
//  Method: avtSLIVRVoxelExtractor::reflect
//
//  Purpose:
//      Reflect vector
//
//  Programmer: Pascal Grosset
//  Creation:   August 14, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtSLIVRVoxelExtractor::reflect(float vec[3], float normal[3], float refl[3])
{
    //Vnew = -2*(V dot N)*N + V
    //out = incidentVec - 2.f * Dot(incidentVec, normal) * normal;
    float vDotN = dot(vec,normal);
    for (int i=0; i<3; i++)
        refl[i] = -2.0 * vDotN * normal[i] + vec[i];

    normalize(refl);
}


// ****************************************************************************
//  Method: avtSLIVRVoxelExtractor::project
//
//  Purpose:
//      Convert from world coordinates to screen coordinates
//
//  Programmer: Pascal Grosset
//  Creation:   August 14, 2016
//
//  Modifications:
//
// ****************************************************************************

double
avtSLIVRVoxelExtractor::project(double _worldCoordinates[3], int pos2D[2], int _screenWidth, int _screenHeight)
{
    double normDevCoord[4];
    double worldCoordinates[4] = {0,0,0,1};
    worldCoordinates[0] = _worldCoordinates[0];
    worldCoordinates[1] = _worldCoordinates[1];
    worldCoordinates[2] = _worldCoordinates[2];

    // World to Clip space (-1 - 1)
    modelViewProj->MultiplyPoint(worldCoordinates, normDevCoord);

    if (normDevCoord[3] == 0)
    {
        debug5 << "avtSLIVRVoxelExtractor::project division by 0 error!" << endl;
        debug5 << "worldCoordinates: " << worldCoordinates[0] << ", " << worldCoordinates[1] << ", " << worldCoordinates[2] << "   " << normDevCoord[0] << ", " << normDevCoord[1] << ", " << normDevCoord[2] << endl;
        debug5 << "Matrix: " << *modelViewProj << endl;
    }

    // NDC
    normDevCoord[0] = normDevCoord[0]/normDevCoord[3];
    normDevCoord[1] = normDevCoord[1]/normDevCoord[3];
    normDevCoord[2] = normDevCoord[2]/normDevCoord[3];
    normDevCoord[3] = normDevCoord[3]/normDevCoord[3];

    // Screen coordinates
    pos2D[0] = round( normDevCoord[0]*(_screenWidth/2.)  + (_screenWidth/2.)  );
    pos2D[1] = round( normDevCoord[1]*(_screenHeight/2.) + (_screenHeight/2.) );


    // Add panning
    pos2D[0] += round(_screenWidth * panPercentage[0]);
    pos2D[1] += round(_screenHeight * panPercentage[1]); 

    return normDevCoord[2];
}

// ****************************************************************************
//  Method: avtSLIVRVoxelExtractor::unProject
//
//  Purpose:
//      Convert from screen coordinates to world coordinates
//
//  Programmer: Pascal Grosset
//  Creation:   August 14, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtSLIVRVoxelExtractor::unProject(int _x, int _y, float _z, double _worldCoordinates[3], int _width, int _height)
{
    // remove panning
    _x -= round(_width * panPercentage[0]);
    _y -= round(_height * panPercentage[1]); 

    double worldCoordinates[4] = {0,0,0,1};
    double in[4] = {0,0,0,1};
    in[0] = (_x - _width/2. )/(_width/2.);
    in[1] = (_y - _height/2.)/(_height/2.);
    in[2] = _z;

    invModelViewProj->MultiplyPoint(in, worldCoordinates);

    if (worldCoordinates[3] == 0)
    {
        debug5 << "avtSLIVRVoxelExtractor::unProject division by 0 error!" << endl;
    }

    worldCoordinates[0] = worldCoordinates[0]/worldCoordinates[3];
    worldCoordinates[1] = worldCoordinates[1]/worldCoordinates[3];
    worldCoordinates[2] = worldCoordinates[2]/worldCoordinates[3];
    worldCoordinates[3] = worldCoordinates[3]/worldCoordinates[3];

    _worldCoordinates[0] = worldCoordinates[0];
    _worldCoordinates[1] = worldCoordinates[1];
    _worldCoordinates[2] = worldCoordinates[2];
}


// ****************************************************************************
//  Method: avtSLIVRVoxelExtractor::getImageDimensions
//
//  Purpose:
//      Transfers the metadata of the patch
//
//  Programmer: Pascal Grosset
//  Creation:   August 14, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtSLIVRVoxelExtractor::getImageDimensions(int &inUse, int dims[2], int screen_ll[2], int screen_ur[2], float &eyeDepth, float &clipDepth)
{
    inUse = patchDrawn;

    dims[0] = imgDims[0];    dims[1] = imgDims[1];

    screen_ll[0] = imgLowerLeft[0];     screen_ll[1] = imgLowerLeft[1];
    screen_ur[0] = imgUpperRight[0];    screen_ur[1] = imgUpperRight[1];

    eyeDepth = eyeSpaceDepth;
    clipDepth = clipSpaceDepth;
}


// ****************************************************************************
//  Method: avtSLIVRVoxelExtractor::getComputedImage
//
//  Purpose:
//      Allocates space to the pointer address and copy the image generated to it
//
//  Programmer: Pascal Grosset
//  Creation:   August 14, 2016
//
//  Modifications:
//
// ****************************************************************************

void
avtSLIVRVoxelExtractor::getComputedImage(float *image)
{
    memcpy(image, imgArray, imgDims[0]*4*imgDims[1]*sizeof(float));

    if (imgArray != NULL)
        delete []imgArray;
    imgArray = NULL;
}


// ****************************************************************************
//  Method: avtSLIVRVoxelExtractor::computePixelColor
//
//  Purpose:
//      Computes color
//      By replicating avtPhong::AddLightingHeadlight
//
//  Programmer: Pascal Grosset
//  Creation:   June 10, 2013
//
//  Modifications:
//      Need to take into account lighting
//      Need to take into accoujnt multiple light sources
//
// ****************************************************************************

void
avtSLIVRVoxelExtractor::computePixelColor(double source_rgb[4], double dest_rgb[4], float _gradient[3])
{
    if (dest_rgb[3] >= 0.99)
    {
        patchDrawn = 1;
        return;
    }

    // Phong Shading
    if (lighting == true)
    {
        float dir[3];           
        dir[0] = -view_direction[0];
        dir[1] = -view_direction[1];
        dir[2] = -view_direction[2];

        normalize(dir);

        double temp_rgb[4];
        temp_rgb[0] = source_rgb[0];
        temp_rgb[1] = source_rgb[1];
        temp_rgb[2] = source_rgb[2];
        temp_rgb[3] = source_rgb[3];

        // cos(angle) = a.b;  angle between normal and light
        float normal_dot_light = dot(_gradient,dir);   // angle between light and normal;
        if (normal_dot_light < 0)
            normal_dot_light = -normal_dot_light;

        debug5 << "normal_dot_light: " << normal_dot_light << "   _gradient: " << _gradient[0] << ", " << _gradient[1] << ", " << _gradient[2] << std::endl;
        // Calculate color using phong shading
        // I = (I  * ka) + [ (I_i  * kd * (L.N)) + (Ia_i * ks * (R.V)^ns) ]_for each light source i
        for (int i=0; i<3; i++)
        {
            source_rgb[i] =  ( (materialProperties[0] + materialProperties[1] * normal_dot_light)           * source_rgb[i] ) +     // I  * ( ka + kd*abs(cos(angle)) )
                               (materialProperties[2] * pow((double)normal_dot_light,materialProperties[3]) * source_rgb[3] )  ;    // I  * kd*abs(cos(angle))

        }
    }
    for (int i=0; i<4; i++)
    {
        // front to back compositing
        dest_rgb[i] = source_rgb[i] * (1.0 - dest_rgb[3]) + dest_rgb[i];

        // // back to front
        //  dest_rgb[i] = std::min( dest_rgb[i] * (1.0 - intermediate_rgb[3]) + intermediate_rgb[i], 1.0);
    }


    patchDrawn = 1;
}


