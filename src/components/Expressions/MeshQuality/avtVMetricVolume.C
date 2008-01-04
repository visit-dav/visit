/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                              avtVMetricVolume.C                           //
// ************************************************************************* //

#include "avtVMetricVolume.h"

#include <vtkCellType.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>

#include <verdict.h>

#include <DebugStream.h>


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
// ****************************************************************************

avtVMetricVolume::avtVMetricVolume()
{
    useOnlyPositiveVolumes = false;
    useVerdictHex = true;
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

    return rv;
#else
    return -1.;
#endif
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
    int  i, j, k;

    if (ds->GetDataObjectType() != VTK_RECTILINEAR_GRID)
        EXCEPTION0(ImproperUseException);

    vtkRectilinearGrid *rg = (vtkRectilinearGrid *) ds;
    vtkDataArray *X = rg->GetXCoordinates();
    vtkDataArray *Y = rg->GetYCoordinates();
    vtkDataArray *Z = rg->GetZCoordinates();
    int dims[3];
    rg->GetDimensions(dims);
    float *Xdist = new float[dims[0]-1];
    for (i = 0 ; i < dims[0]-1 ; i++)
        Xdist[i] = X->GetTuple1(i+1) - X->GetTuple1(i);
    float *Ydist = new float[dims[1]-1];
    for (i = 0 ; i < dims[1]-1 ; i++)
        Ydist[i] = Y->GetTuple1(i+1) - Y->GetTuple1(i);
    float *Zdist = new float[dims[2]-1];
    for (i = 0 ; i < dims[2]-1 ; i++)
        Zdist[i] = Z->GetTuple1(i+1) - Z->GetTuple1(i);

    for (k = 0 ; k < dims[2]-1 ; k++)
        for (j = 0 ; j < dims[1]-1 ; j++)
            for (i = 0 ; i < dims[0]-1 ; i++)
            {
                int idx = k*(dims[1]-1)*(dims[0]-1) + j*(dims[0]-1) + i;
                float vol = Xdist[i]*Ydist[j]*Zdist[k];
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
