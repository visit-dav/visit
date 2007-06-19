/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                         avtViscousStressFilter.C                          //
// ************************************************************************* //

#include <avtViscousStressFilter.h>

#include <math.h>

#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkMath.h>
#include <vtkPointData.h>

#include <ExpressionException.h>

// ****************************************************************************
//  Method: avtViscousStressFilter constructor
//
//  Programmer: Cyrus Harrison
//  Creation:   June 5, 2007
//
// ****************************************************************************

avtViscousStressFilter::avtViscousStressFilter()
{
    ;
}

// ****************************************************************************
//  Method: avtViscousStressFilter destructor
//
//  Programmer: Cyrus Harrison
//  Creation:   June 5, 2007
//
// ****************************************************************************

avtViscousStressFilter::~avtViscousStressFilter()
{
    ;
}

// ****************************************************************************
//  Method: avtViscousStressFilter::DeriveVariable
//
//  Purpose:
//      Constructs a viscous stress tensor given a velocity vector
//
//  Programmer: Cyrus Harrison
//  Creation:   June 5, 2007
//
//  Modifications:
//    Cyrus Harrison, Mon Jun 18 13:41:51 PDT 2007
//    Added explicit check for rectilinear or structured grid.
//
// ****************************************************************************

vtkDataArray *
avtViscousStressFilter::DeriveVariable(vtkDataSet *in_ds)
{
    // get dim and rz_mesh
    int  topo_dim = -1;
    bool rz_mesh  = false;

    if ( *(GetInput()) != NULL )
    {
        avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
        topo_dim = atts.GetTopologicalDimension();

        if(topo_dim == 2)
        if(GetInput()->GetInfo().GetAttributes().GetMeshCoordType()== AVT_XY)
        {rz_mesh = false;}
        else
        {rz_mesh = true;}

        if(topo_dim != 2)
        {
          EXCEPTION1(ExpressionException, "The viscous stress expression "
                   "currently only supports 2D datasets.");
        }
    }

    // make sure input dataset is a rectilinear or structured grid

    int dtype = in_ds->GetDataObjectType();

    if( ( dtype != VTK_STRUCTURED_GRID) and
        ( dtype != VTK_RECTILINEAR_GRID ))
    {
        EXCEPTION1(ExpressionException,
        "Viscous Stress Expression requires a rectilinear or structured mesh");
    }

    // velocity is nodal value
    vtkDataArray *vel = in_ds->GetPointData()->GetArray(activeVariable);

    if( vel == NULL || vel->GetNumberOfComponents() != 3 )
    {
        EXCEPTION1(ExpressionException, "The viscous stress expression "
                   "was not passed a valid velocity vector.");
    }

    // get number of cells
    int ncells = in_ds->GetNumberOfCells();

    // create the result dataset
    vtkFloatArray *res_tensor = vtkFloatArray::New();
    res_tensor->SetNumberOfComponents(9);
    res_tensor->SetNumberOfTuples(ncells);

    // loop index and tensor values
    int i=0;
    double res_vals[9];

    // calculate tensor for each cell
    for( i = 0; i < ncells; i++)
    {
        if(topo_dim == 2)
        {CalculateVStress2D(in_ds,vel,i,rz_mesh,res_vals);}
        res_tensor->SetTuple(i, res_vals);
    }

    return res_tensor;
}



// ****************************************************************************
//  Method: avtViscousStressFilter::CalculateVStress2D
//
//  Purpose:
//      Calculate 2D Viscous Stress Tensor for a Cell
//
//  Programmer: Cyrus Harrison
//  Creation:   June 10 13:35:13 PDT 2007
//
//  Modifications:
//    Cyrus Harrison, Mon Jun 18 13:41:51 PDT 2007
//    Added explicit check for quad cells
//
// ****************************************************************************

void
avtViscousStressFilter::CalculateVStress2D(vtkDataSet *ds,
                                           vtkDataArray *ve,
                                           int idx,
                                           bool rz_mesh,
                                           double *vstress)
{
    // loop index
    int i;
    // point locs and velocities
    double px[4], py[4], vx[4], vy[4];
    // gradient terms
    double dx[2], dy[2], du[2], dv[2];
    double div      = 0.0;
    // gradients
    double dvx[2], dvy[2];
    // extra divergence term if rz_mesh
    double cyl_term = 0.0;
    // helper to make sure we do not divide by zero
    double tiny     = 1e-3;

    double *pt_val = NULL;
    double *ve_val = NULL;

    // make sure the cell is a quad (assumed for proper vel gradients)
    if ( ( ds->GetCellType(i) != VTK_QUAD ) &&
         ( ds->GetCellType(i) != VTK_PIXEL ) )
    {
        // if cell is not a quad, ignore
        memset(vstress,0,sizeof(double)*9);
        return;
    }

    // extract the location and velocity
    vtkCell   *cell = ds->GetCell(idx);
    vtkIdList *cids = cell->GetPointIds();

    int ids[4];
    // get ids that match the gradient template
    ids[3] = cids->GetId(0);
    ids[0] = cids->GetId(1);
    ids[1] = cids->GetId(2);
    ids[2] = cids->GetId(3);

    int id = 0;
    for( i = 0; i< 4; i++)
    {
        id = ids[i];
        pt_val = ds->GetPoint(id);
        ve_val = ve->GetTuple(id);
        px[i] = pt_val[0];
        py[i] = pt_val[1];
        vx[i] = ve_val[0];
        vy[i] = ve_val[1];
    }

    // calculate the gradient
    dx[0] = .5 * (px[0] + px[1] - px[2] - px[3]);
    dx[1] = .5 * (px[1] + px[2] - px[3] - px[0]);

    dy[0] = .5 * (py[0] + py[1] - py[2] - py[3]);
    dy[1] = .5 * (py[1] + py[2] - py[3] - py[0]);

    du[0] = .5 * (vx[0] + vx[1] - vx[2] - vx[3]);
    du[1] = .5 * (vx[1] + vx[2] - vx[3] - vx[0]);

    dv[0] = .5 * (vy[0] + vy[1] - vy[2] - vy[3]);
    dv[1] = .5 * (vy[1] + vy[2] - vy[3] - vy[0]);

    div = 1.0 / (dx[0] *dy[1] - dx[1] *dy[0] + tiny);

    dvx[0] = div * (du[0]*dy[1] - du[1] * dy[0]);
    dvx[1] = div * (du[1]*dx[0] - du[0] * dx[1]);

    dvy[0] = div * (dv[0]*dy[1] - dv[1] * dy[0]);
    dvy[1] = div * (dv[1]*dx[0] - dv[0] * dx[1]);

    // create the tensor

    // if rz mesh include extra divergence term
    if( rz_mesh )
    {
        cyl_term = (vy[0] + vy[1] + vy[2] + vy[3]) /
                   (py[0] + py[1] + py[2] + py[3] + tiny);
    }

    // diag terms
    vstress[0] = 1/3.0 * (2.0 * dvx[0] - dvy[1]- cyl_term);
    vstress[4] = 1/3.0 * (2.0 * dvy[1] - dvx[0]- cyl_term);
    vstress[8] = 0.0;
    // other terms
    vstress[1] = 0.5 * (dvy[0]  + dvx[1]);
    vstress[2] = 0.0;
    vstress[5] = 0.0;

    // use symm to fill out remaining terms
    vstress[3] = vstress[1];
    vstress[6] = vstress[2];
    vstress[7] = vstress[5];
}

