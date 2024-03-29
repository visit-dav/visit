// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtStrainRateExpression.C                           //
// ************************************************************************* //

#include <avtStrainRateExpression.h>
#include <math.h>

#include <vtkCellType.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkIdList.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>

#include <ExpressionException.h>

#include <vector>

// ****************************************************************************
//  Method: avtStrainRateExpression constructor
//
//  Programmer: Thomas R. Treadway
//  Creation:  Wed Nov 15 12:57:36 PST 2006
//
// ****************************************************************************

avtStrainRateExpression::avtStrainRateExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtStrainRateExpression destructor
//
//  Programmer: Thomas R. Treadway
//  Creation:   Wed Nov 15 12:57:36 PST 2006
//
// ****************************************************************************

avtStrainRateExpression::~avtStrainRateExpression()
{
    ;
}


// ****************************************************************************
//  Method: avtStrainRateExpression::DeriveVariable
//
//  Purpose:
//      Determines the strain using Rate
//
//  Programmer: Thomas R. Treadway
//  Creation:   Wed Nov 15 12:57:36 PST 2006
//
//  Modifications:
//    Eric Brugger, Thu Sep  8 14:55:01 PDT 2011
//    I made corrections to the calculation of the strain rate provided by
//    Bob Corey.
//
//    Kathleen Biagas, Wed Apr 4 12:04:10 PDT 2012
//    Set output's data type to same as input. 
// 
//    Justin Privitera, Fri Sep 16 11:58:19 PDT 2022
//    Renamed avtStrainTensorExpression to avtStrainExpression.
//
// ****************************************************************************

vtkDataArray *
avtStrainRateExpression::DeriveVariable(vtkDataSet *in_ds, int currentDomainsIndex)
{
    char msg[1024];
    double vals[3];
    double out2[9];
    // same as Griz variables
    double xx[8], yy[8], zz[8];      // Current element geom.
    double xv[8], yv[8], zv[8];      // Current node velocities.
    double px[8], py[8], pz[8];      // Global derivates dN/dx,dN/dy,dN/dz.
    double F[9];                     // Element deformation gradient.
    double detF;                     // Determinant of element
                                     // deformation gradient.
    double strain[6];                // Calculated strain.

    std::vector<int> cellsToIgnore;
    double avgTensor[9];             // ghost zone value
    int nTensors = 0;                // number of tensors in average

    // Let's get the points from the input dataset.
    if (in_ds->GetDataObjectType() != VTK_UNSTRUCTURED_GRID)
    {
        EXCEPTION2(ExpressionException, outputVariableName, "The strain expression "
                   "only operates on unstructured grids.");
    }
    vtkUnstructuredGrid *in_usg = vtkUnstructuredGrid::SafeDownCast(in_ds);
    size_t nCells = in_usg->GetNumberOfCells();
    vtkDataArray *vel_data = in_ds->GetPointData()->GetArray(varnames[1]);
    if (vel_data == NULL) 
    {           
        sprintf(msg, "The strain expression "
            "could not extract the data array for: %s", varnames[1]);
        EXCEPTION2(ExpressionException, outputVariableName, msg);
    }
    vtkDataArray *ghost_data = in_ds->GetPointData()->GetArray("avtGhostNodes");
    unsigned char *ghost =
        (unsigned char *) (ghost_data ? ghost_data->GetVoidPointer(0) : 0);
    vtkIdList *pointIds = vtkIdList::New();
    vtkDataArray *out = vel_data->NewInstance();
    out->SetNumberOfComponents(9);
    out->SetNumberOfTuples(nCells);
    for (size_t j = 0; j < 9; j++) 
        avgTensor[j] = 0.0;
    for (int i = 0; i < nCells; i++)
    {   // Check Voxel format
        int cellType = in_usg->GetCellType(i);
        // ignore everything but hexes
        if (cellType == VTK_HEXAHEDRON)
        {   // Grab a pointer to the cell's points' underlying data array
            in_usg->GetCellPoints(i, pointIds);
            bool anyGhost = false;
            if (ghost)
            {
                for (size_t j = 0; j < 8; j++)
                {
                    if (ghost[pointIds->GetId(j)] != 0)
                    {   // any ghost nodes in this hex
                        anyGhost = true;
                        break;
                    }
                } 
            }
            if (anyGhost)            
            {
                cellsToIgnore.push_back(i);
                continue;            
            }
            for (size_t j = 0; j < 8; j++)
            {   // Package current velocity values into vtkDataArray
                vel_data->GetTuple(pointIds->GetId(j), vals);
                xv[j] = vals[0];
                yv[j] = vals[1];
                zv[j] = vals[2];
            }
            for (size_t j = 0; j < 8; j++)
            {   // Package current element geometry points into vtkDataArray
                in_usg->GetPoint(pointIds->GetId(j), vals);
                xx[j] = vals[0];
                yy[j] = vals[1];
                zz[j] = vals[2];
            }
            //
            // This is where the strain algorithms start to differ
            avtStrainExpression::HexPartialDerivative
                (px, py, pz, xx, yy, zz);
            for (size_t j = 0; j < 9; j++) 
                F[j] = 0.0;
            // Copied from Griz
            for (size_t k = 0; k < 8; k++ )
            {
                F[0] = F[0] + px[k]*xv[k];
                F[1] = F[1] + py[k]*xv[k];
                F[2] = F[2] + pz[k]*xv[k];
                F[3] = F[3] + px[k]*yv[k];
                F[4] = F[4] + py[k]*yv[k];
                F[5] = F[5] + pz[k]*yv[k];
                F[6] = F[6] + px[k]*zv[k];
                F[7] = F[7] + py[k]*zv[k];
                F[8] = F[8] + pz[k]*zv[k];
            }
            detF = F[0]*F[4]*F[8] + F[1]*F[5]*F[6]
                + F[2]*F[3]*F[7]
                - F[2]*F[4]*F[6] - F[1]*F[3]*F[8]
                - F[0]*F[5]*F[7];
            detF = fabs( detF );
            strain[0] = F[0];
            strain[1] = F[4];
            strain[2] = F[8];
            strain[3] = 0.5 * (F[1] + F[3]);
            strain[4] = 0.5 * (F[5] + F[7]);
            strain[5] = 0.5 * (F[2] + F[6]);
            // End of differences
            //
            // reorder Tensor
            out2[0] = strain[0];  // XX
            out2[1] = strain[3];  // XY
            out2[2] = strain[5];  // XZ
            out2[3] = strain[3];  // YX
            out2[4] = strain[1];  // YY
            out2[5] = strain[4];  // YZ
            out2[6] = strain[5];  // ZX
            out2[7] = strain[4];  // ZY
            out2[8] = strain[2];  // ZZ
            nTensors++;
            for (size_t j = 0; j < 9; j++)
                avgTensor[j] += out2[j];
        } 
        else 
        { // cell is not a hexhedron also ignore              
            cellsToIgnore.push_back(i);
        }
        out->SetTuple(i, out2);
    }
    if (nTensors > 0)
    {       
        for (size_t j = 0; j < 9; j++) 
            avgTensor[j] = avgTensor[j]/nTensors;
    }           
    for (size_t i = 0; i < cellsToIgnore.size(); i++)
    {       
        out->SetTuple(cellsToIgnore[i], avgTensor);
    }
    return out;
}
// ****************************************************************************
//  Method: avtStrainRateExpression::GetVariableDimension
//              
//  Purpose:    
//      Determines what the variable dimension of the output is.
//              
//  Programmer: Thomas R. Treadway
//  Creation:   Tue Nov 28 09:56:22 PST 2006
//              
// ****************************************************************************
                
int         
avtStrainRateExpression::GetVariableDimension(void)
{               
    if (*(GetInput()) == NULL)
        return avtMultipleInputExpressionFilter::GetVariableDimension();
    if (varnames.size() != 2) 
        return avtMultipleInputExpressionFilter::GetVariableDimension();
            
    avtDataAttributes &atts = GetInput()->GetInfo().GetAttributes();
    if (! atts.ValidVariable(varnames[0]))
        return avtMultipleInputExpressionFilter::GetVariableDimension();
    int ncomp1 = atts.GetVariableDimension(varnames[0]);
            
    if (! atts.ValidVariable(varnames[1]))
        return avtMultipleInputExpressionFilter::GetVariableDimension();
    int ncomp2 = atts.GetVariableDimension(varnames[1]);
            
    return GetNumberOfComponentsInOutput(ncomp1, ncomp2);
}
