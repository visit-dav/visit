// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              avtVMetricVolume.C                           //
// ************************************************************************* //

#include "avtVMetricVolume.h"

#include <vtkCellType.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkRectilinearGrid.h>

#include <verdict.h>


double d_hex_volume(double coords[][3]);

inline
void Copy3(double coords[][3], double a[], int i)
{
    a[0] = coords[i][0];
    a[1] = coords[i][1];
    a[2] = coords[i][2];
}


// ****************************************************************************
//  Method: avtVMetricVolume constructor
//
//  Programmer: Hank Childs
//  Creation:   August 31, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Sep 15 09:23:50 PDT 2006
//    Added useVerdictHex flag, so that volume of hex can be calculated
//    differently if desired.
//
//    Eddie Rusu, Tue Jan 14 13:41:26 PST 2020
//    Set volume dependency to true for this metric.
//
// ****************************************************************************

avtVMetricVolume::avtVMetricVolume()
{
    useOnlyPositiveVolumes = false;
    useVerdictHex = true;
    volumeDependent->SetComponent(0, 0, true); // Set volume dependency to true
}


// ****************************************************************************
//  Method: avtVMetricVolume::Metric
//
//  Purpose:
//      Inspect an element and calculate the volume.
//
//  Arguments:
//      coords    The set of xyz points for the cell.
//      numPoints The number of xyz points for the cell.
//      type      The vtk type of the cell.
//
//  Returns:      The volume of the cell, or defaultValue if not supported.
//
//  Programmer:   Akira Haddox
//  Creation:     June 13, 2002
//
//  Modifications:
//
//    Hank Childs, Sat Aug 31 12:25:02 PDT 2002
//    Added support for taking the absolute volumes of values.
//
//    Hank Childs, Fri Sep  6 08:27:05 PDT 2002
//    Only the volume for a tetrahedron is 100% accurate.  Tetrahedralize the
//    remaining cell types to guarantee that we get 100% accurate results.
//
//    Hank Childs, Thu Oct 17 08:07:53 PDT 2002
//    Update for new verdict interface.  Remove previous code to tetrahedralize
//    hexahedrons, since that has now been incorporated into the Verdict
//    library.  Also fixed problem with wedge volumes where the tets we
//    were using were inverted.
//
//    Kathleen Bonnell, Fri Sep 15 09:55:55 PDT 2006 
//    Use different hex volume caluclation if useVerdictHex is false.
//
//    Eddie Rusu, Wed Feb 19 16:33:46 PST 2020
//    Returns 0 instead of -1 if no verdict or unchecked cell.
//    
// ****************************************************************************

double avtVMetricVolume::Metric (double coords[][3], int type)
{
    double rv = 0.;
#ifdef HAVE_VERDICT 
    switch (type)
    {
      case VTK_VOXEL:   // Note that the verdict filter already swapped the
                        // coordinates to make a voxel be like a hex.
      case VTK_HEXAHEDRON:
        if (useVerdictHex)
            rv = v_hex_volume(8,coords); 
        else
            rv = d_hex_volume(coords);
        break;
        
      case VTK_TETRA:
        rv = v_tet_volume(4,coords);
        break;

      case VTK_WEDGE:
        {
            int   subdiv[3][4] = { {0,5,4,3}, {0,2,1,4}, {0,4,5,2} };
            double tet_coords[4][3];
            for (int i = 0 ; i < 3 ; i++)
            {
                for (int j = 0 ; j < 4 ; j++)
                   for (int k = 0 ; k < 3 ; k++)
                       tet_coords[j][k] = coords[subdiv[i][j]][k];
                double temp = v_tet_volume(4, tet_coords);
                if (temp < 0. && useOnlyPositiveVolumes)
                {
                    temp *= -1.;
                }
                rv += temp;
            }
        }
        break;
        
      // The verdict metric for pyramid I have yet to figure out how to work.
      // However, it does the same thing that we do here: Divide the pyramid
      // into two tetrahedrons.
      case VTK_PYRAMID:
        double one[4][3];
        double two[4][3];
            
        Copy3(coords,one[0], 0);
        Copy3(coords,one[1], 1);
        Copy3(coords,one[2], 2);
        Copy3(coords,one[3], 4);

        Copy3(coords,two[0], 0);
        Copy3(coords,two[1], 2);
        Copy3(coords,two[2], 3);
        Copy3(coords,two[3], 4);

        rv = v_tet_volume(4,one) + v_tet_volume(4,two);
        break;
    }

    if (rv < 0. && useOnlyPositiveVolumes)
    {
        rv *= -1.;
    }

#endif
    return rv;
}


// ****************************************************************************
//  Method: avtVMetricVolume::OperateDirectlyOnMesh
//
//  Purpose:
//      Determines if we want to speed up the operation by operating directly
//      on the mesh.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
// ****************************************************************************

bool
avtVMetricVolume::OperateDirectlyOnMesh(vtkDataSet *ds)
{
    return (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID);
}


// ****************************************************************************
//  Method: avtVMetricVolume::MetricForWholeMesh
//
//  Purpose:
//      Determines the volume for each cell in the mesh.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2005
//
// ****************************************************************************

void
avtVMetricVolume::MetricForWholeMesh(vtkDataSet *ds, vtkDataArray *rv)
{
    if (ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
        EXCEPTION0(ImproperUseException);

    vtkRectilinearGrid *rg = (vtkRectilinearGrid *) ds;
    vtkDataArray *X = rg->GetXCoordinates();
    vtkDataArray *Y = rg->GetYCoordinates();
    vtkDataArray *Z = rg->GetZCoordinates();
    int dims[3];
    rg->GetDimensions(dims);
    double *Xdist = new double[dims[0]-1];
    for (int i = 0 ; i < dims[0]-1 ; i++)
        Xdist[i] = X->GetTuple1(i+1) - X->GetTuple1(i);
    double *Ydist = new double[dims[1]-1];
    for (int i = 0 ; i < dims[1]-1 ; i++)
        Ydist[i] = Y->GetTuple1(i+1) - Y->GetTuple1(i);
    double *Zdist = new double[dims[2]-1];
    for (int i = 0 ; i < dims[2]-1 ; i++)
        Zdist[i] = Z->GetTuple1(i+1) - Z->GetTuple1(i);

    for (int k = 0 ; k < dims[2]-1 ; k++)
        for (int j = 0 ; j < dims[1]-1 ; j++)
            for (int i = 0 ; i < dims[0]-1 ; i++)
            {
                int idx = k*(dims[1]-1)*(dims[0]-1) + j*(dims[0]-1) + i;
                double vol = Xdist[i]*Ydist[j]*Zdist[k];
                rv->SetTuple1(idx, vol);
            }

    delete [] Xdist;
    delete [] Ydist;
    delete [] Zdist;
}

double d_hex_volume(double coords[][3])
{
  double x[8];
  double y[8];
  double z[8];
  for (int i = 0; i < 8; i++)
  {
      x[i] = coords[i][0];
      y[i] = coords[i][1];
      z[i] = coords[i][2];
  }
  double aj[9];
  aj[0]=-x[0]-x[1]+x[2]+x[3]-x[4]-x[5]+x[6]+x[7];
  aj[3]=-x[0]-x[1]-x[2]-x[3]+x[4]+x[5]+x[6]+x[7];
  aj[6]=-x[0]+x[1]+x[2]-x[3]-x[4]+x[5]+x[6]-x[7];
  aj[1]=-y[0]-y[1]+y[2]+y[3]-y[4]-y[5]+y[6]+y[7];
  aj[4]=-y[0]-y[1]-y[2]-y[3]+y[4]+y[5]+y[6]+y[7];
  aj[7]=-y[0]+y[1]+y[2]-y[3]-y[4]+y[5]+y[6]-y[7];
  aj[2]=-z[0]-z[1]+z[2]+z[3]-z[4]-z[5]+z[6]+z[7];
  aj[5]=-z[0]-z[1]-z[2]-z[3]+z[4]+z[5]+z[6]+z[7];
  aj[8]=-z[0]+z[1]+z[2]-z[3]-z[4]+z[5]+z[6]-z[7];

  double vol = aj[0]*(aj[4]*aj[8]-aj[5]*aj[7])
             + aj[1]*(aj[5]*aj[6]-aj[3]*aj[8])
             + aj[2]*(aj[3]*aj[7]-aj[4]*aj[6]);
  vol *= 0.0156250;
  return vol;
}
