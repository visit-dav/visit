// ************************************************************************* //
//                              avtVMetricVolume.C                           //
// ************************************************************************* //

#include "avtVMetricVolume.h"

#include <vtkDataSet.h>
#include <vtkFloatArray.h>

#include <verdict.h>

#include <DebugStream.h>


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
// ****************************************************************************

avtVMetricVolume::avtVMetricVolume()
{
    useOnlyPositiveVolumes = false;
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
// ****************************************************************************

double avtVMetricVolume::Metric (double coords[][3], int type)
{
#ifdef HAVE_VERDICT 
    double rv = 0.;
    switch (type)
    {
      case VTK_VOXEL:   // Note that the verdict filter already swapped the
                        // coordinates to make a voxel be like a hex.
      case VTK_HEXAHEDRON:
        rv = v_hex_volume(8,coords);
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

    return rv;
#else
    return -1.;
#endif
}


