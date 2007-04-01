// ************************************************************************* //
//                            avtMassVoxelExtractor.C                        //
// ************************************************************************* //

#include <avtMassVoxelExtractor.h>

#include <float.h>

#include <avtCellList.h>
#include <avtVolume.h>

#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnsignedCharArray.h>


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
// ****************************************************************************

avtMassVoxelExtractor::avtMassVoxelExtractor(int w, int h, int d,
                                               avtVolume *vol, avtCellList *cl)
    : avtExtractor(w, h, d, vol, cl)
{
    ;
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
// ****************************************************************************

avtMassVoxelExtractor::~avtMassVoxelExtractor()
{
    ;
}


// ****************************************************************************
//  Method: avtMassVoxelExtractor::Extract
//
//  Purpose:
//      Extracts sample points from a collection of voxels.
//
//  Arguments:
//      rgrid     The rectilinear grid that contains the voxels.
//
//  Programmer:   Hank Childs
//  Creation:     December 14, 2003
//
// ****************************************************************************

inline int FindIndex(const float &pt, const int &last_hit, const int &n,
                     const float *vals)
{
    int i;

    for (i = last_hit ; i < n-1 ; i++)
    {
        if (pt >= vals[i] && (pt <= vals[i+1]))
            return i;
    }

    for (i = 0 ; i < last_hit ; i++)
    {
        if (pt >= vals[i] && (pt <= vals[i+1]))
            return i;
    }

    return -1;
}

// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Fri Aug 27 16:00:57 PDT 2004
//    Rename ghost data array.
//
// ****************************************************************************

void
avtMassVoxelExtractor::Extract(vtkRectilinearGrid *rgrid)
{
    int  i, j, k, l;

    int dims[3];
    rgrid->GetDimensions(dims);
    const int nX = dims[0];
    const int nY = dims[1];
    const int nZ = dims[2];

    float *x = (float *) rgrid->GetXCoordinates()->GetVoidPointer(0);
    float *y = (float *) rgrid->GetYCoordinates()->GetVoidPointer(0);
    float *z = (float *) rgrid->GetZCoordinates()->GetVoidPointer(0);

    int last_x_hit = 0;
    int last_y_hit = 0;
    int last_z_hit = 0;

    vtkUnsignedCharArray *ghosts = (vtkUnsignedCharArray *)rgrid->GetCellData()
                                                   ->GetArray("avtGhostZones");
    std::vector<float *> cell_arrays;
    for (i = 0 ; i < rgrid->GetCellData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = rgrid->GetCellData()->GetArray(i);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        cell_arrays.push_back((float *) arr->GetVoidPointer(0));
    }
    std::vector<float *> pt_arrays;
    for (i = 0 ; i < rgrid->GetPointData()->GetNumberOfArrays() ; i++)
    {
        vtkDataArray *arr = rgrid->GetPointData()->GetArray(i);
        if (strstr(arr->GetName(), "vtk") != NULL)
            continue;
        if (strstr(arr->GetName(), "avt") != NULL)
            continue;
        pt_arrays.push_back((float *) arr->GetVoidPointer(0));
    }

    int startX = SnapXLeft(x[0]);
    int stopX  = SnapXRight(x[nX-1]);
    int startY = SnapYBottom(y[0]);
    int stopY  = SnapYTop(y[nY-1]);
    int startZ = SnapZFront(z[0]);
    int stopZ  = SnapZBack(z[nZ-1]);
    for (j = startY ; j <= stopY ; j++)
    {
        float yc = YFromIndex(j);
        int yind = FindIndex(yc, last_y_hit, nY, y);
        if (yind == -1)
            continue;
        last_y_hit = yind;

        float y_bottom  = 0.;
        float y_top = 1.;
        if (pt_arrays.size() > 0)
        {
            float y_range = y[yind+1] - y[yind];
            y_bottom = 1. - (yc - y[yind])/y_range;
            y_top = 1. - y_bottom;
        }
        for (i = startX ; i <= stopX ; i++)
        {
            float xc = XFromIndex(i);
            int xind = FindIndex(xc, last_x_hit, nX, x);
            if (xind == -1)
                continue;
            last_x_hit = xind;

            float x_left  = 0.;
            float x_right = 1.;
            if (pt_arrays.size() > 0)
            {
                float x_range = x[xind+1] - x[xind];
                x_left = 1. - (xc - x[xind])/x_range;
                x_right = 1. - x_left;
            }

            last_z_hit = 0;
            int count = 0;
            int firstZ = -1;
            int lastZ  = stopZ;
            for (k = startZ ; k <= stopZ ; k++)
            {
                float zc = ZFromIndex(k);
                int zind = FindIndex(zc, last_z_hit, nZ, z);
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
                    int index = (zind)*nX*nY + (yind)*nX + (xind);
                    if (ghosts->GetValue(index) != 0)
                        continue;
                }

                float z_front  = 0.;
                float z_back = 1.;
                if (pt_arrays.size() > 0)
                {
                    float z_range = z[zind+1] - z[zind];
                    z_front = 1. - (zc - z[zind])/z_range;
                    z_back = 1. - z_front;
                }

                int var_index = 0;
                for (l = 0 ; l < cell_arrays.size() ; l++)
                {
                    int index = zind*((nX-1)*(nY-1)) + yind*(nX-1) + xind;
                    tmpSampleList[count][var_index++] = cell_arrays[l][index];
                }
                if (pt_arrays.size() > 0)
                {
                    int index0 = (zind)*nX*nY + (yind)*nX + (xind);
                    int index1 = (zind)*nX*nY + (yind)*nX + (xind+1);
                    int index2 = (zind)*nX*nY + (yind+1)*nX + (xind);
                    int index3 = (zind)*nX*nY + (yind+1)*nX + (xind+1);
                    int index4 = (zind+1)*nX*nY + (yind)*nX + (xind);
                    int index5 = (zind+1)*nX*nY + (yind)*nX + (xind+1);
                    int index6 = (zind+1)*nX*nY + (yind+1)*nX + (xind);
                    int index7 = (zind+1)*nX*nY + (yind+1)*nX + (xind+1);
                    for (l = 0 ; l < pt_arrays.size() ; l++)
                    {
                        float *pt_array = pt_arrays[l];
                        float val = 
                              x_left*y_bottom*z_front*pt_array[index0] +
                              x_right*y_bottom*z_front*pt_array[index1] +
                              x_left*y_top*z_front*pt_array[index2] +
                              x_right*y_top*z_front*pt_array[index3] +
                              x_left*y_bottom*z_back*pt_array[index4] +
                              x_right*y_bottom*z_back*pt_array[index5] +
                              x_left*y_top*z_back*pt_array[index6] +
                              x_right*y_top*z_back*pt_array[index7];
                        tmpSampleList[count][var_index++] = val;
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


